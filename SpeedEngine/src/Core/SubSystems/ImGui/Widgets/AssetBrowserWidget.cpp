#include "pch.h"

#if defined(COMPILE_WITH_EDITOR)
#include "AssetBrowserWidget.h"
#include "AssetPathPayload.h"

#include <shellapi.h>
#pragma comment(lib, "Shell32.lib")

namespace SE
{
	AssetBrowserWidget::AssetBrowserWidget() : VisibleWidget("AssetBrowserWidget")
	{
		changeDirectory("Assets");
	}

	void AssetBrowserWidget::changeDirectory(const std::filesystem::path& directory)
	{
		m_directory = directory;
		m_selection.Clear();
		m_entries.clear();
		for (const auto& entry : std::filesystem::directory_iterator(m_directory))
			m_entries.push_back(entry);
		std::ranges::sort(m_entries, {}, [](const auto& entry) { return entry.path().filename(); });
	}

	void AssetBrowserWidget::render()
	{
		if (!s_visible) return;

		if (!ImGui::Begin("Asset Browser", &s_visible))
		{
			ImGui::End();
			return;
		}

		ImGui::BeginDisabled(m_directory == "Assets");
		if (ImGui::Button("Up"))
			changeDirectory(m_directory.parent_path());
		ImGui::EndDisabled();
		ImGui::SameLine();
		ImGui::TextUnformatted(m_directory.generic_string().c_str());

		std::filesystem::path nextDirectory;
		ImGui::PushID(m_directory.generic_string().c_str());
		if (ImGui::BeginChild("Files"))
		{
			const int count = static_cast<int>(m_entries.size());
			ImGuiMultiSelectIO* selection = ImGui::BeginMultiSelect(ImGuiMultiSelectFlags_ClearOnEscape, m_selection.Size, count);
			m_selection.ApplyRequests(selection);
			for (int i = 0; i < count; ++i)
			{
				const auto& entry = m_entries[i];
				const bool folder = entry.is_directory();
				const std::string label = entry.path().filename().string() + (folder ? "/" : "");

				ImGui::PushID(i);
				const float size = ImGui::GetTextLineHeight();
				const ImVec2 icon = ImGui::GetCursorScreenPos();
				ImDrawList* draw = ImGui::GetWindowDrawList();
				if (folder)
				{
					const ImU32 color = IM_COL32(220, 180, 80, 255);
					draw->AddRectFilled(ImVec2(icon.x, icon.y + size * 0.15f), ImVec2(icon.x + size * 0.45f, icon.y + size * 0.4f), color);
					draw->AddRectFilled(ImVec2(icon.x, icon.y + size * 0.3f), ImVec2(icon.x + size, icon.y + size * 0.85f), color, size * 0.08f);
				}
				else
				{
					const ImU32 color = ImGui::GetColorU32(ImGuiCol_Text);
					draw->AddRect(ImVec2(icon.x + size * 0.15f, icon.y + size * 0.1f), ImVec2(icon.x + size * 0.85f, icon.y + size * 0.9f), color);
					draw->AddLine(ImVec2(icon.x + size * 0.3f, icon.y + size * 0.4f), ImVec2(icon.x + size * 0.7f, icon.y + size * 0.4f), color);
					draw->AddLine(ImVec2(icon.x + size * 0.3f, icon.y + size * 0.6f), ImVec2(icon.x + size * 0.7f, icon.y + size * 0.6f), color);
				}
				ImGui::Dummy(ImVec2(size, size));
				ImGui::SameLine();
				ImGui::SetNextItemSelectionUserData(i);
				ImGui::Selectable(label.c_str(), m_selection.Contains(static_cast<ImGuiID>(i)), ImGuiSelectableFlags_AllowDoubleClick);
				if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
				{
					if (folder)
						nextDirectory = entry.path();
					else
					{
						const auto extension = entry.path().extension();
						if (extension == ".lua" || extension == ".json" || extension == ".vert" || extension == ".frag" || extension == ".comp")
							ShellExecuteW(nullptr, L"open", std::filesystem::absolute(entry.path()).c_str(), nullptr, nullptr, SW_SHOWNORMAL);
					}
				}

				if (!folder && ImGui::BeginDragDropSource())
				{
					const std::string path = entry.path().generic_string();
					ImGui::SetDragDropPayload(AssetPathPayloadType, path.c_str(), path.size() + 1);
					ImGui::TextUnformatted(path.c_str());
					ImGui::EndDragDropSource();
				}

				ImGui::PopID();
			}

			m_selection.ApplyRequests(ImGui::EndMultiSelect());
		}

		ImGui::EndChild();
		ImGui::PopID();
		if (!nextDirectory.empty())
			changeDirectory(nextDirectory);

		ImGui::End();
	}
}
#endif
