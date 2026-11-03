#include "pch.h"

#if defined(COMPILE_WITH_EDITOR)
#include "../../Graphics/Rendering/RendererManager.h"
#include "../../Graphics/Rendering/SceneRenderer/SceneRenderer.h"
#include "../../Graphics/Rendering/ShadowRenderer/ShadowRenderer.h"
#include "../../Graphics/Rendering/TonemapRenderer/TonemapRenderer.h"
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
				auto* sceneRenderer = Engine::Instance().getSubSystem<RendererManager>()->getRenderer<SceneRenderer>();
				if (sceneRenderer)
				{
					LightingSettings& settings = sceneRenderer->getLightingSettings();
					ImGui::SliderFloat("Ambient", &settings.ambientStrength, 0.0f, 1.0f);
				}

				auto* tonemap = Engine::Instance().getSubSystem<RendererManager>()->getRenderer<TonemapRenderer>();
				if (tonemap)
				{
					int curve = static_cast<int>(tonemap->curve);
					if (ImGui::Combo("Tonemap", &curve, "None\0Reinhard\0ACES\0")) {
						tonemap->curve = static_cast<TonemapCurve>(curve);
					}
					ImGui::SliderFloat("Exposure", &tonemap->exposure, 0.0f, 8.0f, "%.2f");
					ImGui::SliderFloat("Gamma", &tonemap->gamma, 1.0f, 3.0f);
					ImGui::Checkbox("Dither", &tonemap->dither);
				}

				if (sceneRenderer)
				{

					ImGui::SeparatorText("Sun");
					DirectionalLight& sun = Engine::Instance().getSubSystem<RendererManager>()->getSun();
					ImGui::Checkbox("Enabled", &sun.enabled);
					ImGui::ColorEdit3("Color", &sun.color.x);
					ImGui::DragFloat("Intensity", &sun.intensity, 0.05f, 0.0f, 100.0f);
				}

				auto* shadow = Engine::Instance().getSubSystem<RendererManager>()->getRenderer<ShadowRenderer>();
				if (shadow)
				{
					// the animation drives these, so only editable while it's off
					ImGui::BeginDisabled(shadow->animateSun);
					ImGui::SliderFloat("Azimuth", &shadow->sunAzimuth, -180.0f, 180.0f, "%.1f deg");
					ImGui::SliderFloat("Elevation", &shadow->sunElevation, -89.0f, 89.0f, "%.1f deg");
					ImGui::EndDisabled();

					ImGui::Checkbox("Animate Sun", &shadow->animateSun);
					ImGui::SameLine();
					ImGui::Checkbox("Use PCSS", &shadow->usePCSS);
					ImGui::SameLine();
					ImGui::Checkbox("Blue Noise", &shadow->useBlueNoise);

					//ImGui::SliderFloat("Azimuth Speed", &shadow->sunRotateSpeed, -180.0f, 180.0f, "%.0f deg/s");
					//ImGui::SliderFloat("Elevation Speed", &shadow->sunElevationSpeed, 0.0f, 180.0f, "%.0f deg/s");
					ImGui::DragFloatRange2("Elevation Range", &shadow->sunMinElevation, &shadow->sunMaxElevation, 0.5f, -89.0f, 89.0f, "Min %.0f", "Max %.0f");

					ImGui::SeparatorText("Shadows");
					ImGui::DragFloat("Coverage", &shadow->orthoHalfSize, 0.5f, 1.0f, 500.0f);
					ImGui::DragFloat("Light Distance", &shadow->lightDistance, 0.5f, 1.0f, 1000.0f);
					ImGui::SliderFloat("Depth Bias", &shadow->depthBias, 0.0f, 0.02f, "%.4f");
					ImGui::SliderFloat("Normal Offset", &shadow->normalOffset, 0.0f, 0.5f, "%.3f");
					ImGui::SliderFloat("Penumbra Scale", &shadow->penumbraScale, 0.0f, 1.0f, "%.3f");
					ImGui::SliderFloat("Search Radius", &shadow->searchRadius, 0.0f, 0.1f, "%.3f");
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
