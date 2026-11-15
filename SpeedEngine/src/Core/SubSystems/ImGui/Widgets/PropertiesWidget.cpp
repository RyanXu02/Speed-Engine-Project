#include "pch.h"

#if defined(COMPILE_WITH_EDITOR)

#include "Core/SubSystems/Scenes/SceneSystem.h"
#include "Engine.h"
#include "PropertiesWidget.h"
#include "AssetPathPayload.h"
#include "MainWidget.h"
#include "ResourceManagers/ResourceManager.h"
#include "Core/SubSystems/Scriptings/ScriptingSystem.h"

#include <imgui.h>
#include <imgui_stdlib.h>

namespace SE
{
	bool PropertiesWidget::authoredEdit(bool changed)
	{
		if (changed)
			m_session.markDirty();
		return changed;
	}

	PropertiesWidget::PropertiesWidget(EditorSession& session)
		: VisibleWidget("PropertiesWidget"), m_session(session), m_inspectors(session)
	{
// REGISTER YOUR COMPONENT FOR THE PROPERTIES WIDGET HERE:
#pragma region RegisterComponentEditors

		// EXAMPLE:
		// Registers a component type without passing the editor function.
		// But provides an onEdited function that is called when the component is edited
		m_inspectors.registerComponent<NameComponent>("Name", "General", false, {},
			[](Entity& entity, NameComponent& component)
		    {
			    Engine::Instance().getSubSystem<EventSystem>()->publish(std::make_unique<ActiveSceneModified>(ActiveSceneModified::ModifyType::Rename, entity.id(), component.name));
		    });

		// EXAMPLE:
		// Registers a component type with an editor function for components that need special handling
		// onEdited function will not be called since the editor function is provided
		m_inspectors.registerComponent<Transform>("Transform", "General", false,
			[this](Entity&, Transform& component)
		    {
				auto rotation = glm::eulerAngles(component.rotate);
				authoredEdit(ImGui::DragFloat3("Position", &component.translate.x, 0.1f));
				if (authoredEdit(ImGui::DragFloat3("Rotation (radians)", &rotation.x, 0.01f)))
					component.rotate = glm::quat(rotation);
				authoredEdit(ImGui::DragFloat3("Scale", &component.scale.x, 0.01f));
		    });

		m_inspectors.registerComponent<MeshComponent>("Mesh", "Rendering", true,
		    [this](Entity&, MeshComponent& component)
		    {
			    if (!m_meshDraftInitialized)
			    {
				    m_meshPathDraft = component.path;
				    m_meshDraftInitialized = true;
			    }
			    const bool enter = ImGui::InputText("Path", &m_meshPathDraft, ImGuiInputTextFlags_EnterReturnsTrue);
			    const bool deactivated = ImGui::IsItemDeactivatedAfterEdit();
				const bool dropped = acceptAssetPathPayload(m_meshPathDraft);
			    // user confirmed a new path
				if ((enter || deactivated || dropped) && m_meshPathDraft != component.path)
			    {
				    uint32_t resource = 0;
				    const bool empty = m_meshPathDraft.empty() || m_meshPathDraft == "None";
				    if (!empty)
						resource = Engine::Instance().getSubSystem<ResourceManager>()->loadMesh({ .path = m_meshPathDraft, .name = m_meshPathDraft });
				    if (empty || resource != 0)
				    {
					    component.path = m_meshPathDraft;
					    component.meshResourceID = resource;
						m_session.markDirty();
					    m_meshPathError.clear();
				    }
				    else
					    m_meshPathError = "Mesh could not be loaded; previous mesh retained.";
			    }
			    if (!m_meshPathError.empty())
				    ImGui::TextWrapped("%s", m_meshPathError.c_str());
		    });

		m_inspectors.registerComponent<TerrainComponent>("Terrain", "Rendering", true,
		    [this](Entity&, TerrainComponent& component)
		    {
			    if (!m_terrainDraftInitialized)
			    {
				    m_terrainPathDraft = component.path;
				    m_terrainDraftInitialized = true;
			    }
			    const bool enter = ImGui::InputText("Path", &m_terrainPathDraft, ImGuiInputTextFlags_EnterReturnsTrue);
			    const bool deactivated = ImGui::IsItemDeactivatedAfterEdit();
			    const bool dropped = acceptAssetPathPayload(m_terrainPathDraft);
			    // user confirmed a new path
			    if ((enter || deactivated || dropped) && m_terrainPathDraft != component.path)
			    {
				    uint32_t resource = 0;
				    const bool empty = m_terrainPathDraft.empty() || m_terrainPathDraft == "None";
				    if (!empty)
					    resource = Engine::Instance().getSubSystem<ResourceManager>()->loadTerrain({ .path = m_terrainPathDraft, .name = m_terrainPathDraft });
				    if (empty || resource != 0)
				    {
					    component.path = m_terrainPathDraft;
					    component.terrainResourceID = resource;
					    m_session.markDirty();
					    m_terrainPathError.clear();
				    }
				    else
					    m_terrainPathError = "Terrain could not be loaded; previous terrain retained.";
			    }
			    if (!m_terrainPathError.empty())
				    ImGui::TextWrapped("%s", m_terrainPathError.c_str());
		    });

		m_inspectors.registerComponent<ScriptComponent>("Script", "Scripting", false,
		    [](Entity& entity, ScriptComponent& component)
		    {
			    auto scriptingSystem = Engine::Instance().getSubSystem<ScriptingSystem>();
			    ImGui::Text(component.path.c_str());

				// an instance in lua is a table which itself is a map, member in the map can be modified and inspect
			    sol::table instance = scriptingSystem->getScriptInstance(entity);
			    if (instance.valid())
			    {
				    std::vector<std::string> names;
				    for (const auto& [key, value] : instance)
				    {
					    if (key.get_type() != sol::type::string) {
							continue;
						}
					    std::string name = key.as<std::string>();
					    if (name == "entity" || name.starts_with("_")) {// the entity handle and private fields with _ are automatically skipped
							continue;
						}
					    names.push_back(std::move(name));
				    }
			
				    ImGui::SeparatorText("Script Fields");
				    ImGui::PushID(static_cast<int>(entity.id()));
				    for (const std::string& name : names) {
					    sol::object value = instance[name];
					    switch (value.get_type())
					    {
							case sol::type::number:
							{
								double number = value.as<double>();	// lua numbers are doubles
								if (ImGui::DragScalar(name.c_str(), ImGuiDataType_Double, &number, 0.05f)) {
									instance[name] = number;
								}
								break;
							}
							case sol::type::boolean:
							{
								bool flag = value.as<bool>();
								if (ImGui::Checkbox(name.c_str(), &flag)) {
									instance[name] = flag;
								}
								break;
							}
							case sol::type::userdata:
								if (value.is<glm::vec3>()) {
									glm::vec3& vec = value.as<glm::vec3&>();	// reference into the lua userdata, edits write through
									ImGui::DragFloat3(name.c_str(), &vec.x, 0.05f);
								}
								break;
							default:	// besides the above supported types, the rest are skipped for now
								break;
					    }
				    }
				    ImGui::PopID();
			    }
			    else
			    {
				    ImGui::TextDisabled("Script not loaded yet, fields show once it runs");
			    }

			    if (ImGui::Button("Test Load Single Time Script", ImVec2(-1.0f, 0.0f)))
			    {
					//TODO: hard code script path for now
				    scriptingSystem->loadScript("Assets/Scripts/sandbox.lua");
				    scriptingSystem->runScript("Assets/Scripts/sandbox.lua");
			    }
				if (ImGui::Button("Reload Script", ImVec2(-1.0f, 0.0f)))
				{
				    scriptingSystem->reloadScript(component.path);
				}
		    });
		m_inspectors.registerComponent<CameraComponent>("Camera", "Rendering", false);

		m_inspectors.registerComponent<LightComponent>("Light", "Rendering", true,
			[this](Entity&, LightComponent& component)
		    {
				authoredEdit(ImGui::ColorEdit3("Color", &component.color.x));
				authoredEdit(ImGui::DragFloat("Intensity", &component.intensity, 1.0f, 0.0f, 10000.0f));
		    });

		m_inspectors.registerComponent<RigidBody>("Rigid Body", "Physics", true,
			[this](Entity&, RigidBody& component)
		    {
			    auto motionType = component.getMotionType();
			    unsigned int typeIndex = 0;
			    if (motionType == JPH::EMotionType::Kinematic)
				    typeIndex = 1;
			    else if (motionType == JPH::EMotionType::Static)
				    typeIndex = 2;

			    const char* motionTypes[] = { "Dynamic", "Kinematic", "Static" };
			    if (ImGui::BeginCombo("Motion Type", motionTypes[typeIndex]))
				{
					for (unsigned int i = 0; i < IM_COUNTOF(motionTypes); ++i)
					{
					    const bool isSelected = typeIndex == i;
						if (authoredEdit(ImGui::Selectable(motionTypes[i], isSelected) && !isSelected))
					    {
						    if (i == 0)
							    component.setMotionType(JPH::EMotionType::Dynamic);
						    else if (i == 1)
							    component.setMotionType(JPH::EMotionType::Kinematic);
						    else
							    component.setMotionType(JPH::EMotionType::Static);
						}
					    if (isSelected)
						    ImGui::SetItemDefaultFocus();
					}
				    ImGui::EndCombo();
				}

				auto offset = component.getOffset();
				if (authoredEdit(ImGui::DragFloat3("Offset", &offset.x, 0.1f)))
				    component.setOffset(offset);

				auto isActive = component.isActive();
				if (authoredEdit(ImGui::Checkbox("Active", &isActive)))
				{
				    if (isActive)
					    component.activateBody();
				    else
					    component.deactivateBody();
				}

				auto maxLinearVelocity = component.getMaxLinearVelocity();
				if (authoredEdit(ImGui::DragFloat("Max Linear Velocity", &maxLinearVelocity, 0.1f)))
				    component.setMaxLinearVelocity(maxLinearVelocity);
			    auto maxAngularVelocity = component.getMaxAngularVelocity();
				if (authoredEdit(ImGui::DragFloat("Max Angular Velocity", &maxAngularVelocity, 0.1f)))
				    component.setMaxAngularVelocity(maxAngularVelocity);
			    auto restitution = component.getRestitution();
				if (authoredEdit(ImGui::DragFloat("Restitution", &restitution, 0.1f)))
				    component.setRestitution(restitution);
			    auto friction = component.getFriction();
				if (authoredEdit(ImGui::DragFloat("Friction", &friction, 0.1f)))
				    component.setFriction(friction);
			    auto gravityFactor = component.getGravityFactor();
				if (authoredEdit(ImGui::DragFloat("Gravity Factor", &gravityFactor, 0.1f)))
				    component.setGravityFactor(gravityFactor);
		    });

		m_inspectors.registerComponent<PhysicsShapeBox>("Box Collider", "Physics", true,
			[this](Entity&, PhysicsShapeBox& component)
		    {
			    auto halfExtents = component.getHalfExtents();
				if (authoredEdit(ImGui::DragFloat3("Half Extents", &halfExtents.x, 0.1f)))
				    component.setHalfExtents(halfExtents);
		    });

		m_inspectors.registerComponent<PhysicsShapeCapsule>("Capsule Collider", "Physics", true,
			[this](Entity&, PhysicsShapeCapsule& component)
		    {
			    auto halfHeight = component.getHalfHeight();
				if (authoredEdit(ImGui::DragFloat("Half Height", &halfHeight, 0.1f)))
				    component.setHalfHeight(halfHeight);
			    auto radius = component.getRadius();
				if (authoredEdit(ImGui::DragFloat("Radius", &radius, 0.1f)))
				    component.setRadius(radius);
		    });

		m_inspectors.registerComponent<PhysicsShapeSphere>("Sphere Collider", "Physics", true,
			[this](Entity&, PhysicsShapeSphere& component)
		    {
			    auto radius = component.getRadius();
				if (authoredEdit(ImGui::DragFloat("Radius", &radius, 0.1f)))
				    component.setRadius(radius);
		    });

		m_inspectors.registerComponent<PhysicsShapeMesh>("Mesh Collider", "Physics", true,
		    [this](Entity&, PhysicsShapeMesh& component)
		    {
			    if (!m_physicsShapeMeshDraftInitialized)
			    {
				    m_physicsShapeMeshPathDraft = component.getPath();
				    m_physicsShapeMeshDraftInitialized = true;
			    }
			    const bool enter = ImGui::InputText("Path", &m_physicsShapeMeshPathDraft, ImGuiInputTextFlags_EnterReturnsTrue);
			    const bool deactivated = ImGui::IsItemDeactivatedAfterEdit();
				const bool dropped = acceptAssetPathPayload(m_physicsShapeMeshPathDraft);
			    // user confirmed a new path
				if ((enter || deactivated || dropped) && m_physicsShapeMeshPathDraft != component.getPath())
				{
				    component.setPath(m_physicsShapeMeshPathDraft);
					m_session.markDirty();
				}
		    });

		m_inspectors.registerComponent<AudioSFXComponent>("Sound Effect", "Audio", true,
			[this](Entity& e, AudioSFXComponent& component) {
			    const std::string path = component.getFilepath();
				if (!m_soundDraftInitialized)
			    {
					m_soundPathDraft = path;
					m_soundDraftInitialized = true;
			    }
				const bool enter = ImGui::InputText("Path", &m_soundPathDraft, ImGuiInputTextFlags_EnterReturnsTrue);
			    const bool deactivated = ImGui::IsItemDeactivatedAfterEdit();
				const bool dropped = acceptAssetPathPayload(m_soundPathDraft);
			    // user confirmed a new path
				if ((enter || deactivated || dropped) && m_soundPathDraft != path)
			    {
					if (!m_soundPathDraft.empty())
				    {
						if (!component.createSound(m_soundPathDraft, component.isGameSFX()))
							m_soundPathDraft = path;
						else
							m_session.markDirty();
						m_soundPathError.clear();
				    }
				    else
						m_soundPathError = "Sound could not be loaded; previous sound source retained.";
			    }
				if (!m_soundPathError.empty())
					ImGui::TextWrapped("%s", m_soundPathError.c_str());

			    bool isGameSFX = component.isGameSFX();
				ImGui::BeginDisabled(path.empty());
			    if (ImGui::Checkbox("Is Game SFX", &isGameSFX))
			    {
					if (component.createSound(component.getFilepath(), isGameSFX))
						m_session.markDirty();
			    }
			    ImGui::EndDisabled();
				//for testing 
			    if (ImGui::Button("Play Sound", ImVec2(-1.0f, 0.0f)))
			    {
					const glm::vec3 initPos = e.getComponent<Transform>()->translate;
				    component.PlayMySound(initPos);
			    }
			    if (ImGui::Button("Stop Sound", ImVec2(-1.0f, 0.0f)))
			    {
				    component.StopMySound();
			    }
			});

		m_inspectors.registerComponent<AudioMusicComponent>("Music", "Audio", true,
			[this](Entity&, AudioMusicComponent& component)
		    {
			    const std::string path = component.getFilepath();
				if (!m_musicDraftInitialized)
			    {
					m_musicPathDraft = path;
					m_musicDraftInitialized = true;
			    }
				const bool enter = ImGui::InputText("Path", &m_musicPathDraft, ImGuiInputTextFlags_EnterReturnsTrue);
			    const bool deactivated = ImGui::IsItemDeactivatedAfterEdit();
				const bool dropped = acceptAssetPathPayload(m_musicPathDraft);
			    // user confirmed a new path
				if ((enter || deactivated || dropped) && m_musicPathDraft != path)
			    {
					if (!m_musicPathDraft.empty())
				    {
						if (!component.createTrack(m_musicPathDraft))
							m_musicPathDraft = path;
						else
							m_session.markDirty();
						m_musicPathError.clear();
				    }
				    else
						m_musicPathError = "Track could not be loaded; previous sound source retained.";
			    }
				if (!m_musicPathError.empty())
					ImGui::TextWrapped("%s", m_musicPathError.c_str());

			    //for testing
			    if (ImGui::Button("Play Track", ImVec2(-1.0f, 0.0f)))
			    {
				    component.PlayMyTrack();
			    }
			    if (ImGui::Button("Stop Track", ImVec2(-1.0f, 0.0f)))
			    {
				    component.StopMyTrack();
			    }
		    });

		m_inspectors.registerComponent<UICanvasComponent>("Canvas", "UI");
		m_inspectors.registerComponent<UIRectComponent>("Rect", "UI");
		m_inspectors.registerComponent<UIElementComponent>("Element", "UI");
		m_inspectors.registerComponent<UIImageComponent>("Image", "UI");
		m_inspectors.registerComponent<UITextComponent>("Text", "UI");
		m_inspectors.registerComponent<UISelectableComponent>("Selectable", "UI");
		m_inspectors.registerComponent<UIButtonComponent>("Button", "UI");
		m_inspectors.registerComponent<UISliderComponent>("Slider", "UI");

		m_inspectors.registerComponent<ParticleEmitter>("ParticleEmitter", "Rendering", true, 
			[this](Entity&, ParticleEmitter& component)
			{
				//color + alpha selection
				float tintcolorRGBA[4]{ 0.f };
				auto temp = component.GetTintColor();
			    tintcolorRGBA[0] = temp.x, tintcolorRGBA[1] = temp.y, tintcolorRGBA[2] = temp.z;
			    tintcolorRGBA[3] = component.GetAlpha();
				if (authoredEdit(ImGui::ColorEdit4("Tint Color + Alpha", tintcolorRGBA)))
				{
				    component.SetTintColor(glm::vec3(tintcolorRGBA[0], tintcolorRGBA[1], tintcolorRGBA[2]));
				    component.SetAlpha(tintcolorRGBA[3]);
				}
				//emitter scale
			    auto emitterScale = component.GetEmitterScale();
				if (authoredEdit(ImGui::DragFloat3("Emitter Scale", &emitterScale.x, 0.1f, 0.f, FLT_MAX, "%.3f", ImGuiSliderFlags_AlwaysClamp)))
				    component.SetEmitterScale(emitterScale);
				//particle scale min and max
			    auto particleScaleMin = component.GetScaleMin();
			    auto particleScaleMax = component.GetScaleMax();
					//must be bound >= 0
				if (authoredEdit(ImGui::DragFloat3("Particle Scale Min", &particleScaleMin.x, 0.1f, 0.f, FLT_MAX, "%.3f", ImGuiSliderFlags_AlwaysClamp)))
				    component.SetScaleMin(particleScaleMin);
				if (authoredEdit(ImGui::DragFloat3("Particle Scale Max", &particleScaleMax.x, 0.1f, 0.f, FLT_MAX, "%.3f", ImGuiSliderFlags_AlwaysClamp)))
				    component.SetScaleMax(particleScaleMax);
				//acceleration
			    auto accel = component.GetAcceleration();
				if (authoredEdit(ImGui::DragFloat3("Acceleration", &accel.x, 0.1f)))
				    component.SetAcceleration(accel);
				//direction axis
			    auto dirAxis = component.GetDirectionAxis();
				if (authoredEdit(ImGui::DragFloat3("Direction Axis", &dirAxis.x, 0.1f)))
				    component.SetDirectionAxis(dirAxis);
				//cone angle range
			    auto coneAngleRange = component.GetConeAngleRange();
					//limit to 180. cuz its half angle, 180 on both sides = 360 around axis
				if (authoredEdit(ImGui::DragFloatRange2("Cone Angle Range", &coneAngleRange.x, &coneAngleRange.y, 0.1f, 0.f, 180.f, "%.3f", "%.3f", ImGuiSliderFlags_AlwaysClamp)))
				    component.SetConeAngleRange(coneAngleRange);
				//azimuth range
			    auto azimuthRange = component.GetAzimuthRange();
					//also limit, but to 360. 360 azimuth = all around. 
				if (authoredEdit(ImGui::DragFloat2("Azimuth Range", &azimuthRange.x, 0.1f, 0.f, 360.f, "%.3f", ImGuiSliderFlags_AlwaysClamp)))
				    component.SetAzimuthRange(azimuthRange);
				//emit rate
			    auto emitRate = component.GetEmitRate();
					//greater than 0 cuz cant have negative emit rate
				if (authoredEdit(ImGui::DragFloat("Emit Rate", &emitRate, 0.1f, 0.f, FLT_MAX, "%.3f", ImGuiSliderFlags_AlwaysClamp)))
				    component.SetEmitRate(emitRate);
				//emit max (max particles to emit)
			    auto emitMax = component.GetEmitMax();
					//greater than 0
			    unsigned min = 0, max = UINT_MAX;
				if (authoredEdit(ImGui::DragScalar("Max Particles to Emit", ImGuiDataType_U32, &emitMax, 1, &min, &max, "%u")))
				    component.SetEmitMax(emitMax);
				//lifetime min and max
			    auto lifetimeMin = component.GetLifetimeMin();
					//greater than 0
				if (authoredEdit(ImGui::DragFloat("Lifetime Min", &lifetimeMin, 0.1f, 0.f, FLT_MAX, "%.3f", ImGuiSliderFlags_AlwaysClamp)))
			    {
				    component.SetLifetimeMin(lifetimeMin);
				    component.SetLifetimeMax(lifetimeMin);
			    }
			    auto lifetimeMax = component.GetLifetimeMax();
				if (authoredEdit(ImGui::DragFloat("Lifetime Max", &lifetimeMax, 0.1f, lifetimeMin, FLT_MAX, "%.3f", ImGuiSliderFlags_AlwaysClamp)))
				    component.SetLifetimeMax(lifetimeMax);
				//speed min and max
			    auto speedMin = component.GetSpeedMin();
					//greater than 0
				if (authoredEdit(ImGui::DragFloat("Speed Min", &speedMin, 0.1f, 0.f, FLT_MAX, "%.3f", ImGuiSliderFlags_AlwaysClamp)))
			    {
				    component.SetSpeedMin(speedMin);
				    component.SetSpeedMax(speedMin);
			    }
			    auto speedMax = component.GetSpeedMax();
				if (authoredEdit(ImGui::DragFloat("Speed Max", &speedMax, 0.1f, speedMin, FLT_MAX, "%.3f", ImGuiSliderFlags_AlwaysClamp)))
				    component.SetSpeedMax(speedMax);
				//rotation speed min and max
			    auto rotSpeedMin = component.GetRotationSpeedMin();
					//greater than 0
				if (authoredEdit(ImGui::DragFloat("Rotation Speed Min", &rotSpeedMin, 0.1f, 0.f, FLT_MAX, "%.3f", ImGuiSliderFlags_AlwaysClamp)))
			    {
				    component.SetRotationSpeedMin(rotSpeedMin);
				    component.SetRotationSpeedMax(rotSpeedMin);
			    }
			    auto rotSpeedMax = component.GetRotationSpeedMax();
				if (authoredEdit(ImGui::DragFloat("Rotation Speed Max", &rotSpeedMax, 0.1f, rotSpeedMin, FLT_MAX, "%.3f", ImGuiSliderFlags_AlwaysClamp)))
				    component.SetRotationSpeedMax(rotSpeedMax);
				//rotation min and max
			    auto rotMin = component.GetRotationMin();
				if (authoredEdit(ImGui::DragFloat("Rotation Min", &rotMin, 0.1f, 0.f, FLT_MAX, "%.3f", ImGuiSliderFlags_AlwaysClamp)))
			    {
				    component.SetRotationMin(rotMin);
				    component.SetRotationMax(rotMin);
			    }
			    auto rotMax = component.GetRotationMax();
				if (authoredEdit(ImGui::DragFloat("Rotation Max", &rotMax, 0.1f, rotMin, FLT_MAX, "%.3f", ImGuiSliderFlags_AlwaysClamp)))
				    component.SetRotationMax(rotMax);
				//oneshot status
			    auto oneShot = component.GetOneShot();
				if (authoredEdit(ImGui::Checkbox("Is OneShot", &oneShot)))
				    component.SetOneShot(oneShot);
			});

#pragma endregion RegisterComponentEditors


#pragma region EventSubscriptions

		m_SceneChangedSubscription = Engine::Instance().getSubSystem<EventSystem>()->subscribe(EventType::SceneChanged,
		    [this](const Event&)
		    {
				m_inspectedEntityId = 0;
				resetDrafts();
		    });

#pragma endregion EventSubscriptions
	}

	void PropertiesWidget::resetDrafts()
	{
		m_meshDraftInitialized = false;
		m_physicsShapeMeshDraftInitialized = false;
		m_soundDraftInitialized = false;
		m_musicDraftInitialized = false;
		m_meshPathDraft.clear();
		m_physicsShapeMeshPathDraft.clear();
		m_soundPathDraft.clear();
		m_musicPathDraft.clear();
		m_meshPathError.clear();
		m_soundPathError.clear();
		m_musicPathError.clear();
	}

	void PropertiesWidget::render()
	{
		Scene* scene = Engine::Instance().getSubSystem<SceneSystem>()->getCurrentScene();
		Entity entity{ 0, nullptr };
		std::string entityName = "None Selected";
		uint32_t selectedEntityId = m_session.selectedEntities().size() == 1 ? m_session.activeEntity() : 0;

		// Get the name of the selected entity
		if (selectedEntityId)
		{
			entity = scene->getEntity(selectedEntityId);
			if (const auto* name = entity.getComponent<NameComponent>())
				entityName = name->name;
		}
		if (m_inspectedEntityId != selectedEntityId)
		{
			m_inspectedEntityId = selectedEntityId;
			resetDrafts();
		}
		if (!s_visible) return;
		// Window title
		const std::string windowTitle = std::format("Properties - {}###PropertiesWidget", entityName);
		if (!ImGui::Begin(windowTitle.c_str(), &s_visible))
		{
			ImGui::End();
			return;
		}
		// No entity selected text
		if (!selectedEntityId)
		{
			ImGui::TextUnformatted("No entity selected");
			ImGui::End();
			return;
		}

		ImGui::PushID(static_cast<int>(entity.id()));
		ImGui::BeginDisabled(MainWidget::isSceneEditingBlocked());
		ImGui::Text("Entity SSID: %llu", entity.ssid());

		const auto* components = entity.getComponentRegistry();
		const ComponentInspectorRegistry::Entry* remove = nullptr;
		// Render each registered component inspector
		for (const auto& entry : m_inspectors.getEntries())
		{
			if (!components->contains(entry.type))
				continue;

			ImGui::PushID(&entry);
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(ImGui::GetStyle().ItemSpacing.x, 6.7f));
			if (ImGui::CollapsingHeader(entry.name.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
			{
				try
				{
					entry.render(entity);
				}
				catch (const nlohmann::json::exception& error)
				{
					ImGui::TextWrapped("Cannot edit: %s", error.what());
				}

				if (entry.removable)
				{
					if (ImGui::SmallButton("Remove Component"))
						remove = &entry;
				}
				ImGui::Spacing();
			}

			ImGui::PopStyleVar();
			ImGui::PopID();
		}

		// Render any components that are not registered in the inspector registry
		for (const auto& [type, component] : *components)
		{
			const bool known = std::any_of(m_inspectors.getEntries().begin(), m_inspectors.getEntries().end(),
			    [&type](const auto& entry)
			    {
				    return entry.type == type;
			    });
			if (!known)
			{
				std::string name = Serializable::demagleTypeInfo(type);
				ImGui::PushID(type.name());
				if (ImGui::CollapsingHeader(name.c_str()))
					ImGui::TextUnformatted("No inspector registered, register this in PropertiesWidget.cpp");
				ImGui::PopID();
			}
		}

		// Remove component if requested
		if (remove)
		{
			remove->remove(entity);
			if (!remove->has(entity))
				m_session.markDirty();
			if (remove->type == typeid(MeshComponent))
			{
				m_meshDraftInitialized = false;
				m_meshPathError.clear();
			}
			if (remove->type == typeid(TerrainComponent))
			{
				m_terrainDraftInitialized = false;
				m_terrainPathError.clear();
			}
			if (remove->type == typeid(PhysicsShapeMesh))
				m_physicsShapeMeshDraftInitialized = false;
			if (remove->type == typeid(AudioSFXComponent))
			{
				m_soundDraftInitialized = false;
				m_soundPathError.clear();
			}
			if (remove->type == typeid(AudioMusicComponent))
			{
				m_musicDraftInitialized = false;
				m_musicPathError.clear();
			}
		}

		// Add component popup
		if (ImGui::Button("Add Component", ImVec2(-1.0, 0.0)))
			ImGui::OpenPopup("AddComponent");
		if (ImGui::BeginPopup("AddComponent"))
		{
			std::map<std::string, std::vector<const ComponentInspectorRegistry::Entry*>> categories;
			for (const auto& entry : m_inspectors.getEntries())
			{
				if (!entry.addable || entry.has(entity))
					continue;
				categories[entry.category].push_back(&entry);
			}
			for (const auto& [category, entries] : categories)
			{
				if (!ImGui::BeginMenu(category.c_str()))
					continue;
				for (const auto* entry : entries)
				{
					ImGui::PushID(entry);
					if (ImGui::MenuItem(entry->name.c_str()))
					{
						entry->add(entity);
						if (entry->has(entity))
							m_session.markDirty();
					}
					ImGui::PopID();
				}
				ImGui::EndMenu();
			}
			if (categories.empty())
				ImGui::TextDisabled("No more components available");
			ImGui::EndPopup();
		}

		ImGui::EndDisabled();
		ImGui::PopID();
		ImGui::End();
	}

	void PropertiesWidget::shutdown() {}

} // namespace SE
#endif
