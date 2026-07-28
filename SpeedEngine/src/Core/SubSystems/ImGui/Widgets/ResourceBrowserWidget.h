#pragma once
#include "../Widget.h"
#include "../../Events/EventSubscription.h"
#include <imgui.h>

namespace SE 
{
	class ResourceBrowserWidget : public Widget
	{
	public:
		ResourceBrowserWidget();

		void render() override;
		void shutdown() override;

	private:
		std::unordered_map<uint32_t, ResourceType> m_resourceCache;
		EventSubscription m_resourceChanged;

		// Display options, CURRENTLY NOT BEING CHANGED
		struct BrowserOptions
		{
			float iconSize = 100.0f;
			int iconSpacing = 10;
			bool showTypeOverlay = true;
			bool stretchSpacing = true;
		} m_options;

		// Layout calculations
		struct LayoutData
		{
			ImVec2 itemSize;
			ImVec2 itemStep;
			float itemSpacing = 0.0f;
			int columnCount = 0;
			int lineCount = 0;
		} m_layout;

		// State
		std::unordered_set<uint32_t> m_selectedResources;
		bool m_requestDelete = false;
		uint32_t m_contextMenuItemId = 0; //currently not being used in code

		// Helper functions
		void UpdateLayoutSizes(float availWidth);
		void DrawResourceGrid();
		void DrawResourceItem(uint32_t id, ResourceType type);
		const char* GetResourceTypeString(ResourceType type) const;
		ImVec4 GetResourceTypeColor(ResourceType type) const;
		void DeleteSelectedResources();
	};

}

