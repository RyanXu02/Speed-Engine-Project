#pragma once

#include <imgui.h>

namespace SE
{
	inline constexpr char AssetPathPayloadType[] = "ASSET_PATH";

	inline bool acceptAssetPathPayload(std::string& value)
	{
		bool dropped = false;
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(AssetPathPayloadType))
			{
				value = static_cast<const char*>(payload->Data);
				dropped = true;
			}
			ImGui::EndDragDropTarget();
		}
		return dropped;
	}
}
