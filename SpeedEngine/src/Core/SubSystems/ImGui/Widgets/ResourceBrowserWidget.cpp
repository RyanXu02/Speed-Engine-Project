#include "pch.h"
#include "ResourceBrowserWidget.h"

#include "../../../ResourceManagers/ResourceManager.h"
#include "../../Events/Event.h"


namespace SE 
{
	ResourceBrowserWidget::ResourceBrowserWidget() : Widget("ResourceBrowserWidget") {
		m_resourceCache = ResourceManager::Instance().getInitialRTMap();

		m_resourceChanged = EventSystem::Instance().subscribe(EventType::ResourceChanged, [this](Event& event) 
			{
				ResourceChanged& rce = static_cast<ResourceChanged&>(event);
				if (rce.adding) m_resourceCache[rce.id] = rce.type; //adding resource
				else {
					m_resourceCache.erase(rce.id); //deleting resource
					m_selectedResources.erase(rce.id); // Remove from selection if deleted
				}
			});
	}

	void ResourceBrowserWidget::render() {
		ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);

		if (!ImGui::Begin("Resource Browser"))
		{
			ImGui::End();
			return;
		}

		if (ImGui::IsKeyPressed(ImGuiKey_Delete) && !m_selectedResources.empty())
		{
			m_requestDelete = true;
		}

		// Calculate available space for grid
		float availWidth = ImGui::GetContentRegionAvail().x;
		UpdateLayoutSizes(availWidth);

		// Handle deletion requests
		if (m_requestDelete)
		{
			DeleteSelectedResources();
			m_requestDelete = false;
		}

		DrawResourceGrid();

		ImGui::End();
	}

	void ResourceBrowserWidget::UpdateLayoutSizes(float avail_width)
	{
		m_layout.itemSpacing = static_cast<float>(m_options.iconSpacing);

		if (!m_options.stretchSpacing)
			avail_width += floorf(m_layout.itemSpacing * 0.5f);

		m_layout.itemSize = ImVec2(floorf(m_options.iconSize), floorf(m_options.iconSize));
		m_layout.columnCount = std::max((int)(avail_width / (m_layout.itemSize.x + m_layout.itemSpacing)), 1);

		if (m_options.stretchSpacing && m_layout.columnCount > 1)
			m_layout.itemSpacing = floorf(avail_width - m_layout.itemSize.x * m_layout.columnCount) / m_layout.columnCount;

		m_layout.lineCount = (static_cast<int>(m_resourceCache.size()) + m_layout.columnCount - 1) / m_layout.columnCount;
		m_layout.itemStep = ImVec2(m_layout.itemSize.x + m_layout.itemSpacing, m_layout.itemSize.y + m_layout.itemSpacing);
	}

	void ResourceBrowserWidget::DrawResourceGrid()
	{
		ImGui::BeginChild("ResourceGrid", ImVec2(0, 0), 0, ImGuiChildFlags_AlwaysUseWindowPadding);
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(m_options.iconSpacing, m_options.iconSpacing));

		int col = 0;
		for (const auto& [id, type] : m_resourceCache)
		{
			if (id < 2) continue; // skip default shader

			DrawResourceItem(id, type);

			col++;
			if (col < m_layout.columnCount)
				ImGui::SameLine();
			else
				col = 0;
		}

		ImGui::PopStyleVar(2);
		ImGui::EndChild();
	}

	void ResourceBrowserWidget::DrawResourceItem(uint32_t id, ResourceType type)
	{
		bool isSelected = m_selectedResources.find(id) != m_selectedResources.end();
		ImVec4 bgColor = isSelected ? ImVec4(0.26f, 0.59f, 0.98f, 0.6f) : ImVec4(0.2f, 0.2f, 0.2f, 0.8f);

		ImGui::PushID(id);
		ImGui::PushStyleColor(ImGuiCol_Button, bgColor);
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.26f, 0.59f, 0.98f, 0.4f));

		std::string name = ResourceManager::Instance().getResource(id)->getResourceName();

		// Draw button square with ID as placeholder content
		char label[32];
		snprintf(label, sizeof(label), "%s##%u", name.c_str(), id);

		if (ImGui::Button(label, m_layout.itemSize))
		{
			if (ImGui::GetIO().KeyCtrl)
			{
				// Toggle selection on Ctrl+Click
				if (isSelected)
					m_selectedResources.erase(id);
				else
					m_selectedResources.insert(id);
			}
			else
			{
				// Single selection on normal click
				m_selectedResources.clear();
				m_selectedResources.insert(id);
			}
		}

		// Handle right-click context menu
		if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
		{
			// Select the right-clicked item if not already part of selection
			if (!isSelected)
			{
				m_selectedResources.clear();
				m_selectedResources.insert(id);
			}
			m_contextMenuItemId = id;
		}

		// Draw context menu
		if (ImGui::BeginPopupContextItem(nullptr, ImGuiPopupFlags_MouseButtonRight))
		{
			if (ImGui::MenuItem("Open (nonfunctional)"))
			{
				// TODO: Implement open resource action
			}
			ImGui::Separator();
			if (ImGui::MenuItem("Delete"))
			{
				m_requestDelete = true;
			}
			ImGui::Separator();
			if (ImGui::MenuItem("Properties (nonfunctional)"))
			{
				// TODO: Open properties dialog
			}
			ImGui::EndPopup();
		}

		// Show type overlay
		if (m_options.showTypeOverlay)
		{
			ImDrawList* drawList = ImGui::GetWindowDrawList();
			ImVec2 itemMin = ImGui::GetItemRectMin();
			ImVec2 itemMax = ImGui::GetItemRectMax();

			const char* typeStr = GetResourceTypeString(type);
			ImVec4 typeColor = GetResourceTypeColor(type);
			ImU32 bgImU32 = ImGui::GetColorU32(typeColor);

			drawList->AddRectFilled(itemMin, ImVec2(itemMax.x, itemMin.y + 20), bgImU32);
			drawList->AddText(itemMin, ImGui::GetColorU32(ImGuiCol_Text), typeStr);
		}

		ImGui::PopStyleColor(2);
		ImGui::PopID();
	}

	const char* ResourceBrowserWidget::GetResourceTypeString(ResourceType type) const
	{
		switch (type)
		{
		case ResourceType::Shader: return "Shader";
		case ResourceType::Material: return "Material";
		case ResourceType::MeshResource: return "Mesh";
		default: return "Unknown";
		}
	}

	ImVec4 ResourceBrowserWidget::GetResourceTypeColor(ResourceType type) const
	{
		switch (type)
		{
		case ResourceType::Shader: return ImVec4(0.2f, 0.8f, 0.2f, 0.8f); // Green
		case ResourceType::Material: return ImVec4(0.8f, 0.2f, 0.8f, 0.8f); // Magenta
		case ResourceType::MeshResource: return ImVec4(0.1f, 0.2f, 0.8f, 0.8f); // Blue
		default: return ImVec4(0.5f, 0.5f, 0.5f, 0.8f); // Gray
		}
	}

	void ResourceBrowserWidget::DeleteSelectedResources()
	{
		for (uint32_t id : m_selectedResources)
		{
			ResourceManager::Instance().removeResource(id);
		}
		m_selectedResources.clear();
	}

	void ResourceBrowserWidget::shutdown() {
		m_selectedResources.clear();
		m_resourceCache.clear();
	}
}