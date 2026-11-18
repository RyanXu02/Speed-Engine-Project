#include "pch.h"

#include "RendererManager.h"

#include "Core/ResourceManagers/ResourceManager.h"

#include "Core/SubSystems/Graphics/Rendering/DebugRenderer/DebugRenderer.h"
#include "Core/SubSystems/Graphics/Rendering/ShadowRenderer/ShadowRenderer.h"
#include "Core/SubSystems/Graphics/Rendering/DeferredLightingRenderer/DeferredLightingRenderer.h"
#include "Core/SubSystems/Graphics/Rendering/GeometryRenderer/GeometryRenderer.h"
#include "Core/SubSystems/Graphics/Rendering/ImageBasedRenderer/ImageBasedRenderer.h"
#include "Core/SubSystems/Graphics/Rendering/SceneRenderer/SceneRenderer.h"
#include "Core/SubSystems/Graphics/Rendering/GrayscaleRenderer/GrayscaleRenderer.h"
#include "Core/SubSystems/Graphics/Rendering/TextRenderer/TextRenderer.h"
#include "Core/SubSystems/Graphics/Rendering/TonemapRenderer/TonemapRenderer.h"
#include "Core/SubSystems/Graphics/Rendering/ParticleRenderer/ParticleRenderer.h"
#include "Core/SubSystems/Graphics/Rendering/TerrainRenderer/TerrainRenderer.h"
#include "Core/SubSystems/Graphics/Rendering/WireframeRenderer/WireframeRenderer.h"

#include "Core/ResourceManagers/Material/Material.h"
#include "Core/Engine.h"
#include "Core/EngineStates.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace SE
{
	namespace TextureSlots
	{
		extern constexpr int G_POSITION = 0;
		extern constexpr int G_NORMAL = 1;
		extern constexpr int G_ALBEDO = 2;
		extern constexpr int G_MATERIAL = 3;
		extern constexpr int G_EMISSIVE = 4;
		extern constexpr int SHADOW_MAP = 5;
		extern constexpr int BLUE_NOISE = 6;
		extern constexpr int IBL_PREFILTER = 7;
		extern constexpr int IBL_BRDF_LUT = 8;
		extern constexpr int SKY = 9;

		constexpr int MATERIAL_LAST = 6;	// this needs to be updated to match the material enums
		static_assert(NUM_TEXTURE_TYPES - 1 == MATERIAL_LAST, "update the slot table when adding a TextureType");
		static_assert(MATERIAL_LAST < IBL_PREFILTER, "material maps overlap the IBL slots");
		static_assert(BLUE_NOISE < IBL_PREFILTER, "deferred inputs overlap the IBL slots");
	}

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
#pragma region RenderPasses
		//TODO: potentially a single scattered sky pass here

		// border is 1.0 or max depth, so anything outside the map reads as lit
		const Attachment shadowDepth{ .internalFormat = GL_DEPTH_COMPONENT24, .format = GL_DEPTH_COMPONENT, .type = GL_FLOAT,
			.minFilter = GL_NEAREST, .magFilter = GL_NEAREST, .wrapS = GL_CLAMP_TO_BORDER, .wrapT = GL_CLAMP_TO_BORDER };
		FBO& shadowMap = createRenderTarget("ShadowMap", Descriptor{ .width = 2048, .height = 2048, .colors = {}, .depth = shadowDepth }, RenderTargetSize{ .followSceneViewport = false });
		auto& shadowRenderer = addRenderer<ShadowRenderer>();
		addPass(RenderPass{ .renderer = shadowRenderer, .output = shadowMap, .clearMask = GL_DEPTH_BUFFER_BIT });

		// Geometry pass, opaque submeshes into the G-buffer
		// attachments: 0 position, 1 normal, 2 albedo, 3 ao, metallic, and roughness in rgb channels, 4 emissive
		const Attachment floatAttachment{ .internalFormat = GL_RGBA16F, .format = GL_RGBA, .type = GL_FLOAT, .minFilter = GL_NEAREST, .magFilter = GL_NEAREST };
		const Attachment byteAttachment{ .minFilter = GL_NEAREST, .magFilter = GL_NEAREST };
		FBO& gBuffer = createRenderTarget("GBuffer", Descriptor{ .colors = { floatAttachment, floatAttachment, byteAttachment, byteAttachment, floatAttachment } });
		auto& geometryRenderer = addRenderer<GeometryRenderer>();
		addPass(RenderPass{ .renderer = geometryRenderer, .output = gBuffer, .clearMask = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, .clearColor = glm::vec4(0.0f) });

		// Geometry pass for terrain, draws on top of gBuffer
		auto& terrainRenderer = addRenderer<TerrainRenderer>();
		addPass(RenderPass{ .renderer = terrainRenderer, .output = gBuffer, .clearMask = 0 });

		//TODO: add an AO pass here

		// Image based lighting, no pass of its own, interally precomputes its maps once in init and the lighting passes read them
		addRenderer<ImageBasedRenderer>("Assets/Textures/photo_studio_loft_hall_2k.hdr");

		// lighting stays linear and unclamped in half floats until the tonemap pass, 8 bit here would band
		const Attachment hdrAttachment{ .internalFormat = GL_RGBA16F, .format = GL_RGBA, .type = GL_FLOAT };
		FBO& deferredColor = createRenderTarget("DeferredColor", Descriptor{ .colors = { hdrAttachment } });
		auto& deferredLightingRenderer = addRenderer<DeferredLightingRenderer>("GBuffer", "ShadowMap");
		addPass(RenderPass{ .renderer = deferredLightingRenderer, .output = deferredColor, .clearMask = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT });

		// Forward pass, copies DeferredColor then draws transparent submeshes sorted back to front
		FBO& sceneColor = createRenderTarget("SceneColor", Descriptor{ .colors = { hdrAttachment } });
		auto& sceneRenderer = addRenderer<SceneRenderer>("DeferredColor");
		addPass(RenderPass{ .renderer = sceneRenderer, .output = sceneColor, .clearMask = 0 });

		// Billboard particles draw on top of SceneColor, depth tested against the scene
		auto& particleRenderer = addRenderer<ParticleRenderer>();
		addPass(RenderPass{
			.renderer = particleRenderer,
			.output = sceneColor,
			.clearMask = 0 });

		// Tonemap pass, HDR SceneColor to the 8 bit display ready FinalColor, also copies the depth
		FBO& finalColor = createRenderTarget("FinalColor", Descriptor{});
		auto& tonemapRenderer = addRenderer<TonemapRenderer>("SceneColor");
		addPass(RenderPass{ .renderer = tonemapRenderer, .output = finalColor, .clearMask = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT });

		// Wireframe renderer, discards previous passes
		auto& wireframeRenderer = addRenderer<WireframeRenderer>();
		addPass(RenderPass{ .renderer = wireframeRenderer, .output = finalColor, .clearMask = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, .wireframeOnly = true });

		// Example: A render pass that draws on top of FinalColor, after the tonemap so debug colors show as is
		auto& debugRenderer = addRenderer<DebugRenderer>();
		addPass(RenderPass{ .renderer = debugRenderer,  .output = finalColor,  .clearMask = 0, .debugOnly = true });

		// Example: A render pass that samples "FinalColor" and output to a new "Grayscale" target
		Descriptor grayscaleDescriptor{
			.colors = { Attachment{} },
			.depth = std::nullopt };
		FBO& grayscale = createRenderTarget("Grayscale", grayscaleDescriptor);
		auto& grayscaleRenderer = addRenderer<GrayscaleRenderer>("FinalColor"); // make your own renderer that consumes some target
		addPass(RenderPass{ .renderer = grayscaleRenderer,  .output = grayscale,  .clearMask = GL_COLOR_BUFFER_BIT });

		// Example:
		// What an AO pass could look like:
		/*
			// Compute AO mask
			Descriptor aoDescriptor{
				.colors = { 
					Attachment{
						.internalFormat = GL_R8,
						.format = GL_RED,
						.type = GL_UNSIGNED_BYTE 
					}
				},
				.depth = std::nullopt
			};
			aoDescriptor.depth.reset();
			aoDescriptor.colors.emplace_back(Attachment{ GL_R8, GL_RED, GL_UNSIGNED_BYTE });
			FBO& ao = createRenderTarget("AO", aoDescriptor);
			// Sample SceneColor's depth attachment
			auto& aoRenderer = addRenderer<SSAORenderer>("SceneColor");
			addPass(RenderPass{ 
				.renderer = aoRenderer, 
				.output = ao, 
				.clearMask = GL_COLOR_BUFFER_BIT, 
				.clearColor = glm::vec4(1.0f) });

			// Combine the scene color with the AO mask.
			Descriptor compositeDescriptor;
			compositeDescriptor.depth.reset();
			FBO& sceneWithAO = createRenderTarget("SceneWithAO", compositeDescriptor);
			// Sample both input targets
			auto& compositeRenderer = addRenderer<AOCompositeRenderer>("SceneColor", "AO");
			addPass(RenderPass{ 
				.renderer = compositeRenderer, 
				.output = sceneWithAO, 
				.clearMask = GL_COLOR_BUFFER_BIT });

			setFinalSceneOutput("SceneWithAO");
		*/

		// change this to set the final output to a specific target
		setFinalSceneOutput("FinalColor");
#pragma endregion

		m_textRenderer = &addRenderer<TextRenderer>();

		// imgui always presents after the scene pipeline on the default framebuffer
		m_imguiRenderer = std::unique_ptr<ImGuiRenderer>(new ImGuiRenderer(*m_logger, *m_window));
		m_imguiRenderer->init();
	}

	void RendererManager::render(const RenderFrame& frame, float t)
	{
		int width = 0;
		int height = 0;
		glfwGetFramebufferSize(m_window->getWindow(), &width, &height);
#if !defined(COMPILE_WITH_EDITOR)
		requestSceneResize(width, height);
#endif
		_applySceneResize();

		m_sceneViewport->setCamera(interpolateCamera(frame.camera.previous, frame.camera.current, t));
		m_sceneViewport->updateMatrices();
		RenderContext context{ frame,
			*m_sceneViewport, *Engine::Instance().getSubSystem<ResourceManager>(), *m_meshes, m_gpu, m_targets, m_sun, t };

		for (const auto& pass : m_passes)
		{
			if (pass.debugOnly && !EngineStates::isDebugRenderingEnabled())
				continue;
			// Don't do extra passes when wireframe mode is on
			if (pass.wireframeOnly && !EngineStates::isWireframeRenderingEnabled() || !pass.wireframeOnly && EngineStates::isWireframeRenderingEnabled())
				continue;

			resetPassState();
			pass.output.bind(); // bind the pass's output
			glViewport(0, 0, pass.output.getWidth(), pass.output.getHeight());
			glClearColor(pass.clearColor.r, pass.clearColor.g, pass.clearColor.b, pass.clearColor.a);
			glClearDepth(pass.clearDepth);
			glClear(pass.clearMask);
			context.output = &pass.output;
			pass.renderer.render(context);
		}

		// UI and text go on top the selected final output
		if (!frame.uiDraws.empty() || !frame.textDraws.empty())
		{
			resetPassState();
			m_finalSceneOutput->bind();
			glViewport(0, 0, m_finalSceneOutput->getWidth(), m_finalSceneOutput->getHeight());
			GLint count = 0;
			glGetIntegerv(GL_MAX_DRAW_BUFFERS, &count);
			std::vector<GLenum> drawBuffers(count);
			for (GLint i = 0; i < count; ++i)
			{
				GLint buffer = 0;
				glGetIntegerv(GL_DRAW_BUFFER0 + i, &buffer);
				drawBuffers[i] = static_cast<GLenum>(buffer);
			}
			glDrawBuffer(GL_COLOR_ATTACHMENT0 + static_cast<GLenum>(m_finalColorAttachment));
			context.output = m_finalSceneOutput;
			m_textRenderer->render(context);
			glDrawBuffers(count, drawBuffers.data());
		}

		resetPassState();
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, width, height);
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClearDepth(1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Show final scene output to the default framebuffer if not in editor mode
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

		_prepareGameInputRegion(width, height);

		m_imguiRenderer->beginFrame();
		m_imguiRenderer->render(context);

		_publishGameInputRegion();

		m_imguiRenderer->finalizeFrame();


		m_window->swapBuffers();
	}

	void RendererManager::_prepareGameInputRegion([[maybe_unused]] int framebufferWidth, [[maybe_unused]] int framebufferHeight)
	{
#if defined(COMPILE_WITH_EDITOR)
		m_gameInputRegion = {};
#else
		int windowWidth = 0;
		int windowHeight = 0;
		glfwGetWindowSize(m_window->getWindow(), &windowWidth, &windowHeight);
		setGameInputRegion({ { 0.0f, 0.0f },
			{ static_cast<float>(windowWidth), static_cast<float>(windowHeight) }, getFinalSceneSize(),
			windowWidth > 0 && windowHeight > 0 && framebufferWidth > 0 && framebufferHeight > 0 });
#endif
	}

	void RendererManager::_publishGameInputRegion()
	{
#if defined(COMPILE_WITH_EDITOR)
		m_gameInputRegion.acceptsInput &= EngineStates::isGameInputActive();
#endif
		m_gameInputRegion.acceptsInput &= !m_imguiRenderer->isBlockingGameInput(m_scenePreviewItemActive);
		InputEvent inputRegionEvent{ InputEvent::Type::InputRegion };
		inputRegionEvent.inputRegion = m_gameInputRegion;
		Engine::Instance().getSubSystem<Window>()->inputEvents().push(inputRegionEvent);
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
		for (auto& target : m_targets | std::views::values)
		{
			if (target.size.followSceneViewport)
			{
				target.framebuffer->resize(
					std::max(1u, static_cast<uint32_t>(width * target.size.scale)),
					std::max(1u, static_cast<uint32_t>(height * target.size.scale))
				);
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
		m_textRenderer = nullptr;
		m_renderers.clear();
		m_meshes.reset();
		m_gpu.clear();
		m_finalSceneOutput = nullptr;
		m_targets.clear();
		m_sceneViewport.reset();
		m_pendingSceneSize.reset();
		SubSystem::shutdown();
	}

} // namespace SE
