#include "pch.h"

#if defined(COMPILE_WITH_EDITOR)
#include <imgui.h>
#include "EngineStates.h"
#include "EngineStateWidget.h"
#include "Engine.h"
#include "SubSystems/Physics/PhysicsSystem.h"

namespace SE
{
	void EngineStateWidget::render()
	{
		if (!s_visible) return;

		if (!ImGui::Begin("Engine State", &s_visible))
		{
			ImGui::End();
			return;
		}

		const bool playing = EngineStates::isPlaying();

		ImGui::BeginDisabled(playing);
		if (ImGui::Button("Play"))
		{
			EngineStates::play();
			if (PhysicsSystem* physicsSystem = Engine::Instance().getSubSystem<PhysicsSystem>())
			{
				physicsSystem->resume();
			}
		}
		ImGui::EndDisabled();

		ImGui::SameLine();

		ImGui::BeginDisabled(!playing);
		if (ImGui::Button("Pause"))
		{
			EngineStates::pause();
			if (PhysicsSystem* physicsSystem = Engine::Instance().getSubSystem<PhysicsSystem>())
			{
				physicsSystem->pause();
			}
		}
		ImGui::EndDisabled();

		ImGui::SameLine();
		ImGui::Text("State: %s", playing ? "Playing" : "Paused");

		ImGui::End();
	}

	void EngineStateWidget::shutdown()
	{
	}
} // namespace SE
#endif
