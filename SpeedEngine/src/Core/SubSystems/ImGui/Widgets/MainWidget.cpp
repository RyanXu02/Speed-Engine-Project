#include "pch.h"
#include "MainWidget.h"
#include "Engine.h"

#include "../../Scenes/SceneSystem.h"
#include "../../Graphics/Rendering/RendererManager.h"
#include "../../Graphics/Rendering/SceneRenderer/SceneRenderer.h"

#include <imgui.h>

namespace SE
{
	void MainWidget::render()
	{
		ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);

		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Edit"))
			{
				if (ImGui::MenuItem("Undo", "Ctrl+Z")) {}
				if (ImGui::MenuItem("Redo", "Ctrl+Y", false, false)) {} // Disabled item
				ImGui::Separator();
				if (ImGui::MenuItem("Cut", "Ctrl+X")) {}
				if (ImGui::MenuItem("Copy", "Ctrl+C")) {}
				if (ImGui::MenuItem("Paste", "Ctrl+V")) {}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Entity"))
			{
				if (ImGui::MenuItem("Create Empty")) 
				{
					if (Scene * scene = Engine::Instance().getSubSystem<SceneSystem>()->getCurrentScene())
					{
						scene->addEntity(std::make_unique<Entity>("New Empty"));
					}
				}
				// add prefabs later
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Camera"))
			{
				auto* SR = Engine::Instance().getSubSystem<RendererManager>()->getRenderer<SceneRenderer>();
				ImGui::SliderFloat("sensitivity", &SR->getCameraSensitivity(), 0.001f, 0.01f);
				ImGui::SliderFloat("MoveSpeed", &SR->getCameraMoveSpeed(), 1.0f, 20.0f);
				ImGui::SliderFloat("DollySpeed", &SR->getCameraDollySpeed(), 1.0f, 40.0f);
				ImGui::EndMenu();
			}
		}
		ImGui::EndMainMenuBar();
	}
	void MainWidget::shutdown()
	{
		_setStatus("Shutting down MainWidget");
	}
}
