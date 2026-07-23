#pragma once
#include "../Renderer.h"

#include "../../../ImGui/Widget.h"

namespace SE
{
	class Window;

    class ImGuiRenderer : public Renderer
    {
	public:
		ImGuiRenderer(Logger& logger, const Window& window) : Renderer(logger, "ImGuiRenderer"), m_window(&window) {}

		void init() override;
		void initWidgets();
		void update(double deltaTime) override;
		void render(Viewport& viewport) const override;
		void shutdown() override;

		template <typename T> requires std::derived_from<T, Widget>
		uint32_t createWidget()
		{
			uint32_t id = _assignID();
			m_widgets[id] = std::make_unique<T>();
			return id;
		}

		template <typename T> requires std::derived_from<T, Widget>
		T* getWidget(uint32_t id)
		{
			auto it = m_widgets.find(id);
			if (it != m_widgets.end())
			{
				return dynamic_cast<T*>(it->second.get());
			}
			return nullptr;
		}

		void destroyWidget(uint32_t id)
		{
			auto it = m_widgets.find(id);
			if (it != m_widgets.end())
			{
				it->second.reset();
				m_widgets.erase(it);
			}
		}

		void finalizeFrame() const;

	private:
		std::unordered_map<uint32_t, std::unique_ptr<Widget>> m_widgets;

		std::atomic<uint32_t> m_ids{ 0 };
		uint32_t _assignID() { return m_ids.fetch_add(1, std::memory_order_relaxed); }

		const Window* m_window{ nullptr };


	};
}

