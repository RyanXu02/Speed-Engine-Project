#include "pch.h"

#if defined(COMPILE_WITH_EDITOR)
#include "../../Graphics/Rendering/RendererManager.h"
#include "../../Graphics/Rendering/SceneRenderer/SceneRenderer.h"
#include "../../Scenes/SceneSystem.h"
#include "Core/SubSystems/Physics/PhysicsSystem.h"
#include "Engine.h"
#include "MainWidget.h"
#include "MemoryWidget.h"
#include "ConsoleWidget.h"
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
				if (ImGui::MenuItem("shaco death sound lol"))
				{
					auto AS = Engine::Instance().getSubSystem<AudioSystem>();
					//AS->PlaySound("Assets/Audio/Shaco_Original_Death_2.wav");
					AS->PlaySFX(AS->testsfxid);
				}
				if (ImGui::MenuItem("funkytown music test"))
				{
					auto AS = Engine::Instance().getSubSystem<AudioSystem>();
					AS->PlayMusic(AS->testmusicid);
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
				auto* SR = Engine::Instance().getSubSystem<RendererManager>()->getRenderer<SceneRenderer>();
				if (SR)
				{
					ImGui::SliderFloat("sensitivity", &SR->getCameraSensitivity(), 0.001f, 0.01f);
					ImGui::SliderFloat("MoveSpeed", &SR->getCameraMoveSpeed(), 1.0f, 20.0f);
					ImGui::SliderFloat("DollySpeed", &SR->getCameraDollySpeed(), 1.0f, 40.0f);
					ImGui::EndMenu();
				}
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
		}
		ImGui::EndMainMenuBar();
	}
	void MainWidget::shutdown()
	{
		_setStatus("Shutting down MainWidget");
	}
} // namespace SE
#endif
