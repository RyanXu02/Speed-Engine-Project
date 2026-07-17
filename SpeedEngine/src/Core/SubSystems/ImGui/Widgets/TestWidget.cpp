#include "pch.h"
#include "TestWidget.h"

#include <imgui.h>

namespace SE
{
	void TestWidget::render()
	{
		_setMessage("Rendering TestWidget");

        ImGuiWindowFlags window_flags = 0; // Default flags allow docking
        ImGui::Begin("TestWidget", nullptr, window_flags);

        if (ImGui::BeginTable("table1", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
        {
            // Submit columns name with TableSetupColumn() and call TableHeadersRow() to create a row with a header in each column.
            // (Later we will show how TableSetupColumn() has other uses, optional flags, sizing weight etc.)
            ImGui::TableSetupColumn("One");
            ImGui::TableSetupColumn("Two");
            ImGui::TableSetupColumn("Three");
            ImGui::TableHeadersRow();
            for (int row = 0; row < 6; row++)
            {
                ImGui::TableNextRow();
                for (int column = 0; column < 3; column++)
                {
                    ImGui::TableSetColumnIndex(column);
                    ImGui::Text("Hello %d,%d", column, row);
                }
            }
            ImGui::EndTable();
        }

        ImGui::End();
	}
	
	void TestWidget::shutdown()
	{
		_setMessage("Shutting down TestWidget");
	}
}