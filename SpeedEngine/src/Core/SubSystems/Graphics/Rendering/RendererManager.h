#pragma once
#include "../../SubSystem.h"

#include "../Window.h"

#include "Renderer.h"
#include "SceneRenderer/SceneRenderer.h"
#include "ImGuiRenderer/ImGuiRenderer.h"

#include "Viewport.h"

namespace SE
{
    class RendererManager : public SubSystem
    {
    public:
		RendererManager() = delete;
		RendererManager(Window& window) : SubSystem("RendererManager"), m_window(&window) {};
		~RendererManager() {};

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

		// viewport management
		uint32_t createViewport(uint32_t width, uint32_t height);
		void destroyViewport(uint32_t viewportId);
		Viewport* getViewport(uint32_t viewportId);

		void init() override;
		void update(double deltaTime) override;
		void shutdown() override;

		void render() const;

    private:
		const Window* m_window;

		std::unique_ptr<SceneRenderer> m_sceneRenderer;
		std::unique_ptr<ImGuiRenderer> m_imguiRenderer;
		// subsequent renderers for parts such as ui and debug views
		// ...

		std::unordered_map<uint32_t, std::unique_ptr<Viewport>> m_viewports;
    };
}

