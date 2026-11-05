#include "pch.h"

#if defined(COMPILE_WITH_EDITOR)
#include <imgui.h>
#include <imgui_internal.h>
#include "EngineStates.h"
#include "EngineStateWidget.h"
#include "Engine.h"
#include "SubSystems/Physics/PhysicsSystem.h"

namespace SE
{
	void EngineStateWidget::render()
	{
		constexpr float toolbarVerticalPadding = 2.0f;
		constexpr float controlVerticalPadding = 2.0f;
		const ImGuiStyle& style = ImGui::GetStyle();
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(style.WindowPadding.x, toolbarVerticalPadding));
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(style.FramePadding.x, controlVerticalPadding));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(style.WindowMinSize.x, 0.0f));
		const float toolbarHeight = ImGui::GetFrameHeight() + toolbarVerticalPadding * 2.0f;
		const ImGuiWindowFlags flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
			ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoSavedSettings;
		if (!ImGui::BeginViewportSideBar("Engine Controls", ImGui::GetMainViewport(), ImGuiDir_Up, toolbarHeight, flags))
		{
			ImGui::End();
			ImGui::PopStyleVar(3);
			return;
		}

		const float spacing = style.ItemSpacing.x;
		const float buttonWidth = ImGui::CalcTextSize("Pause").x + style.FramePadding.x * 2.0f + ImGui::GetFontSize();
		const float stateWidth = std::max(ImGui::CalcTextSize("State: Playing").x, ImGui::CalcTextSize("State: Paused").x);
		const float inputWidth = ImGui::GetFontSize() * 10.0f;
		const float controlsWidth = buttonWidth * 2.0f + stateWidth + inputWidth +
			ImGui::CalcTextSize("Input").x + style.ItemInnerSpacing.x + spacing * 3.0f;
		const float startX = ImGui::GetCursorPosX() + std::max(0.0f, (ImGui::GetContentRegionAvail().x - controlsWidth) * 0.5f);
		ImGui::SetCursorPosX(startX);

		ImGui::BeginDisabled(EngineStates::isPlaying());
		if (ImGui::Button("Play", ImVec2(buttonWidth, 0.0f)))
		{
			EngineStates::play();
			if (PhysicsSystem* physicsSystem = Engine::Instance().getSubSystem<PhysicsSystem>())
			{
				physicsSystem->resume();
			}
		}
		ImGui::EndDisabled();

		ImGui::SameLine();

		ImGui::BeginDisabled(!EngineStates::isPlaying());
		if (ImGui::Button("Pause", ImVec2(buttonWidth, 0.0f)))
		{
			EngineStates::pause();
			if (PhysicsSystem* physicsSystem = Engine::Instance().getSubSystem<PhysicsSystem>())
			{
				physicsSystem->pause();
			}
		}
		ImGui::EndDisabled();

		ImGui::SameLine();
		ImGui::AlignTextToFramePadding();
		ImGui::Text("State: %s", EngineStates::isPlaying() ? "Playing" : "Paused");

		ImGui::SameLine(startX + buttonWidth * 2.0f + stateWidth + spacing * 3.0f);
		ImGui::SetNextItemWidth(inputWidth);
		int inputMode = static_cast<int>(EngineStates::getInputMode());
		if (ImGui::Combo("Input", &inputMode, "Game\0Editor\0"))
			EngineStates::setInputMode(static_cast<EngineStates::InputMode>(inputMode));

		ImGui::End();
		ImGui::PopStyleVar(3);
	}
} // namespace SE
#endif
