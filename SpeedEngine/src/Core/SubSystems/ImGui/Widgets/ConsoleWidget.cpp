#include "pch.h"

#include "ConsoleWidget.h"
#include "Logger/ConsoleSink.h"

#pragma region Commands
#include "Core/Engine.h"
#include "Core/EngineStates.h"
#include "Core/Subsystems/Graphics/Window.h"
#pragma endregion

namespace SE
{

	ConsoleWidget::ConsoleWidget() : VisibleWidget("ConsoleWidget"), m_autoScroll(true), m_scrollToBottom(false), m_historyPos(-1)
	{
		m_logger.init(getName());

		m_inputBuf[0] = '\0';

// REGISTER YOUR COMMANDS HERE
#pragma region RegisterCommands
		registerCommand("help", [this](CommandArg)
			{
				m_logger.info("Available commands:");
				for (const auto& [name, callback] : m_commandRegistry)
				{
					m_logger.info("  -> {}", name);
				}
			});

		registerCommand("clear", [this](CommandArg)
		    {
			    auto sink = m_logger.getConsoleSink();
			    sink->clear();
		    });

		registerCommand("spawn", [this](CommandArg args)
		    {
			    if (args.size() != 1)
				    m_logger.error("Usage: spawn <arg>");
			    else
					m_logger.info("spawn command with arg: {}", args[0]);
		    });

		registerCommand("resize", [this](CommandArg args)
		    {
				if (args.size() != 2)
				{
				    m_logger.error("Usage: resize <width> <height>");
				    return;
				}

				const auto parseDim = [](std::string_view sv, int& val)
			    {
				    const auto result = std::from_chars(sv.data(), sv.data() + sv.size(), val);
				    return result.ec == std::errc::invalid_argument || result.ec == std::errc::result_out_of_range;
			    };

				int w = 0;
				int h = 0;
			    if (parseDim(args[0], w) || parseDim(args[1], h) || w < 800 || h < 600)
			    {
				    m_logger.error("Width and height must be positive ints");
				    return;
			    }

				Engine::Instance().getSubSystem<Window>()->setSize(w, h);
				m_logger.info("Resized window to {} {}", args[0], args[1]);
		    });

		registerCommand("debug", [this](CommandArg args)
			{
				if (args.size() > 1 || (!args.empty() && args[0] != "on" && args[0] != "off"))
				{
					m_logger.error("Usage: debug [on|off] (no argument toggles)");
					return;
				}

				const bool enabled = args.empty() ? !EngineStates::isDebugRenderingEnabled() : args[0] == "on";
				EngineStates::setDebugRenderingEnabled(enabled);
				m_logger.info("Debug rendering {}", enabled ? "enabled" : "disabled");
			});
#pragma endregion
	}

	void ConsoleWidget::registerCommand(std::string_view name, CommandCallback callback)
	{
		m_commandRegistry.emplace(std::string(name), std::move(callback));
	}

	void ConsoleWidget::_executeCommand(std::string_view commandLine)
	{
		// helper
		const auto Stricmp = [](std::string_view lhs, std::string_view rhs)
		{
			int d;
			const char* s1 = lhs.data();
			const char* s2 = rhs.data();
			while ((d = toupper(*s2) - toupper(*s1)) == 0 && *s1)
			{
				s1++;
				s2++;
			}
			return d;
		};
		const auto Strdup = [](std::string_view text)
		{
			IM_ASSERT(text.data() != nullptr);
			size_t len = text.size() + 1;
			void* buf = ImGui::MemAlloc(len);
			IM_ASSERT(buf);
			return (char*)memcpy(buf, (const void*)text.data(), len);
		};

        // Insert into history
		m_historyPos = -1;
		for (int i = m_history.Size - 1; i >= 0; i--)
			if (Stricmp(m_history[i], commandLine) == 0)
			{
				ImGui::MemFree(m_history[i]);
				m_history.erase(m_history.begin() + i);
				break;
			}
		m_history.push_back(Strdup(commandLine));

		// process the command
		constexpr std::string_view whitespace = " \t\r\n";

		const auto start = commandLine.find_first_not_of(whitespace);
		if (start == std::string_view::npos)
			return;

		m_logger.info("> {}", commandLine);
		m_scrollToBottom = true;

		commandLine.remove_prefix(start);
		const auto separator = commandLine.find_first_of(whitespace);
		const std::string name(commandLine.substr(0, separator));

		std::vector<std::string_view> arguments;
		auto argumentStart = separator;
		while (argumentStart != std::string_view::npos)
		{
			argumentStart = commandLine.find_first_not_of(whitespace, argumentStart);
			if (argumentStart == std::string_view::npos)
				break;

			const auto argumentEnd = commandLine.find_first_of(whitespace, argumentStart);
			arguments.push_back(commandLine.substr(argumentStart, argumentEnd - argumentStart));
			argumentStart = argumentEnd;
		}

		const auto command = m_commandRegistry.find(name);
		if (command == m_commandRegistry.end())
		{
			m_logger.warn("Unknown command: {}", name);
			return;
		}

		command->second(arguments);

	}

	// whole lotta code from imgui_demo.cpp
	void ConsoleWidget::render()
	{
		const bool focusInput = ImGui::IsKeyPressed(ImGuiKey_GraveAccent, false);
		if (focusInput)
			s_visible = true;

		if (!s_visible) return;

		_setStatus("Rendering ConsoleWidget");
		// ImGui::SetNextWindowSize(ImVec2(520, 600), ImGuiCond_FirstUseEver);

		if (focusInput)
		{
			ImGui::SetNextWindowCollapsed(false);
			ImGui::SetNextWindowFocus();
		}

		if (!ImGui::Begin("Console", &s_visible, ImGuiWindowFlags_NoFocusOnAppearing))
		{
			ImGui::End();
			return;
		}

		auto sink = m_logger.getConsoleSink();
		if (ImGui::Button("Clear"))
		{
			sink->clear();
		}

		ImGui::SameLine();
		//ImGui::Checkbox("Auto-scroll", &m_autoScroll);

		m_filter.Draw("Filter");
		ImGui::Separator();

		const auto records = sink->snapshot();
		auto& style = ImGui::GetStyle();
		const float footer_height_to_reserve = style.SeparatorSize + style.ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
		if (ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footer_height_to_reserve), ImGuiChildFlags_NavFlattened, ImGuiWindowFlags_HorizontalScrollbar))
		{
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1)); // Tighten spacing

			for (const auto& record : records)
			{
				if (!m_filter.PassFilter(record.text.c_str()))
					continue;
				
				bool colored = record.level >= spdlog::level::warn;
				if (colored)
					ImGui::PushStyleColor(ImGuiCol_Text, record.level >= spdlog::level::err ? ImVec4(1.0f, 0.4f, 0.4f, 1.0f) : ImVec4(1.0f, 0.8f, 0.3f, 1.0f));
				ImGui::TextUnformatted(record.text.data(), record.text.data() + record.text.size());
				if (colored)
					ImGui::PopStyleColor();
			}
			if (m_scrollToBottom || (m_autoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()))
				ImGui::SetScrollHereY(1.0f);
			m_scrollToBottom = false;
			
			ImGui::PopStyleVar();
		}
		ImGui::EndChild();
		ImGui::Separator();

        ImGuiInputTextFlags input_text_flags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_EscapeClearsAll | ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackHistory;
		if (focusInput)
			ImGui::SetKeyboardFocusHere();
		if (ImGui::InputText("Input", m_inputBuf, sizeof(m_inputBuf), input_text_flags, &ConsoleWidget::_textEditCallback, this))
		{
			_executeCommand(m_inputBuf);
			m_inputBuf[0] = '\0';
			ImGui::SetKeyboardFocusHere(-1);
		}

		ImGui::SetItemDefaultFocus();
		ImGui::End();
	}

	int ConsoleWidget::_textEditCallback(ImGuiInputTextCallbackData* data)
	{
		auto& console = *static_cast<ConsoleWidget*>(data->UserData);
		if (data->EventFlag == ImGuiInputTextFlags_CallbackHistory)
		{
			const int previousPosition = console.m_historyPos;
			if (data->EventKey == ImGuiKey_UpArrow)
			{
				if (console.m_historyPos == -1)
				{
					console.m_historyPos = console.m_history.Size - 1;
				}
				else if (console.m_historyPos > 0)
				{
					--console.m_historyPos;
				}
			}
			else if (data->EventKey == ImGuiKey_DownArrow && console.m_historyPos != -1)
			{
				if (++console.m_historyPos >= console.m_history.Size)
				{
					console.m_historyPos = -1;
				}
			}
			if (console.m_historyPos != previousPosition)
			{
				data->DeleteChars(0, data->BufTextLen);
				if (console.m_historyPos >= 0)
				{
					data->InsertChars(0, console.m_history[console.m_historyPos]);
				}
			}
		}
		else if (data->EventFlag == ImGuiInputTextFlags_CallbackCompletion)
		{
			const std::string_view input(data->Buf, data->BufTextLen);
			const auto start = input.find_first_not_of(" \t");
			if (start == std::string_view::npos || static_cast<size_t>(data->CursorPos) < start)
			{
				return 0;
			}
			const auto separator = input.find_first_of(" \t", start);
			const auto end = separator == std::string_view::npos ? input.size() : separator;
			if (static_cast<size_t>(data->CursorPos) != end)
			{
				return 0;
			}
			const auto prefix = input.substr(start, end - start);
			std::string completion;
			size_t matches = 0;
			for (const auto& [name, callback] : console.m_commandRegistry)
			{
				if (!std::string_view(name).starts_with(prefix))
				{
					continue;
				}
				if (matches++ == 0)
				{
					completion = name;
				}
				else
				{
					size_t length = 0;
					while (length < completion.size() && length < name.size() && completion[length] == name[length])
					{
						++length;
					}
					completion.resize(length);
				}
			}
			if (matches != 0)
			{
				if (matches == 1 && separator == std::string_view::npos)
				{
					completion += ' ';
				}
				data->DeleteChars(static_cast<int>(start), static_cast<int>(end - start));
				data->InsertChars(static_cast<int>(start), completion.c_str());
			}
		}
		return 0;
	}

	void ConsoleWidget::shutdown()
	{
		_setStatus("Shutting down ConsoleWidget");
		for (char* command : m_history)
			ImGui::MemFree(command);
		m_history.clear();
		m_historyPos = -1;
	}
} // namespace SE
