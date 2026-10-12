#pragma once

namespace SE
{
	// @brief Base class for all ImGui widgets
	class Widget
	{
	public:
		Widget(std::string_view name)
		    : m_name(std::string(name)){};
		virtual ~Widget() = default;

		virtual void render() = 0;
		virtual void shutdown() = 0;

		const std::string& getMessage() const { return m_logmesg; }
		const std::string& getName() const { return m_name; }

	protected:
		void _setStatus(std::string_view message) { m_logmesg = std::string(message); }

	private:
		std::string m_logmesg{ "" };
		std::string m_name{ "unnamed widget" };
	};

	template<typename T, bool InitiallyVisible = true>
	class VisibleWidget : public Widget
	{
	public:
		using Widget::Widget;

		static void setVisible(bool visible) { s_visible = visible; }
		static bool isVisible() { return s_visible; }

	protected:
		static inline bool s_visible = InitiallyVisible;
	};
} // namespace SE
