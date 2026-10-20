#include "pch.h"

#include "RendererManager.h"

#include "Core/ResourceManagers/ResourceManager.h"
#include "Core/SubSystems/Scenes/SceneSystem.h"
#include "Core/SubSystems/Graphics/Rendering/DebugRenderer/DebugRenderer.h"
#include "Core/SubSystems/Graphics/Rendering/SceneRenderer/SceneRenderer.h"
#include "Core/Engine.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace SE
{
	// Common pass baseline
	// Passes bind their own inputs and restore their state beyond this list
	static void resetPassState()
	{
		glDisable(GL_BLEND);
		glDisable(GL_STENCIL_TEST);
		glDisable(GL_SCISSOR_TEST);
		glDisable(GL_POLYGON_OFFSET_FILL);
		glDisable(GL_RASTERIZER_DISCARD);
		glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
		glDisable(GL_FRAMEBUFFER_SRGB);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glDepthMask(GL_TRUE);
		glDepthRange(0.0, 1.0);
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glStencilMask(~0u);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glFrontFace(GL_CCW);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glUseProgram(0);
		glBindVertexArray(0);
		glActiveTexture(GL_TEXTURE0);
	}

	void RendererManager::init()
	{
		SubSystem::init();

		int width = 0;
		int height = 0;
		glfwGetFramebufferSize(m_window->getWindow(), &width, &height);
		m_sceneViewport = std::make_unique<Viewport>(std::max(1, width), std::max(1, height));
		m_meshes = std::make_unique<MeshGPUCache>(*m_logger);

		// Configure your targets, execution order, outputs, and clears here
		FBO& sceneColor = createRenderTarget("SceneColor", Descriptor{});

		auto& sceneRenderer = addRenderer<SceneRenderer>();
		addPass({ sceneRenderer, sceneColor, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT });

		auto& debugRenderer = addRenderer<DebugRenderer>();
		addPass({ debugRenderer, sceneColor, 0 });

		setFinalSceneOutput("SceneColor");

		// imgui always presents after the scene pipeline on the default framebuffer
		m_imguiRenderer = std::unique_ptr<ImGuiRenderer>(new ImGuiRenderer(*m_logger, *m_window));
		m_imguiRenderer->init();
	}

	void RendererManager::update(double deltaTime)
	{
		SubSystem::update(deltaTime);
		RenderContext context{ Engine::Instance().getSubSystem<SceneSystem>()->getCurrentScene(),
			*m_sceneViewport, *Engine::Instance().getSubSystem<ResourceManager>(), *m_meshes, m_targets };

		for (auto& renderer : m_renderers)
			renderer->update(deltaTime, context);

		m_sceneViewport->updateMatrices();
	}

	void RendererManager::render()
	{
		int width = 0;
		int height = 0;
		glfwGetFramebufferSize(m_window->getWindow(), &width, &height);
		requestSceneResize(width, height);
		_applySceneResize();

		m_sceneViewport->updateMatrices();
		RenderContext context{ Engine::Instance().getSubSystem<SceneSystem>()->getCurrentScene(),
			*m_sceneViewport, *Engine::Instance().getSubSystem<ResourceManager>(), *m_meshes, m_targets };

		for (const auto& pass : m_passes)
		{
			resetPassState();
			pass.output.bind();
			glViewport(0, 0, pass.output.getWidth(), pass.output.getHeight());
			glClearColor(pass.clearColor.r, pass.clearColor.g, pass.clearColor.b, pass.clearColor.a);
			glClearDepth(pass.clearDepth);
			glClear(pass.clearMask);
			context.output = &pass.output;
			pass.renderer.render(context);
		}

		resetPassState();
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, width, height);
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClearDepth(1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Show final scene output to the default framebuffer (window) if not in editor mode
#if !defined(COMPILE_WITH_EDITOR)
		if (width > 0 && height > 0)
		{
			glBindFramebuffer(GL_READ_FRAMEBUFFER, m_finalSceneOutput->getID());
			glReadBuffer(GL_COLOR_ATTACHMENT0 + static_cast<GLenum>(m_finalColorAttachment));
			glBlitFramebuffer(0, 0, m_finalSceneOutput->getWidth(), m_finalSceneOutput->getHeight(),
				0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_LINEAR);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
#endif
		context.output = nullptr;
		m_imguiRenderer->beginFrame();
		m_imguiRenderer->render(context);
		m_imguiRenderer->finalizeFrame();
		m_window->swapBuffers();
	}

	FBO& RendererManager::createRenderTarget(const std::string& name, Descriptor descriptor, RenderTargetSize size)
	{
		if (m_targets.contains(name))
			throw std::invalid_argument("Render target already exists: " + name);
		if (size.scale <= 0.0f)
			throw std::invalid_argument("Render target scale must be positive");

		if (size.followSceneViewport)
		{
			descriptor.width = std::max(1u, static_cast<uint32_t>(m_sceneViewport->getWidth() * size.scale));
			descriptor.height = std::max(1u, static_cast<uint32_t>(m_sceneViewport->getHeight() * size.scale));
		}
		auto framebuffer = std::make_unique<FBO>(descriptor);
		FBO& result = *framebuffer;
		m_targets.emplace(name, RenderTarget{ std::move(framebuffer), size });
		return result;
	}

	void RendererManager::setFinalSceneOutput(const std::string& target, size_t colorAttachment)
	{
		FBO& framebuffer = getRenderTarget(target);
		framebuffer.getColorTexture(colorAttachment);
		m_finalSceneOutput = &framebuffer;
		m_finalColorAttachment = colorAttachment;
	}

	void RendererManager::requestSceneResize(uint32_t width, uint32_t height)
	{
		if (width > 0 && height > 0)
			m_pendingSceneSize = { width, height };
	}

	void RendererManager::_applySceneResize()
	{
		if (!m_pendingSceneSize)
			return;

		auto [width, height] = *m_pendingSceneSize;
		m_pendingSceneSize.reset();
		if (width == m_sceneViewport->getWidth() && height == m_sceneViewport->getHeight())
			return;

		m_sceneViewport->resize(width, height);
		for (auto& [name, target] : m_targets)
		{
			if (target.size.followSceneViewport)
			{
				target.framebuffer->resize(std::max(1u, static_cast<uint32_t>(width * target.size.scale)),
					std::max(1u, static_cast<uint32_t>(height * target.size.scale)));
			}
		}
	}

	void RendererManager::shutdown()
	{
		if (m_imguiRenderer)
		{
			m_imguiRenderer->shutdown();
			m_imguiRenderer.reset();
		}
		m_passes.clear();
		for (auto& renderer : m_renderers | std::views::reverse)
			renderer->shutdown();
		m_renderers.clear();
		m_meshes.reset();
		m_finalSceneOutput = nullptr;
		m_targets.clear();
		m_sceneViewport.reset();
		m_pendingSceneSize.reset();
		SubSystem::shutdown();
	}

} // namespace SE
