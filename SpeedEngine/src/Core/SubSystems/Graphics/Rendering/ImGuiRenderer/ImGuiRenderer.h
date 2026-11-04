#pragma once
#include "Core/SubSystems/ImGui/Widget.h"
#include "Core/SubSystems/Graphics/Rendering/Renderer.h"
#include "Core/SubSystems/Input/InputEvent.h"

namespace SE
{
	class Window;

	// @brief Renderer that uses ImGui to render widgets
	class ImGuiRenderer : public Renderer
	{
		friend class RendererManager;

	public:
		void init() override;
		void initWidgets();
		void beginFrame();
		void render(RenderContext& context) override;
		void shutdown() override;

		void setInputRegion(const InputRegion& inputRegion, bool itemActive)
		{
			m_inputRegion = inputRegion;
			m_inputRegionItemActive = itemActive;
		}

		// @brief Creates a widget of the specified type
		// @tparam T The type of the widget to create
		// @return The ID of the created widget
		template<typename T>
		requires std::derived_from<T, Widget> uint32_t createWidget()
		{
			uint32_t id = _assignID();
			m_widgets[id] = std::make_unique<T>();
			return id;
		}

		template<typename T>
		requires std::derived_from<T, Widget> T* getWidget(uint32_t id)
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
		ImGuiRenderer(Logger& logger, const Window& window)
		    : Renderer(logger, "ImGuiRenderer"),
		      m_window(&window)
		{}

		std::unordered_map<uint32_t, std::unique_ptr<Widget>> m_widgets;

#if defined(COMPILE_WITH_EDITOR)
		uint32_t m_mainWidgetId = 0;
#endif

		std::atomic<uint32_t> m_ids{ 0 };
		uint32_t _assignID() { return m_ids.fetch_add(1, std::memory_order_relaxed); }

		const Window* m_window{ nullptr };

		InputRegion m_inputRegion;
		bool m_inputRegionItemActive{ false };
	};
} // namespace SE
