#include "pch.h"

#include "ConsoleWidget.h"
#include "Logger/ConsoleSink.h"

namespace SE
{

	ConsoleWidget::ConsoleWidget() : VisibleWidget("ConsoleWidget"), m_autoScroll(true), m_scrollToBottom(false), m_historyPos(-1)
	{
		m_logger.init(getName());

		m_inputBuf[0] = '\0';

		registerCommand("help", [this](std::string_view) 
			{
				const auto logger = spdlog::get("Console");
				logger->info("Available commands:");
				for (const auto& [name, callback] : m_commandRegistry)
				{
					logger->info("  -> {}", name);
				}
			});

		registerCommand("clear", [this](std::string_view)
		    {
			    auto sink = m_logger.getConsoleSink();
			    sink->clear();
		    });
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

		const auto start = commandLine.find_first_not_of(" \t\r\n");
		if (start == std::string_view::npos) return;
		
		commandLine.remove_prefix(start);
		const auto separator = commandLine.find_first_of(" \t\r\n");

		std::string_view arguments;
		const auto argumentStart = commandLine.find_first_not_of(" \t\r\n", separator);
		if (argumentStart != std::string_view::npos)
			arguments = commandLine.substr(argumentStart);

		static Logger logger = []
		{
			Logger result;
			result.init("Console");
			return result;
		}();

		const std::string name(commandLine.substr(0, separator));
		const auto command = m_commandRegistry.find(name);
		if (command == m_commandRegistry.end())
		{
			logger.warn("Unknown command: {}", name);
			return;
		}
		try
		{
			command->second(arguments);
		}
		catch (const std::exception& error)
		{
			logger.error("Command '{}' failed: {}", name, error.what());
		}
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
