#include "pch.h"

#include "TonemapRenderer.h"
#include "Core/Engine.h"
#include "Core/ResourceManagers/ResourceManager.h"
#include "Core/SubSystems/Graphics/Rendering/RenderContext.h"

#include <glad/glad.h>

namespace SE
{
	TonemapRenderer::TonemapRenderer(Logger& logger, std::string inputTarget)
	    : Renderer(logger, "TonemapRenderer"),
	      m_inputTarget(std::move(inputTarget)),
	      m_shader(0, "TonemapRenderer")
	{}

	void TonemapRenderer::init()
	{
		m_logger.info("Initializing");

		auto* resources = Engine::Instance().getSubSystem<ResourceManager>();
		const std::string* vertexSource = resources->getString("Assets/Shaders/tonemap.vert");
		const std::string* fragmentSource = resources->getString("Assets/Shaders/tonemap.frag");
		GLCompStatus status = m_shader.init(*vertexSource, *fragmentSource);
		if (!status.success) {
			m_shader.destroy();
			throw std::runtime_error("Failed to initialize TonemapRenderer shader: " + status.log);
		}
		glGenVertexArrays(1, &m_VAO);
		if (!m_VAO) {
			shutdown();
			throw std::runtime_error("Failed to create TonemapRenderer vertex array");
		}
	}

	void TonemapRenderer::render(RenderContext& context)
	{
		const FBO& input = context.getRenderTarget(m_inputTarget);

		glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);

		m_shader.bind();
		m_shader.setInt("hdrColor", 0);
		m_shader.setFloat("exposure", exposure);
		m_shader.setFloat("gamma", gamma);
		m_shader.setBool("useDither", dither);
		m_shader.setInt("tonemapCurve", static_cast<int>(curve));

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, input.getColorTexture());
		glBindVertexArray(m_VAO);
		glDrawArrays(GL_TRIANGLES, 0, 3);

		glBindVertexArray(0);
		glBindTexture(GL_TEXTURE_2D, 0);
		m_shader.unbind();

		// copy the scene depth so passes drawn on top of this output are occluded correctly
		glBindFramebuffer(GL_READ_FRAMEBUFFER, input.getID());
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, context.output->getID());
		glBlitFramebuffer(0, 0, input.getWidth(), input.getHeight(),
			0, 0, context.output->getWidth(), context.output->getHeight(), GL_DEPTH_BUFFER_BIT, GL_NEAREST);
		context.output->bind();

		glEnable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);
	}

	void TonemapRenderer::shutdown()
	{
		m_logger.info("Shutting down");
		glDeleteVertexArrays(1, &m_VAO);
		m_VAO = 0;
		m_shader.destroy();
	}
} // namespace SE
