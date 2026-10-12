#pragma once

#include "SubSystems/ImGui/Widget.h"
#include "Logger/Logger.h"

#include <imgui.h>

namespace SE
{
	class ConsoleWidget : public Widget
	{
	public:
		using CommandCallback = std::function<void(std::string_view)>;

		ConsoleWidget();
		void render() override;
		void shutdown() override;

		static void setVisible(bool visible) { s_visible = visible; }
		static bool isVisible() { return s_visible; }

		void registerCommand(std::string_view name, CommandCallback callback);

	private:
		static int _textEditCallback(ImGuiInputTextCallbackData* data);
		void _executeCommand(std::string_view commandLine);

		static inline bool s_visible = true;

		std::unordered_map<std::string, CommandCallback> m_commandRegistry;

		Logger m_logger;

		char m_inputBuf[256];
		ImVector<char*> m_items;
		ImVector<const char*> m_commands;
		ImVector<char*> m_history;
		int m_historyPos; // -1: new line, 0..History.Size-1 browsing history.
		ImGuiTextFilter m_filter;
		bool m_autoScroll;
		bool m_scrollToBottom;
	};
} // namespace SE

