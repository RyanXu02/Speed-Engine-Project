#pragma once
#include "../../SubSystem.h"

#include "../Window.h"

#include "Renderer.h"
#include "SceneRenderer/SceneRenderer.h"
#include "ImGuiRenderer/ImGuiRenderer.h"

#include "Viewport.h"

namespace SE
{
	// @brief Manager for all renderers in the engine
	// @brief Manages the lifecycle of renderers, viewports, and rendering operations
    class RendererManager : public SubSystem
    {
		friend class Engine; // only Engine should create instance of subsystems

    public:
		const Window* getWindow() const { return m_window; }
		void setWindow(Window& window) { m_window = &window; }

		template <typename T> requires std::derived_from<T, Renderer>
        T* getRenderer()
        {
            if (m_sceneRenderer && dynamic_cast<T*>(m_sceneRenderer.get()))
            {
                return static_cast<T*>(m_sceneRenderer.get());
            }
            return nullptr;
        }

		template <typename T> requires std::derived_from<T, Renderer>
		void removeRenderer()
		{
			if (m_sceneRenderer && dynamic_cast<T*>(m_sceneRenderer.get()))
			{
				m_sceneRenderer->shutdown();
				m_sceneRenderer.reset();
			}
		}

		void init() override;
		void update(double deltaTime) override;
		void shutdown() override;

		void render() const;

    private:
		RendererManager() = delete;
		RendererManager(Window& window) : SubSystem("RendererManager"), m_window(&window) {};

		const Window* m_window;

		std::unique_ptr<SceneRenderer> m_sceneRenderer;
		std::unique_ptr<ImGuiRenderer> m_imguiRenderer;
		// subsequent renderers for parts such as ui and debug views
		// ...
    };
}

