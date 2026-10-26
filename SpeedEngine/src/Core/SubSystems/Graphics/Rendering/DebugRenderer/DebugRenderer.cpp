#include "pch.h"

#include "DebugRenderer.h"
#include "Core/Engine.h"
#include "Core/ResourceManagers/ResourceManager.h"
#include "Core/SubSystems/Graphics/Rendering/RenderContext.h"
#include "Core/SubSystems/Physics/PhysicsSystem.h"

#include <glad/glad.h>

namespace SE
{
	void DebugRenderer::init()
	{
		m_logger.info("Initializing");

		auto* resources = Engine::Instance().getSubSystem<ResourceManager>();
		const std::string* vertexSource = resources->getString("Assets/Shaders/debug.vert");
		const std::string* fragmentSource = resources->getString("Assets/Shaders/debug.frag");
		GLCompStatus status = m_shader.init(*vertexSource, *fragmentSource);
		if (!status.success)
		{
			m_shader.destroy();
			throw std::runtime_error("Failed to initialize DebugRenderer shader: " + status.log);
		}

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

	void DebugRenderer::drawLine(const glm::vec3& from, const glm::vec3& to, const glm::vec4& color)
	{
		m_lines.push_back({ from, color });
		m_lines.push_back({ to, color });
	}

	void DebugRenderer::drawTri(const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3, const glm::vec4& color, bool)
	{
		m_triangles.push_back({ v1, color });
		m_triangles.push_back({ v2, color });
		m_triangles.push_back({ v3, color });
	}

	void DebugRenderer::render(RenderContext& context)
	{
		// "draw" physics bodies, which hooks back into DebugRenderer with drawLine
		Engine::Instance().getSubSystem<PhysicsSystem>()->debugDrawBodies();

		if (m_lines.empty() && m_triangles.empty())
			return;

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glDepthMask(GL_FALSE);
		glDisable(GL_CULL_FACE);
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

		m_shader.bind();
		m_shader.setMat4("projectionView", context.viewport.getCameraFrustum().viewProjectionMatrix);
		glBindVertexArray(m_VAO);
		_draw(m_triangles, GL_TRIANGLES);
		_draw(m_lines, GL_LINES);
		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		m_shader.unbind();

		glDisable(GL_BLEND);
		glEnable(GL_CULL_FACE);
		glDepthMask(GL_TRUE);
		glDepthFunc(GL_LESS);

		m_lines.clear();
		m_triangles.clear();
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
		m_logger.info("Shutting down");
		glDeleteBuffers(1, &m_VBO);
		glDeleteVertexArrays(1, &m_VAO);
		m_VBO = 0;
		m_VAO = 0;
		m_shader.destroy();
		m_lines.clear();
		m_triangles.clear();
	}
} // namespace SE
