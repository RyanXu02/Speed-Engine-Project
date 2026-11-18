#include "pch.h"

#include "DebugRenderer.h"
#include "Core/Engine.h"
#include "Core/ResourceManagers/ResourceManager.h"
#include "Core/SubSystems/Graphics/Rendering/RenderContext.h"

#include <glad/glad.h>

namespace SE
{
	void DebugRenderer::init()
	{
		auto& resources = *Engine::Instance().getSubSystem<ResourceManager>();
		m_shaderId = resources.requestShader({ "Assets/Shaders/debug.vert", "Assets/Shaders/debug.frag", "debug" });
	}

	void DebugRenderer::_lazyInit()
	{
		m_logger.info("Initializing");

		glGenVertexArrays(1, &m_VAO);
		glGenBuffers(1, &m_VBO);
		if (!m_VAO || !m_VBO)
		{
			shutdown();
			throw std::runtime_error("Failed to create DebugRenderer vertex buffers");
		}

		glBindVertexArray(m_VAO);
		glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(DebugVertex), (void*)offsetof(DebugVertex, position));
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(DebugVertex), (void*)offsetof(DebugVertex, color));
		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	void DebugRenderer::render(RenderContext& context)
	{
		if (context.frame.debugLines.empty() && context.frame.debugTriangles.empty())
			return;
		auto* shader = context.gpu.shader(context.resources, m_shaderId, m_logger);
		if (!shader)
			return;

		if (!m_VAO)
			_lazyInit();

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glDepthMask(GL_FALSE);
		glDisable(GL_CULL_FACE);
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

		shader->bind();
		shader->setMat4("projectionView", context.viewport.getCameraFrustum().viewProjectionMatrix);
		glBindVertexArray(m_VAO);
		_draw(context.frame.debugTriangles, GL_TRIANGLES);
		_draw(context.frame.debugLines, GL_LINES);
		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		shader->unbind();

		glDisable(GL_BLEND);
		glEnable(GL_CULL_FACE);
		glDepthMask(GL_TRUE);
		glDepthFunc(GL_LESS);
	}

	void DebugRenderer::_draw(std::span<const DebugVertex> vertices, unsigned int mode) const
	{
		if (vertices.empty())
			return;

		glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
		glBufferData(GL_ARRAY_BUFFER, vertices.size_bytes(), vertices.data(), GL_STREAM_DRAW);
		glDrawArrays(mode, 0, static_cast<GLsizei>(vertices.size()));
	}

	void DebugRenderer::shutdown()
	{
		if (!m_VAO && !m_VBO)
			return;

		m_logger.info("Shutting down");
		glDeleteBuffers(1, &m_VBO);
		glDeleteVertexArrays(1, &m_VAO);
		m_VBO = 0;
		m_VAO = 0;
	}
} // namespace SE
