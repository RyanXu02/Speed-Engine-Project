#include "pch.h"
#include "ParticleRenderer.h"

#include "Core/Engine.h"
#include "Core/ResourceManagers/ResourceManager.h"
#include "Core/SubSystems/Graphics/Rendering/RenderContext.h"

#include <glad/glad.h>

namespace SE
{
	void ParticleRenderer::init()
	{
		m_logger.info("Initializing");

		auto* resources = Engine::Instance().getSubSystem<ResourceManager>();
		const std::string* vertexSource = resources->getString("Assets/Shaders/particle_billboard.vert");
		const std::string* fragmentSource = resources->getString("Assets/Shaders/particle_billboard.frag");
		GLCompStatus status = m_bbShader.init(*vertexSource, *fragmentSource);
		if (!status.success)
		{
			m_bbShader.destroy();
			throw std::runtime_error("Failed to initialize ParticleRenderer shader: " + status.log);
		}

		// unit quad as a triangle strip
		constexpr float quad[] = { -0.5f, -0.5f, 0.5f, -0.5f, -0.5f, 0.5f, 0.5f, 0.5f };

		glGenVertexArrays(1, &m_VAO);
		glGenBuffers(1, &m_quadVBO);
		glGenBuffers(1, &m_instanceVBO);

		glBindVertexArray(m_VAO);

		glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);

		glBindBuffer(GL_ARRAY_BUFFER, m_instanceVBO);
		const GLsizei stride = sizeof(ParticleInstance);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(ParticleInstance, position));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(ParticleInstance, size));
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(ParticleInstance, rotation));
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(ParticleInstance, color));
		for (GLuint i = 1; i <= 4; ++i)
			glVertexAttribDivisor(i, 1);

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	void ParticleRenderer::render(RenderContext& context)
	{
		const auto& particles = context.frame.particles;
		if (particles.empty())
			return;

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glDepthMask(GL_FALSE); // test against the scene, but do not write
		glDisable(GL_CULL_FACE);
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		
		// additive and alpha blend modes
		if (blendMode == ParticleBlendMode::Additive) {
			glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		}
		else {
			glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		}

		const CameraFrustum& camera = context.viewport.getCameraFrustum();
		m_bbShader.bind();
		m_bbShader.setMat4("view", camera.viewMatrix);
		m_bbShader.setMat4("projectionView", camera.viewProjectionMatrix);

		//rendered with instancing
		glBindVertexArray(m_VAO);
		glBindBuffer(GL_ARRAY_BUFFER, m_instanceVBO);
		glBufferData(GL_ARRAY_BUFFER, particles.size() * sizeof(ParticleInstance), particles.data(), GL_STREAM_DRAW);
		glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, static_cast<GLsizei>(particles.size()));
		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		m_bbShader.unbind();

		glDisable(GL_BLEND);
		glEnable(GL_CULL_FACE);
		glDepthMask(GL_TRUE);
		glDepthFunc(GL_LESS);
	}

	void ParticleRenderer::shutdown()
	{
		if (!m_VAO)
			return;

		m_logger.info("Shutting down");
		glDeleteBuffers(1, &m_instanceVBO);
		glDeleteBuffers(1, &m_quadVBO);
		glDeleteVertexArrays(1, &m_VAO);
		m_instanceVBO = m_quadVBO = m_VAO = 0;
		m_bbShader.destroy();
	}
} // namespace SE
