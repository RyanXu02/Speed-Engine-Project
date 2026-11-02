#include "pch.h"

#if defined(COMPILE_WITH_EDITOR)
#include "../../Graphics/Rendering/RendererManager.h"
#include "../../Graphics/Rendering/SceneRenderer/SceneRenderer.h"
#include "../../Scenes/SceneSystem.h"
#include "Core/SubSystems/Physics/PhysicsSystem.h"
#include "Engine.h"
#include "EngineStates.h"
#include "MainWidget.h"
#include "MemoryWidget.h"
#include "ConsoleWidget.h"
#include "EngineStateWidget.h"
#include "PropertiesWidget.h"
#include "ResourceBrowserWidget.h"
#include "SceneInfoWidget.h"
#include "ScenePreviewWidget.h"

//TESTING AUDIO SYSTEM
#include "Core/SubSystems/Audio/AudioSystem.h"

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
				const std::string path = "Assets/Data/default-scene.json"; //TODO: hard code scene path for now

				if (ImGui::MenuItem("Save", "Ctrl+S"))
				{
					if (Scene* scene = Engine::Instance().getSubSystem<SceneSystem>()->getCurrentScene())
					{
						scene->saveScene(path);
					}
				}
				if (ImGui::MenuItem("Load", "Ctrl+O"))
				{
					if (Scene* scene = Engine::Instance().getSubSystem<SceneSystem>()->getCurrentScene())
					{
						// load scene does not defer to next frame so it will crash running systems on null references
						// reload does defer request so use it for now 
						//scene->loadScene(path);
						scene->reloadScene();
					}
				}
				if (ImGui::MenuItem("mute test"))
				{
					auto AS = Engine::Instance().getSubSystem<AudioSystem>();
					AS->toggleMute();
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Edit"))
			{
				if (ImGui::MenuItem("Undo", "Ctrl+Z"))
				{}
				if (ImGui::MenuItem("Redo", "Ctrl+Y", false, false))
				{} // Disabled item
				ImGui::Separator();
				if (ImGui::MenuItem("Cut", "Ctrl+X"))
				{}
				if (ImGui::MenuItem("Copy", "Ctrl+C"))
				{}
				if (ImGui::MenuItem("Paste", "Ctrl+V"))
				{}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Entity"))
			{
				if (ImGui::MenuItem("Create Empty"))
				{
					if (Scene* scene = Engine::Instance().getSubSystem<SceneSystem>()->getCurrentScene())
					{
						scene->addEntity("New Empty");
					}
				}
				// add prefabs later
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Camera"))
			{
				auto& controller = Engine::Instance().getCameraController();
				ImGui::SliderFloat("sensitivity", &controller.getCameraSensitivity(), 0.001f, 0.01f);
				ImGui::SliderFloat("MoveSpeed", &controller.getCameraMoveSpeed(), 1.0f, 20.0f);
				ImGui::SliderFloat("DollySpeed", &controller.getCameraDollySpeed(), 1.0f, 40.0f);
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Lighting"))
			{
				auto* SR = Engine::Instance().getSubSystem<RendererManager>()->getRenderer<SceneRenderer>();
				if (SR)
				{
					LightingSettings& settings = SR->getLightingSettings();
					ImGui::SliderFloat("Ambient", &settings.ambientStrength, 0.0f, 1.0f);
					ImGui::SliderFloat("Gamma", &settings.gamma, 1.0f, 3.0f);

					ImGui::SeparatorText("Sun");
					DirectionalLight& sun = Engine::Instance().getSubSystem<RendererManager>()->getSun();
					ImGui::Checkbox("Enabled", &sun.enabled);
					ImGui::DragFloat3("Direction", &sun.direction.x, 0.01f, -1.0f, 1.0f);
					ImGui::ColorEdit3("Color", &sun.color.x);
					ImGui::DragFloat("Intensity", &sun.intensity, 0.05f, 0.0f, 100.0f);
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Window"))
			{
				bool vis = MemoryWidget::isVisible();
				if (ImGui::MenuItem("Memory Panel", nullptr, vis))
				{
					MemoryWidget::setVisible(!vis);
				}
				const bool consoleVisible = ConsoleWidget::isVisible();
				if (ImGui::MenuItem("Console", nullptr, consoleVisible))
				{
					ConsoleWidget::setVisible(!consoleVisible);
				}
				const bool propertiesVisible = PropertiesWidget::isVisible();
				if (ImGui::MenuItem("Properties", nullptr, propertiesVisible))
				{
					PropertiesWidget::setVisible(!propertiesVisible);
				}
				const bool resourceBrowserVisible = ResourceBrowserWidget::isVisible();
				if (ImGui::MenuItem("Resource Browser", nullptr, resourceBrowserVisible))
				{
					ResourceBrowserWidget::setVisible(!resourceBrowserVisible);
				}
				const bool sceneInfoVisible = SceneInfoWidget::isVisible();
				if (ImGui::MenuItem("Scene Info", nullptr, sceneInfoVisible))
				{
					SceneInfoWidget::setVisible(!sceneInfoVisible);
				}
				const bool engineStateVisible = EngineStateWidget::isVisible();
				if (ImGui::MenuItem("Engine State", nullptr, engineStateVisible))
				{
					EngineStateWidget::setVisible(!engineStateVisible);
				}
				const bool scenePreviewVisible = ScenePreviewWidget::isVisible();
				if (ImGui::MenuItem("Scene Preview", nullptr, scenePreviewVisible))
				{
					ScenePreviewWidget::setVisible(!scenePreviewVisible);
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Physics"))
			{
				if (ImGui::MenuItem("Run Physics", nullptr))
				{
					if (PhysicsSystem* physicsSystem = Engine::Instance().getSubSystem<PhysicsSystem>())
					{
						physicsSystem->resume();
					}
				}
				if (ImGui::MenuItem("Pause Physics", nullptr))
				{
					if (PhysicsSystem* physicsSystem = Engine::Instance().getSubSystem<PhysicsSystem>())
					{
						physicsSystem->pause();
					}
				}
				ImGui::EndMenu();
			}
			const bool debugEnabled = EngineStates::isDebugRenderingEnabled();
			if (ImGui::Button(debugEnabled ? "Debug Rendering: On" : "Debug Rendering: Off"))
				EngineStates::setDebugRenderingEnabled(!debugEnabled);
		}
		ImGui::EndMainMenuBar();
	}
	void MainWidget::shutdown()
	{
		_setStatus("Shutting down MainWidget");
	}
} // namespace SE
#endif
