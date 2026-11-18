#include "pch.h"

#include "ImageBasedRenderer.h"
#include "Core/Engine.h"
#include "Core/ResourceManagers/ResourceManager.h"
#include "Core/ResourceManagers/Shader/Shader.h"

#include <glad/glad.h>

namespace SE
{
	static constexpr int ENV_CUBEMAP_SIZE = 512;	// cube face size converted from the HDR
	static constexpr int PREFILTER_SIZE = 128;		// mip 0 face size of the prefiltered cubemap
	static constexpr int PREFILTER_MIPS = 5;		// roughness 0, 0.25, 0.5, 0.75, 1, MAX_REFLECTION_LOD in the lighting shaders is this - 1
	static constexpr int BRDF_LUT_SIZE = 512;
	static constexpr int SH_GROUP_SIZE = 16;		// local_size in irradianceSH.comp
	static constexpr int IMAGE_GROUP_SIZE = 8;		// local_size in the other IBL compute shaders

	static int _groupCount(int size, int groupSize)
	{
		return (size + groupSize - 1) / groupSize;
	}

	ImageBasedRenderer::ImageBasedRenderer(Logger& logger, std::string hdrPath)
	    : Renderer(logger, "ImageBasedRenderer"),
	      m_hdrPath(std::move(hdrPath))
	{}

	void ImageBasedRenderer::init()
	{
		m_logger.info("Initializing");

		// filter across cube face edges, otherwise blurry mips show seams
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

		// the equirect is only needed while precomputing
		auto& resources = *Engine::Instance().getSubSystem<ResourceManager>();
		const auto id = resources.requestTexture({ m_hdrPath, "Environment" });
		resources.waitForStartupResource(id);
		const auto* image = resources.getResource<TextureData>(id);
		if (!image || image->hdrPixels.empty()) {
			m_logger.warn("Failed to load HDR environment {}, using flat ambient instead", m_hdrPath);
			return;
		}

		unsigned int hdrTexture = 0;
		glGenTextures(1, &hdrTexture);
		glBindTexture(GL_TEXTURE_2D, hdrTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, image->width, image->height, 0, GL_RGB, GL_FLOAT, image->hdrPixels.data());
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);		// longitude wraps around
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glBindTexture(GL_TEXTURE_2D, 0);

		_equirectangularToCubemap(hdrTexture);
		_irradianceSH(hdrTexture, image->width, image->height);
		_prefilter();
		_integrateBRDF();
		glDeleteTextures(1, &hdrTexture);

		glUseProgram(0);
		glActiveTexture(GL_TEXTURE0);
		m_ready = m_envCubemap && m_prefilterMap && m_brdfLUTTexture;
		if (m_ready) {
			m_logger.info("Precomputed image based lighting from {}", m_hdrPath);
		}
	}

	void ImageBasedRenderer::shutdown()
	{
		m_logger.info("Shutting down");
		const unsigned int textures[] = { m_envCubemap, m_prefilterMap, m_brdfLUTTexture };
		glDeleteTextures(static_cast<GLsizei>(std::size(textures)), textures);
		m_envCubemap = 0;
		m_prefilterMap = 0;
		m_brdfLUTTexture = 0;
		m_ready = false;
	}

	bool ImageBasedRenderer::_compile(Shader& shader, std::string_view path)
	{
		auto* resources = Engine::Instance().getSubSystem<ResourceManager>();
		const auto id = resources->requestComputeShader({ path, path });
		if (!resources->waitForStartupResource(id)) {
			m_logger.warn("Missing compute shader {}", path);
			return false;
		}
		const auto status = shader.initCompute(resources->getResource<ShaderSource>(id)->compute);
		if (!status.success) {
			m_logger.warn("Failed to compile {}: {}", path, status.log);
			return false;
		}
		return true;
	}

	// equirect to a mipmapped cubemap, the mips let the prefilter read blurrier levels for sparse samples
	void ImageBasedRenderer::_equirectangularToCubemap(unsigned int hdrTexture)
	{
		Shader shader;
		if (!_compile(shader, "Assets/Shaders/equirectangularToCubemap.comp")) {
			return;
		}

		const int mips = static_cast<int>(std::log2(ENV_CUBEMAP_SIZE)) + 1;
		glGenTextures(1, &m_envCubemap);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_envCubemap);
		glTexStorage2D(GL_TEXTURE_CUBE_MAP, mips, GL_RGBA16F, ENV_CUBEMAP_SIZE, ENV_CUBEMAP_SIZE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		shader.bind();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, hdrTexture);
		glBindImageTexture(0, m_envCubemap, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA16F);
		const int groups = _groupCount(ENV_CUBEMAP_SIZE, IMAGE_GROUP_SIZE);
		glDispatchCompute(groups, groups, 6);
		glMemoryBarrier(GL_ALL_BARRIER_BITS);

		glBindTexture(GL_TEXTURE_CUBE_MAP, m_envCubemap);
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		glBindTexture(GL_TEXTURE_2D, 0);
		glBindImageTexture(0, 0, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
		shader.destroy();
	}

	// GPU sums each 16x16 block, the CPU adds the blocks and applies the cosine lobe
	void ImageBasedRenderer::_irradianceSH(unsigned int hdrTexture, int width, int height)
	{
		Shader shader;
		if (!_compile(shader, "Assets/Shaders/irradianceSH.comp")) {
			return;
		}

		const int groupsX = _groupCount(width, SH_GROUP_SIZE);
		const int groupsY = _groupCount(height, SH_GROUP_SIZE);
		const size_t partialCount = static_cast<size_t>(groupsX) * groupsY * 9;

		unsigned int buffer = 0;
		glGenBuffers(1, &buffer);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, partialCount * sizeof(glm::vec4), nullptr, GL_STREAM_READ);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, buffer);

		shader.bind();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, hdrTexture);
		glDispatchCompute(groupsX, groupsY, 1);
		glMemoryBarrier(GL_ALL_BARRIER_BITS);

		std::vector<glm::vec4> partials(partialCount);
		glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, partialCount * sizeof(glm::vec4), partials.data());
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
		glDeleteBuffers(1, &buffer);
		glBindTexture(GL_TEXTURE_2D, 0);
		shader.destroy();

		// sum in double, there are thousands of blocks
		std::array<glm::dvec3, 9> radiance{};
		for (size_t i = 0; i < partials.size(); ++i) {
			radiance[i % 9] += glm::dvec3(partials[i]);
		}

		// cosine lobe convolution per band (Ramamoorthi and Hanrahan 2001): pi, 2pi/3, pi/4,
		// then divided by pi so the shader only multiplies by albedo
		constexpr std::array<double, 9> BAND_SCALE = { 1.0, 2.0 / 3.0, 2.0 / 3.0, 2.0 / 3.0, 0.25, 0.25, 0.25, 0.25, 0.25 };
		for (size_t i = 0; i < 9; ++i) {
			m_shIrradiance[i] = glm::vec3(radiance[i] * BAND_SCALE[i]);
		}
	}

	// one dispatch per mip, rougher mips blur over a wider GGX lobe
	void ImageBasedRenderer::_prefilter()
	{
		if (!m_envCubemap) {
			return;
		}

		Shader shader;
		if (!_compile(shader, "Assets/Shaders/prefilter.comp")) {
			return;
		}

		glGenTextures(1, &m_prefilterMap);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_prefilterMap);
		glTexStorage2D(GL_TEXTURE_CUBE_MAP, PREFILTER_MIPS, GL_RGBA16F, PREFILTER_SIZE, PREFILTER_SIZE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		shader.bind();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_envCubemap);
		for (int mip = 0; mip < PREFILTER_MIPS; ++mip) {
			shader.setFloat("roughness", static_cast<float>(mip) / static_cast<float>(PREFILTER_MIPS - 1));
			glBindImageTexture(0, m_prefilterMap, mip, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA16F);
			const int groups = _groupCount(PREFILTER_SIZE >> mip, IMAGE_GROUP_SIZE);
			glDispatchCompute(groups, groups, 6);
		}
		glMemoryBarrier(GL_ALL_BARRIER_BITS);

		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		glBindImageTexture(0, 0, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
		shader.destroy();
	}

	// independent of the environment, only depends on N.V and roughness
	void ImageBasedRenderer::_integrateBRDF()
	{
		Shader shader;
		if (!_compile(shader, "Assets/Shaders/brdf.comp")) {
			return;
		}

		glGenTextures(1, &m_brdfLUTTexture);
		glBindTexture(GL_TEXTURE_2D, m_brdfLUTTexture);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RG16F, BRDF_LUT_SIZE, BRDF_LUT_SIZE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glBindTexture(GL_TEXTURE_2D, 0);

		shader.bind();
		glBindImageTexture(0, m_brdfLUTTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG16F);
		const int groups = _groupCount(BRDF_LUT_SIZE, IMAGE_GROUP_SIZE);
		glDispatchCompute(groups, groups, 1);
		glMemoryBarrier(GL_ALL_BARRIER_BITS);

		glBindImageTexture(0, 0, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG16F);
		shader.destroy();
	}
} // namespace SE
