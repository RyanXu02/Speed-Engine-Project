#pragma once

#include "SubSystems/ImGui/Widget.h"
#include "Logger/Logger.h"

#include <imgui.h>

namespace SE
{
	class ConsoleWidget : public VisibleWidget<ConsoleWidget>
	{
	public:
		using CommandArg = const std::vector<std::string_view>&;
		using CommandCallback = std::function<void(CommandArg)>;

		ConsoleWidget();
		void render() override;
		void shutdown() override;

		// @brief register a command with the console
		// @param name the name of the command, used to invoke the command
		// @param callback the function to call when the command is invoked
		void registerCommand(std::string_view name, CommandCallback callback);

	private:
		// @brief used for command history and auto complete
		static int _textEditCallback(ImGuiInputTextCallbackData* data);
		// @brief executes a command, called when enter key
		void _executeCommand(std::string_view commandLine);

		std::unordered_map<std::string, CommandCallback> m_commandRegistry;

		Logger m_logger;

		char m_inputBuf[256];
		ImVector<char*> m_items;
		ImVector<const char*> m_commands;
		ImVector<char*> m_history;
		int m_historyPos;
		ImGuiTextFilter m_filter;
		bool m_autoScroll;
		bool m_scrollToBottom;
	};
} // namespace SE

