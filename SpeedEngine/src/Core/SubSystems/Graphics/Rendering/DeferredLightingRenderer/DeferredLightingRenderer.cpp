#include "pch.h"

#include "DeferredLightingRenderer.h"
#include "Core/Engine.h"
#include "Core/ResourceManagers/ResourceManager.h"
#include "Core/SubSystems/Graphics/Rendering/RenderContext.h"
#include "Core/SubSystems/Graphics/Rendering/RendererManager.h"
#include "Core/SubSystems/Graphics/Rendering/SceneRenderer/SceneRenderer.h"
#include "Core/SubSystems/Graphics/Rendering/ShadowRenderer/ShadowRenderer.h"

#include <glad/glad.h>

namespace SE
{
	// sampler names in deferred_lighting.frag, in G-buffer attachment order (see RendererManager::init)
	static constexpr std::array<std::string_view, 5> GBUFFER_SAMPLERS = {
		"gPosition",
		"gNormal",
		"gAlbedo",
		"gMaterial",
		"gEmissive",
	};

	// shadow map goes in the slot right after the G-buffer
	static constexpr int SHADOW_MAP_SLOT = static_cast<int>(GBUFFER_SAMPLERS.size());

	DeferredLightingRenderer::DeferredLightingRenderer(Logger& logger, std::string gBufferTarget, std::string shadowMapTarget)
	    : Renderer(logger, "DeferredLightingRenderer"),
	      m_gBufferTarget(std::move(gBufferTarget)),
	      m_shadowMapTarget(std::move(shadowMapTarget)),
	      m_shader(0, "DeferredLightingRenderer")
	{}

	void DeferredLightingRenderer::init()
	{
		m_logger.info("Initializing");

		auto* resources = Engine::Instance().getSubSystem<ResourceManager>();
		const std::string* vertexSource = resources->getString("Assets/Shaders/deferredLighting.vert");
		const std::string* fragmentSource = resources->getString("Assets/Shaders/deferredLighting.frag");
		GLCompStatus status = m_shader.init(*vertexSource, *fragmentSource);
		if (!status.success) {
			m_shader.destroy();
			throw std::runtime_error("Failed to initialize DeferredLightingRenderer shader: " + status.log);
		}
		glGenVertexArrays(1, &m_VAO);
		if (!m_VAO) {
			shutdown();
			throw std::runtime_error("Failed to create DeferredLightingRenderer vertex array");
		}
	}

	void DeferredLightingRenderer::render(RenderContext& context)
	{
		// from geometry renderer pass
		const FBO& gBuffer = context.getRenderTarget(m_gBufferTarget);
		const CameraFrustum& camera = context.viewport.getCameraFrustum();

		glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);

		m_shader.bind();
		for (size_t slot = 0; slot < GBUFFER_SAMPLERS.size(); ++slot) {
			m_shader.setInt(GBUFFER_SAMPLERS[slot], static_cast<int>(slot));
			glActiveTexture(GL_TEXTURE0 + static_cast<GLenum>(slot));
			glBindTexture(GL_TEXTURE_2D, gBuffer.getColorTexture(slot));
		}
		m_shader.setVec3("camPos", camera.getPosition());

		// from shadow pass
		const FBO& shadowMap = context.getRenderTarget(m_shadowMapTarget);
		m_shader.setInt("shadowMap", SHADOW_MAP_SLOT);
		glActiveTexture(GL_TEXTURE0 + SHADOW_MAP_SLOT);
		glBindTexture(GL_TEXTURE_2D, shadowMap.getDepthTexture());
		if (auto* shadowRenderer = Engine::Instance().getSubSystem<RendererManager>()->getRenderer<ShadowRenderer>()) {
			m_shader.setMat4("lightSpaceMatrix", shadowRenderer->getLightSpaceMatrix());
			m_shader.setFloat("shadowBias", shadowRenderer->depthBias);
			m_shader.setFloat("shadowNormalOffset", shadowRenderer->normalOffset);
		}

		if (auto* sceneRenderer = Engine::Instance().getSubSystem<RendererManager>()->getRenderer<SceneRenderer>()) {
			sceneRenderer->setLightUniforms(context, m_shader);
		}

		glBindVertexArray(m_VAO);
		glDrawArrays(GL_TRIANGLES, 0, 3);

		glBindVertexArray(0);
		for (size_t slot = 0; slot < GBUFFER_SAMPLERS.size(); ++slot) {
			glActiveTexture(GL_TEXTURE0 + static_cast<GLenum>(slot));
			glBindTexture(GL_TEXTURE_2D, 0);
		}
		glActiveTexture(GL_TEXTURE0 + SHADOW_MAP_SLOT);
		glBindTexture(GL_TEXTURE_2D, 0);
		glActiveTexture(GL_TEXTURE0);
		m_shader.unbind();

		// copy opaque depth so passes reading this output are occluded correctly
		glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer.getID());
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, context.output->getID());
		glBlitFramebuffer(0, 0, gBuffer.getWidth(), gBuffer.getHeight(),
			0, 0, context.output->getWidth(), context.output->getHeight(), GL_DEPTH_BUFFER_BIT, GL_NEAREST);
		context.output->bind();

		glEnable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);
	}

	void DeferredLightingRenderer::shutdown()
	{
		m_logger.info("Shutting down");
		glDeleteVertexArrays(1, &m_VAO);
		m_VAO = 0;
		m_shader.destroy();
	}
} // namespace SE
