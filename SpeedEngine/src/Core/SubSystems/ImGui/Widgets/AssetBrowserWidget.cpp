#include "pch.h"

#if defined(COMPILE_WITH_EDITOR)
#include "AssetBrowserWidget.h"
#include "AssetPathPayload.h"

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
				ImGui::SetNextItemSelectionUserData(i);
				ImGui::Selectable(label.c_str(), m_selection.Contains(static_cast<ImGuiID>(i)), ImGuiSelectableFlags_AllowDoubleClick);
				if (folder && ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
					nextDirectory = entry.path();

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
