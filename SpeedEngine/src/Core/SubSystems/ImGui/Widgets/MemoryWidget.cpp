#include "pch.h"

#if defined(COMPILE_WITH_EDITOR)

#include "../../../Engine.h"
#include "MemoryWidget.h"

#include <imgui.h>

namespace SE
{
	void MemoryWidget::setVisible(bool v)
	{
		s_visible = v;
	}

	bool MemoryWidget::isVisible()
	{
		return s_visible;
	}

	void MemoryWidget::render()
	{
		_setStatus("Rendering MemoryWidget");
		if (!s_visible)
			return;

		// get snapshot from memory tracking system
		MemoryTrackingSystem::Snapshot snap;
		try
		{
			snap = Engine::Instance().getSubSystem<MemoryTrackingSystem>()->getSnapshot();
		}
		catch (...)
		{
			// subsystem might not exist in some builds
			return;
		}

		ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);
		if (!ImGui::Begin("Memory Panel", &s_visible))
		{
			ImGui::End();
			return;
		}

		ImGui::Text("Current: %zu bytes", snap.currentBytes);
		ImGui::Text("Peak: %zu bytes", snap.peakBytes);
		ImGui::Text("Allocations: %zu", snap.totalAllocations);
		ImGui::Text("Deallocations: %zu", snap.totalDeallocations);
		ImGui::Separator();

		for (const auto& [category, records] : snap.categories)
		{
			if (ImGui::CollapsingHeader(category.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::BeginChild((std::string("child_") + category).c_str(), ImVec2(0, 160), true);
				for (const auto& info : records)
				{
					ImGui::Text("%p | %6zu bytes | %s | num=%zu", info.address, info.size, info.item.c_str(), info.num);
				}
				ImGui::EndChild();
			}
		}

		ImGui::End();
	}

	void MemoryWidget::shutdown()
	{
		_setStatus("Shutting down MemoryWidget");
	}
}

#endif
