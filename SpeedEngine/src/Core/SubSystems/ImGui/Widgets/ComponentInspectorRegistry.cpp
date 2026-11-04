#include "pch.h"
#if defined(COMPILE_WITH_EDITOR)

#include "ComponentInspectorRegistry.h"

#include "imgui.h"
#include "imgui_stdlib.h"

namespace SE
{
	bool ComponentInspectorRegistry::_editComponentJson(const char* label, nlohmann::json& value)
	{
		bool changed = false;
		ImGui::PushID(label);

		if (value.is_boolean())
		{
			bool field = value.get<bool>();
			if (ImGui::Checkbox(label, &field))
			{
				value = field;
				changed = true;
			}
		}
		else if (value.is_number_unsigned())
		{
			uint64_t field = value.get<uint64_t>();
			if (ImGui::InputScalar(label, ImGuiDataType_U64, &field))
			{
				value = field;
				changed = true;
			}
		}
		else if (value.is_number_integer())
		{
			int64_t field = value.get<int64_t>();
			if (ImGui::InputScalar(label, ImGuiDataType_S64, &field))
			{
				value = field;
				changed = true;
			}
		}
		else if (value.is_number_float())
		{
			double field = value.get<double>();
			if (ImGui::InputScalar(label, ImGuiDataType_Double, &field, nullptr, nullptr, "%.6g"))
			{
				value = field;
				changed = true;
			}
		}
		else if (value.is_string())
		{
			std::string field = value.get<std::string>();
			if (ImGui::InputText(label, &field))
			{
				value = field;
				changed = true;
			}
		}
		else if (value.is_array() && value.size() >= 2 && value.size() <= 4 &&
		         // also gotta check if all elements are numbers too:
		         std::ranges::all_of(value,
		             [](const auto& item)
		             {
			             return item.is_number();
		             }))
		{
			// Keep each element's signed/unsigned/floating JSON type
			ImGui::TextUnformatted(label);
			for (size_t i = 0; i < value.size(); ++i)
			{
				if (i != 0)
					ImGui::SameLine();
				std::string index = std::format("##{}", i);
				changed |= _editComponentJson(index.c_str(), value[i]);
			}
		}
		else if (value.is_object())
		{
			const bool root = std::strcmp(label, "Properties") == 0;
			if (root || ImGui::TreeNodeEx(label, ImGuiTreeNodeFlags_DefaultOpen))
			{
				for (auto& item : value.items())
					changed |= _editComponentJson(item.key().c_str(), item.value());
				if (!root)
					ImGui::TreePop();
			}
		}
		else
		{
			ImGui::Text("%s: %s (read-only)", label, value.dump().c_str());
		}

		ImGui::PopID();
		return changed;
	}
}
#endif
