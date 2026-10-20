#include "pch.h"

#include "GrayscaleRenderer.h"
#include "Core/Engine.h"
#include "Core/ResourceManagers/ResourceManager.h"
#include "Core/SubSystems/Graphics/Rendering/RenderContext.h"

#include <glad/glad.h>

namespace SE
{

	GrayscaleRenderer::GrayscaleRenderer(Logger& logger, std::string inputTarget)
	    : Renderer(logger, "GrayscaleRenderer"),
	      m_inputTarget(std::move(inputTarget)),
	      m_shader(0, "GrayscaleRenderer")
	{}

	void GrayscaleRenderer::init()
	{
		m_logger.info("Initializing");

		auto* resources = Engine::Instance().getSubSystem<ResourceManager>();
		std::string vertexSource = resources->getString("Assets/Shaders/grayscale.vert");
		std::string fragmentSource = resources->getString("Assets/Shaders/grayscale.frag");
		GLCompStatus status = m_shader.init(vertexSource, fragmentSource);
		if (!status.success)
		{
			m_shader.destroy();
			throw std::runtime_error("Failed to initialize GrayscaleRenderer shader: " + status.log);
		}
		glGenVertexArrays(1, &m_VAO);
		if (!m_VAO)
		{
			shutdown();
			throw std::runtime_error("Failed to create GrayscaleRenderer vertex array");
		}
	}

	void GrayscaleRenderer::render(RenderContext& context) const
	{
		// from scene renderer pass
		const FBO& input = context.getRenderTarget(m_inputTarget);

		glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);

		m_shader.bind();
		m_shader.setInt("sceneColor", 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, input.getColorTexture());
		glBindVertexArray(m_VAO);
		glDrawArrays(GL_TRIANGLES, 0, 3);

		glBindVertexArray(0);
		glBindTexture(GL_TEXTURE_2D, 0);
		m_shader.unbind();
		glEnable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);
	}

	void GrayscaleRenderer::shutdown()
	{
		m_logger.info("Shutting down");
		glDeleteVertexArrays(1, &m_VAO);
		m_VAO = 0;
		m_shader.destroy();
	}
} // namespace SE
