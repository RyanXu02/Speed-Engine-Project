#include "pch.h"

#if defined(COMPILE_WITH_EDITOR)
#include "../../Graphics/Rendering/RendererManager.h"
#include "../../Graphics/Rendering/ImageBasedRenderer/ImageBasedRenderer.h"
#include "../../Graphics/Rendering/ParticleRenderer/ParticleRenderer.h"
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
#include <imgui_stdlib.h>

namespace SE
{
	void MainWidget::beginEditorFrame()
	{
		auto* scenes = Engine::Instance().getSubSystem<SceneSystem>();
		if (m_sceneRevision != scenes->getSceneRevision())
		{
			m_sceneRevision = scenes->getSceneRevision();
			s_dirty = false;
		}
		s_editsBlocked = scenes->hasPendingTransition() || m_action != Action::None || m_dialog != Dialog::None;
	}

	void MainWidget::render()
	{
		ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);
		auto* scenes = Engine::Instance().getSubSystem<SceneSystem>();
		auto* scene = scenes->getCurrentScene();
		const bool available = !s_editsBlocked;

		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("New Scene", "Ctrl+N", false, available))
					queueAction(Action::New);
				if (ImGui::MenuItem("Open Scene...", "Ctrl+O", false, available))
					queueAction(Action::Open);
				if (ImGui::MenuItem("Save", "Ctrl+S", false, available && scene))
					queueAction(Action::Save);
				if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S", false, available && scene))
					queueAction(Action::SaveAs);
				if (ImGui::MenuItem("Reload Scene", "Ctrl+R", false, available && scene && !scene->getSourcePath().empty()))
					queueAction(Action::Reload);
				ImGui::Separator();
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
				if (ImGui::MenuItem("Create Empty", nullptr, false, !s_editsBlocked && scene))
				{
					if (Scene* scene = Engine::Instance().getSubSystem<SceneSystem>()->getCurrentScene())
					{
						scene->addEntity("New Empty");
						markSceneDirty();
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
					ImGui::SliderFloat("Ambient", &settings.ambientStrength, 0.0f, 1.0f); // flat ambient, used while IBL is off
				}

				auto* ibl = Engine::Instance().getSubSystem<RendererManager>()->getRenderer<ImageBasedRenderer>();
				if (ibl)
				{
					ImGui::Checkbox("IBL", &ibl->enabled);
					ImGui::SameLine();
					ImGui::SliderFloat("IBL Intensity", &ibl->intensity, 0.0f, 4.0f, "%.2f");

					ImGui::Checkbox("Skybox", &ibl->drawSkybox);
					ImGui::SameLine();
					int source = static_cast<int>(ibl->skyboxSource);
					if (ImGui::Combo("Sky Source", &source, "Environment\0Irradiance\0")) {
						ibl->skyboxSource = static_cast<SkyboxSource>(source);
					}
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
			if (ImGui::BeginMenu("Particles"))
			{
				auto* particles = Engine::Instance().getSubSystem<RendererManager>()->getRenderer<ParticleRenderer>();
				if (particles)
				{
					int mode = static_cast<int>(particles->blendMode);
					if (ImGui::Combo("Blend", &mode, "Alpha\0Additive\0")) {
						particles->blendMode = static_cast<ParticleBlendMode>(mode);
					}
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
			if (scene)
			{
				ImGui::SameLine();
				ImGui::Text("%s%s%s", scene->getName().c_str(), s_dirty ? " *" : "",
					scene->getSourcePath().empty() ? " (unsaved)" : "");
			}
		}
		ImGui::EndMainMenuBar();
	}

	void MainWidget::queueAction(Action action)
	{
		if (m_action != Action::None || m_dialog != Dialog::None
			|| Engine::Instance().getSubSystem<SceneSystem>()->hasPendingTransition())
			return;
		m_action = action;
		m_actionFrame = ImGui::GetFrameCount();
		m_actionRevision = m_sceneRevision;
		m_error.clear();
	}

	void MainWidget::finishAction()
	{
		m_action = Action::None;
		m_dialog = Dialog::None;
		m_openDialog = false;
		m_confirmOverwrite = false;
		m_error.clear();
	}

	void MainWidget::performAction()
	{
		auto* scenes = Engine::Instance().getSubSystem<SceneSystem>();
		switch (m_action)
		{
		case Action::New:
			scenes->requestNewScene("Untitled");
			break;
		case Action::Open:
			prepDialog(Dialog::Open);
			return;
		case Action::Reload:
			scenes->requestReloadScene();
			break;
		default:
			break;
		}
		finishAction();
	}

	bool MainWidget::saveScene(const std::string& path)
	{
		auto* scene = Engine::Instance().getSubSystem<SceneSystem>()->getCurrentScene();
		if (!scene || !scene->saveScene(path))
		{
			m_error = "Save failed. The scene was not replaced. Check the path and console, then retry or cancel.";
			return false;
		}
		s_dirty = false;
		m_error.clear();
		return true;
	}

	void MainWidget::refreshSceneFiles()
	{
		m_sceneFiles.clear();
		try
		{
			for (const auto& file : std::filesystem::directory_iterator(m_folder))
			{
				const auto extension = file.path().extension().string();
				if (file.is_regular_file() && (extension == ".json" || extension == ".JSON"))
					m_sceneFiles.push_back(file.path().filename().string());
			}
			std::ranges::sort(m_sceneFiles);
		}
		catch (const std::filesystem::filesystem_error& error)
		{
			m_error = error.what();
		}
	}

	void MainWidget::prepDialog(Dialog dialog)
	{
		m_dialog = dialog;
		m_openDialog = true;
		m_confirmOverwrite = false;
		if (dialog == Dialog::Open || dialog == Dialog::SavePath)
		{
			auto* scene = Engine::Instance().getSubSystem<SceneSystem>()->getCurrentScene();
			m_filename = dialog == Dialog::SavePath ? "untitled.json" : "";
			if (scene && !scene->getSourcePath().empty())
			{
				const std::filesystem::path source(scene->getSourcePath());
				m_folder = source.parent_path().string();
				if (dialog == Dialog::SavePath)
					m_filename = source.filename().string();
			}
			refreshSceneFiles();
		}
	}

	void MainWidget::finishEditorFrame()
	{
		auto* scenes = Engine::Instance().getSubSystem<SceneSystem>();
		if (m_action != Action::None && m_actionRevision != scenes->getSceneRevision() && m_dialog == Dialog::None)
			finishAction();
		const auto& io = ImGui::GetIO();
		if (!s_editsBlocked && io.KeyCtrl && !io.KeyAlt && !io.KeySuper && !io.WantTextInput
			&& !ImGui::IsAnyItemActive() && !ImGui::IsPopupOpen(nullptr, ImGuiPopupFlags_AnyPopupId | ImGuiPopupFlags_AnyPopupLevel))
		{
			if (!io.KeyShift && ImGui::IsKeyPressed(ImGuiKey_N, false))
				queueAction(Action::New);
			else if (!io.KeyShift && ImGui::IsKeyPressed(ImGuiKey_O, false))
				queueAction(Action::Open);
			else if (!io.KeyShift && scenes->getCurrentScene() && !scenes->getCurrentScene()->getSourcePath().empty()
				&& ImGui::IsKeyPressed(ImGuiKey_R, false))
				queueAction(Action::Reload);
			else if (scenes->getCurrentScene() && ImGui::IsKeyPressed(ImGuiKey_S, false))
				queueAction(io.KeyShift ? Action::SaveAs : Action::Save);
		}

		if (m_dialog == Dialog::None && m_action != Action::None && !scenes->hasPendingTransition()
			&& ImGui::GetFrameCount() > m_actionFrame)
		{
			auto* scene = scenes->getCurrentScene();
			if (m_action == Action::SaveAs || (m_action == Action::Save && (!scene || scene->getSourcePath().empty())))
				prepDialog(Dialog::SavePath);
			else if (m_action == Action::Save)
			{
				if (saveScene(scene->getSourcePath()))
					performAction();
				else
					prepDialog(Dialog::SavePath);
			}
			else if (s_dirty)
				prepDialog(Dialog::Confirm);
			else
				performAction();
		}
		drawActiveDialog();
		s_editsBlocked = scenes->hasPendingTransition() || m_action != Action::None || m_dialog != Dialog::None;
	}

	void MainWidget::drawActiveDialog()
	{
		if (m_dialog == Dialog::None)
			return;
		const bool confirm = m_dialog == Dialog::Confirm;
		const char* title = confirm ? "Unsaved scene changes" : "Scene file";
		if (m_openDialog)
		{
			ImGui::OpenPopup(title);
			m_openDialog = false;
		}
		ImGui::SetNextWindowSize(ImVec2(560, 0), ImGuiCond_Appearing);
		if (!ImGui::BeginPopupModal(title, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
			return;

		auto* scenes = Engine::Instance().getSubSystem<SceneSystem>();
		if (m_actionRevision != scenes->getSceneRevision())
		{
			ImGui::CloseCurrentPopup();
			finishAction();
			ImGui::EndPopup();
			return;
		}
		ImGui::BeginDisabled(scenes->hasPendingTransition());
		if (confirm)
		{
			ImGui::TextWrapped("Save your changes before continuing?");
			if (!m_error.empty())
				ImGui::TextWrapped("%s", m_error.c_str());
			if (ImGui::Button("Save"))
			{
				auto* scene = scenes->getCurrentScene();
				if (!scene || scene->getSourcePath().empty())
				{
					ImGui::CloseCurrentPopup();
					prepDialog(Dialog::SavePath);
				}
				else if (saveScene(scene->getSourcePath()))
				{
					ImGui::CloseCurrentPopup();
					m_dialog = Dialog::None;
					performAction();
				}
			}
			ImGui::SameLine();
			if (ImGui::Button("Discard"))
			{
				ImGui::CloseCurrentPopup();
				m_dialog = Dialog::None;
				performAction();
			}
		}
		else
		{
			const bool saving = m_dialog == Dialog::SavePath;
			ImGui::TextUnformatted(saving ? "Save scene as" : "Open scene");
			ImGui::BeginDisabled(m_confirmOverwrite);
			if (ImGui::InputText("Folder", &m_folder, ImGuiInputTextFlags_EnterReturnsTrue))
				refreshSceneFiles();
			ImGui::SameLine();
			if (ImGui::Button("Refresh"))
			{
				m_error.clear();
				refreshSceneFiles();
			}
			ImGui::BeginChild("SceneFiles", ImVec2(520, 160), true);
			for (const auto& file : m_sceneFiles)
				if (ImGui::Selectable(file.c_str(), m_filename == file))
					m_filename = file;
			ImGui::EndChild();
			if (ImGui::IsWindowAppearing())
				ImGui::SetKeyboardFocusHere();
			const bool enter = ImGui::InputText("Filename / path", &m_filename, ImGuiInputTextFlags_EnterReturnsTrue);
			ImGui::EndDisabled();
			if (!m_error.empty())
				ImGui::TextWrapped("%s", m_error.c_str());

			if (m_confirmOverwrite)
			{
				ImGui::TextWrapped("File already exists. Overwrite it?");
				if (ImGui::Button("Overwrite") && saveScene(m_selectedPath))
				{
					ImGui::CloseCurrentPopup();
					m_dialog = Dialog::None;
					performAction();
				}
				ImGui::SameLine();
				if (ImGui::Button("Back"))
					m_confirmOverwrite = false;
			}
			else if (ImGui::Button(saving ? "Save" : "Open") || enter)
			{
				try
				{
					if (m_filename.empty())
						m_error = "Enter a filename or choose a scene file.";
					else
					{
						std::filesystem::path filename(m_filename);
						if (saving && !filename.has_extension())
							filename += ".json";
						m_selectedPath = (std::filesystem::path(m_folder) / filename).lexically_normal().generic_string();
						if (!saving)
						{
							if (std::filesystem::is_regular_file(m_selectedPath))
							{
								scenes->requestOpenScene(m_selectedPath);
								ImGui::CloseCurrentPopup();
								finishAction();
							}
							else
								m_error = "Scene file does not exist.";
						}
						else if (std::filesystem::exists(m_selectedPath))
							m_confirmOverwrite = true;
						else if (saveScene(m_selectedPath))
						{
							ImGui::CloseCurrentPopup();
							m_dialog = Dialog::None;
							performAction();
						}
					}
				}
				catch (const std::filesystem::filesystem_error& error)
				{
					m_error = error.what();
				}
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel") || ImGui::IsKeyPressed(ImGuiKey_Escape, false))
		{
			ImGui::CloseCurrentPopup();
			finishAction();
		}
		ImGui::EndDisabled();
		ImGui::EndPopup();
	}

	void MainWidget::shutdown()
	{
		_setStatus("Shutting down MainWidget");
	}
} // namespace SE
#endif
