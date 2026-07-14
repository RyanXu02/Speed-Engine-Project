#pragma once
#include <typeindex>

#include "../../SubSystem.h"
#include "Renderer.h"
#include "RenderContext.h"

namespace SE
{
    class RendererManager : public SubSystem
    {
    public:
		RendererManager() = delete;
		RendererManager(const Window& window) : SubSystem("RendererManager"), m_window(&window) {};
		~RendererManager() {};

		const Window* getWindow() const { return m_window; }
		void setWindow(const Window& window) { m_window = &window; }

		void addRenderer(std::unique_ptr<Renderer> renderer, std::unique_ptr<RenderContext> context);

		template <typename T> requires std::derived_from<T, Renderer>
        T* getRenderer()
        {
			auto it = std::ranges::find_if(m_renderers, [&](const std::unique_ptr<Renderer>& renderer) {
				return dynamic_cast<T*>(renderer.get()) != nullptr;
				});
			if (it != m_renderers.end())
				return static_cast<T*>(it->get());
			return nullptr;
        }

		template <typename T> requires std::derived_from<T, Renderer>
		void removeRenderer()
		{
			auto it = std::ranges::find_if(m_renderers, [&](const std::unique_ptr<Renderer>& renderer) {
				return dynamic_cast<T*>(renderer.get()) != nullptr;
				});
			if (it != m_renderers.end())
			{
				auto type = std::type_index(typeid(**it));
				m_rendererMap.erase(type);
				m_renderers.erase(it);

			}

		}

		template <typename T> requires std::derived_from<T, Renderer>
		void modifyRendererContext(std::unique_ptr<RenderContext> newContext)
		{
			auto it = std::ranges::find_if(m_renderers, [&](const std::unique_ptr<Renderer>& renderer) {
				return dynamic_cast<T*>(renderer.get()) != nullptr;
				});
			if (it != m_renderers.end())
			{
				auto type = std::type_index(typeid(**it));
				m_rendererMap[type] = std::move(newContext);
			}
		}

		void init() override;
		void update(double deltaTime) override;
		void shutdown() override;

    private:
		const Window* m_window;

		std::vector<std::unique_ptr<Renderer>> m_renderers;
		std::unordered_map<std::type_index, std::unique_ptr<RenderContext>> m_rendererMap;
    };
}

