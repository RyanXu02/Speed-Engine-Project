#pragma once
#include "Core/SubSystems/Graphics/Rendering/Renderer.h"

#include <glm/glm.hpp>

// references from
// https://learnopengl.com/PBR/IBL/Specular-IBL
// https://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
// https://cdn2.unrealengine.com/Resources/files/2013SiggraphPresentationsNotes-26915738.pdf
namespace SE
{
	class Shader;

	enum class SkyboxSource : int
	{
		Environment,	// the HDR itself
		Irradiance		// the SH irradiance
	};

	// @brief Image based lighting from an equirectangular HDR, precomputed once with compute shaders in init
	class ImageBasedRenderer : public Renderer
	{
	public:
		// @param hdrPath equirectangular .hdr environment to light the scene with
		ImageBasedRenderer(Logger& logger, std::string hdrPath);

		void init() override;
		void render(RenderContext&) override {}
		void shutdown() override;

		bool isReady() const { return m_ready; }
		unsigned int getEnvCubemap() const { return m_envCubemap; }
		unsigned int getPrefilterMap() const { return m_prefilterMap; }
		unsigned int getBRDFLUTTexture() const { return m_brdfLUTTexture; }
		const std::array<glm::vec3, 9>& getSHIrradiance() const { return m_shIrradiance; }

		bool enabled = true;
		float intensity = 1.0f;		// scales both the lighting and the skybox
		bool drawSkybox = true;
		SkyboxSource skyboxSource = SkyboxSource::Environment;

	private:
		bool _compile(Shader& shader, std::string_view path);
		void _equirectangularToCubemap(unsigned int hdrTexture);
		void _irradianceSH(unsigned int hdrTexture, int width, int height);
		void _prefilter();
		void _integrateBRDF();

		std::string m_hdrPath;
		bool m_ready = false;

		unsigned int m_envCubemap = 0;		// source cubemap, mipmapped for the prefilter
		unsigned int m_prefilterMap = 0;	// GGX prefiltered, mip = roughness
		unsigned int m_brdfLUTTexture = 0;	// RG16F scale and bias on F0
		std::array<glm::vec3, 9> m_shIrradiance{}; // cosine convolved and divided by pi, ready to multiply by albedo
	};
} // namespace SE
