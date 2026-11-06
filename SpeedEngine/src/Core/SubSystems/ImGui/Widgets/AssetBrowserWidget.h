#pragma once

#include "SubSystems/ImGui/Widget.h"

#include <imgui.h>

namespace SE
{
	class AssetBrowserWidget : public VisibleWidget<AssetBrowserWidget>
	{
	public:
		AssetBrowserWidget();

		void render() override;
		void shutdown() override {}

	private:
		void changeDirectory(const std::filesystem::path& directory);

		std::filesystem::path m_directory;
		std::vector<std::filesystem::directory_entry> m_entries;

		ImGuiSelectionBasicStorage m_selection;
	};
}
