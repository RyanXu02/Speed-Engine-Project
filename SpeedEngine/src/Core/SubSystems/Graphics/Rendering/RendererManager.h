#pragma once
#include "Core/SubSystems/SubSystem.h"
#include "Core/SubSystems/Graphics/Window.h"
#include "Core/SubSystems/Graphics/Rendering/ImGuiRenderer/ImGuiRenderer.h"
#include "Core/SubSystems/Graphics/Rendering/RenderContext.h"
#include "Core/SubSystems/Graphics/Rendering/Renderer.h"

namespace SE
{
	// @brief Use this to define a render pass
	// @brief A render pass is a single rendering operation that uses a specific renderer and outputs to a specific FBO
	// @param renderer The renderer to use for this pass
	// @param output The FBO to render to
	// @param clearMask What to clear the current bound FBO to, default is GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT
	// @param clearColor The color to clear the output FBO to, default is (0.1, 0.1, 0.1, 1.0)
	// @param clearDepth The depth to clear the output FBO to, default is 1.0
	struct RenderPass
	{
		Renderer& renderer;
		FBO& output;
		GLbitfield clearMask = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT;
		glm::vec4 clearColor{ 0.1f, 0.1f, 0.1f, 1.0f };
		double clearDepth = 1.0;
	};

	// @brief Manager for all renderers in the engine
	// @brief Manages the lifecycle of renderers, viewports, and rendering operations
	class RendererManager : public SubSystem
	{
		friend class Engine; // only Engine should create instance of subsystems

	public:
		const Window* getWindow() const { return m_window; }
		void setWindow(Window& window) { m_window = &window; }

		template<typename T>
		requires std::derived_from<T, Renderer> T* getRenderer()
		{
			for (auto& renderer : m_renderers)
			{
				if (auto* result = dynamic_cast<T*>(renderer.get()))
					return result;
			}

			// imgui renderer is special
			return dynamic_cast<T*>(m_imguiRenderer.get());
		}

		template<typename T, typename... Args>
		requires std::derived_from<T, Renderer> T& addRenderer(Args&&... args)
		{
			auto renderer = std::make_unique<T>(*m_logger, std::forward<Args>(args)...);
			renderer->init();
			T& result = *renderer;
			m_renderers.push_back(std::move(renderer));
			return result;
		}
		// @brief Creates a render target to be used in a render pass
		// @param name The name of the render target
		// @param descriptor The descriptor of the render target, includes width, height, and attachments
		// @param size The size of the render target, either following the scene viewport or using a fixed scale
		// @return A reference to the created FBO
		FBO& createRenderTarget(const std::string& name, Descriptor descriptor, RenderTargetSize size = {});
		FBO& getRenderTarget(const std::string& name) const { return *m_targets.at(name).framebuffer; }

		// @brief Adds a render pass to the renderer manager
		// @param pass The render pass to add
		// @param 
		void addPass(RenderPass pass) { m_passes.push_back(pass); }

		// @brief Sets the final scene output to be presented to the window
		// @param target The name of the render target to use as the final scene output
		// @param colorAttachment The color attachment index of the render target to use as the final scene output
		void setFinalSceneOutput(const std::string& target, size_t colorAttachment = 0);

		// @brief Gets the final scene output texture to be presented to the window
		// @return The texture ID of the final scene output
		uint32_t getFinalSceneTexture() const { return m_finalSceneOutput->getColorTexture(m_finalColorAttachment); }

		// @brief Gets the scene viewport, which defines the size and camera of the scene being rendered
		// @return A reference to the scene viewport
		Viewport& getSceneViewport() const { return *m_sceneViewport; }

		void requestSceneResize(uint32_t width, uint32_t height);

		void init() override;
		void update(double deltaTime) override;
		void shutdown() override;

		void render();

	private:
		explicit RendererManager(Window& window) : SubSystem("RendererManager"), m_window(&window){};

		const Window* m_window;

		void _applySceneResize();

		std::unique_ptr<Viewport> m_sceneViewport;

		std::unique_ptr<MeshGPUCache> m_meshes;

		RenderTargets m_targets;

		std::vector<std::unique_ptr<Renderer>> m_renderers;
		std::unique_ptr<ImGuiRenderer> m_imguiRenderer; // imgui is special

		std::vector<RenderPass> m_passes;

		FBO* m_finalSceneOutput = nullptr;
		size_t m_finalColorAttachment = 0;

		std::optional<std::pair<uint32_t, uint32_t>> m_pendingSceneSize;
	};
} // namespace SE
