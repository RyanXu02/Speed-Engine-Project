#include "pch.h"

#if defined(COMPILE_WITH_EDITOR)

#include "../../Scenes/SceneSystem.h"
#include "Engine.h"
#include "PropertiesWidget.h"
#include "ResourceManagers/ResourceManager.h"
#include "imgui.h"
#include "imgui_stdlib.h"
#include <SubSystems/Scriptings/ScriptingSystem.h>
//#include <SubSystems/Audio/AudioSystem.h>

namespace SE
{
	PropertiesWidget::PropertiesWidget()
		: VisibleWidget("PropertiesWidget")
	{
// REGISTER YOUR COMPONENT FOR THE PROPERTIES WIDGET HERE:
#pragma region RegisterComponentEditors

		// EXAMPLE:
		// Registers a component type without passing the editor function.
		// But provides an onEdited function that is called when the component is edited
		m_inspectors.registerComponent<NameComponent>("Name", false, {},
		    [this](Entity& entity, NameComponent& component)
		    {
			    Engine::Instance().getSubSystem<EventSystem>()->publish(std::make_unique<ActiveSceneModified>(ActiveSceneModified::ModifyType::Rename, entity.id(), component.name));
		    });

		// EXAMPLE:
		// Registers a component type with an editor function for components that need special handling
		// onEdited function will not be called since the editor function is provided
		m_inspectors.registerComponent<Transform>("Transform", false,
		    [](Entity&, Transform& component)
		    {
			    auto position = component.getTranslation();
			    auto rotation = component.getRotation();
			    auto scale = component.getScale();
			    if (ImGui::DragFloat3("Position", &position.x, 0.1f))
				    component.setPosition(position);
			    if (ImGui::DragFloat3("Rotation (radians)", &rotation.x, 0.01f))
				    component.setRotation(rotation);
			    if (ImGui::DragFloat3("Scale", &scale.x, 0.01f))
				    component.setScale(scale);
		    });

		m_inspectors.registerComponent<MeshComponent>("Mesh", true,
		    [this](Entity&, MeshComponent& component)
		    {
			    if (!m_meshDraftInitialized)
			    {
				    m_meshPathDraft = component.path;
				    m_meshDraftInitialized = true;
			    }
			    const bool enter = ImGui::InputText("Path", &m_meshPathDraft, ImGuiInputTextFlags_EnterReturnsTrue);
			    const bool deactivated = ImGui::IsItemDeactivatedAfterEdit();
			    // user confirmed a new path
			    if ((enter || deactivated) && m_meshPathDraft != component.path)
			    {
				    uint32_t resource = 0;
				    const bool empty = m_meshPathDraft.empty() || m_meshPathDraft == "None";
				    if (!empty)
						resource = Engine::Instance().getSubSystem<ResourceManager>()->loadMesh({ .path = m_meshPathDraft, .name = m_meshPathDraft });
				    if (empty || resource != 0)
				    {
					    component.path = m_meshPathDraft;
					    component.meshResourceID = resource;
					    m_meshPathError.clear();
				    }
				    else
					    m_meshPathError = "Mesh could not be loaded; previous mesh retained.";
			    }
			    if (!m_meshPathError.empty())
				    ImGui::TextWrapped("%s", m_meshPathError.c_str());
		    });

		m_inspectors.registerComponent<PrefabComponent>("Prefab");

		m_inspectors.registerReadOnly<RelationshipComponent>("Relationship",
		    [](Entity&, RelationshipComponent& component)
		    {
			    ImGui::Text("Parent ID: %u", component.parent);
			    for (size_t i = 0; i < component.children.size(); ++i)
				    ImGui::Text("Child %zu: %u", i, component.children[i]);
		    });

		m_inspectors.registerComponent<ScriptComponent>("ScriptComponent", false,
		    [](Entity&, ScriptComponent& component)
		    {
			    auto scriptingSystem = Engine::Instance().getSubSystem<ScriptingSystem>();
			    ImGui::Text(component.path.c_str());
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
		m_inspectors.registerComponent<CameraComponent>("CameraComponent", false,
		    [](Entity&, CameraComponent& component)
		    {
			    
		    });

		m_inspectors.registerComponent<RigidBody>("RigidBody", true,
		    [](Entity&, RigidBody& component)
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
					    if (ImGui::Selectable(motionTypes[i], isSelected))
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
			    if (ImGui::DragFloat3("Offset", &offset.x, 0.1f))
				    component.setOffset(offset);

				auto isActive = component.isActive();
				if (ImGui::Checkbox("Active", &isActive))
				{
				    if (isActive)
					    component.activateBody();
				    else
					    component.deactivateBody();
				}

				auto maxLinearVelocity = component.getMaxLinearVelocity();
			    if (ImGui::DragFloat("Max Linear Velocity", &maxLinearVelocity, 0.1f))
				    component.setMaxLinearVelocity(maxLinearVelocity);
			    auto maxAngularVelocity = component.getMaxAngularVelocity();
			    if (ImGui::DragFloat("Max Angular Velocity", &maxAngularVelocity, 0.1f))
				    component.setMaxAngularVelocity(maxAngularVelocity);
			    auto restitution = component.getRestitution();
			    if (ImGui::DragFloat("Restitution", &restitution, 0.1f))
				    component.setRestitution(restitution);
			    auto friction = component.getFriction();
			    if (ImGui::DragFloat("Friction", &friction, 0.1f))
				    component.setFriction(friction);
			    auto gravityFactor = component.getGravityFactor();
			    if (ImGui::DragFloat("Gravity Factor", &gravityFactor, 0.1f))
				    component.setGravityFactor(gravityFactor);
		    });

		m_inspectors.registerComponent<PhysicsShapeBox>("PhysicsShapeBox", true,
		    [](Entity&, PhysicsShapeBox& component)
		    {
			    auto halfExtents = component.getHalfExtents();
			    if (ImGui::DragFloat3("Half Extents", &halfExtents.x, 0.1f))
				    component.setHalfExtents(halfExtents);
		    });

		m_inspectors.registerComponent<PhysicsShapeCapsule>("PhysicsShapeCapsule", true,
		    [](Entity&, PhysicsShapeCapsule& component)
		    {
			    auto halfHeight = component.getHalfHeight();
			    if (ImGui::DragFloat("Half Height", &halfHeight, 0.1f))
				    component.setHalfHeight(halfHeight);
			    auto radius = component.getRadius();
			    if (ImGui::DragFloat("Radius", &radius, 0.1f))
				    component.setRadius(radius);
		    });

		m_inspectors.registerComponent<PhysicsShapeSphere>("PhysicsShapeSphere", true,
		    [](Entity&, PhysicsShapeSphere& component)
		    {
			    auto radius = component.getRadius();
			    if (ImGui::DragFloat("Radius", &radius, 0.1f))
				    component.setRadius(radius);
		    });

		m_inspectors.registerComponent<PhysicsShapeMesh>("PhysicsShapeMesh", true,
		    [this](Entity&, PhysicsShapeMesh& component)
		    {
			    if (!m_physicsShapeMeshDraftInitialized)
			    {
				    m_physicsShapeMeshPathDraft = component.getPath();
				    m_physicsShapeMeshDraftInitialized = true;
			    }
			    const bool enter = ImGui::InputText("Path", &m_physicsShapeMeshPathDraft, ImGuiInputTextFlags_EnterReturnsTrue);
			    const bool deactivated = ImGui::IsItemDeactivatedAfterEdit();
			    // user confirmed a new path
			    if ((enter || deactivated) && m_physicsShapeMeshPathDraft != component.getPath())
				    component.setPath(m_physicsShapeMeshPathDraft);
		    });

		m_inspectors.registerComponent<AudioSFXComponent>("AudioSFXComponent", true, 
			[](Entity& e, AudioSFXComponent& component) {
			    const std::string path = component.getFilepath();
			    if (!component.drafInitialized)
			    {
				    component.pathDraft = path;
				    component.drafInitialized = true;
			    }
			    const bool enter = ImGui::InputText("Path", &component.pathDraft, ImGuiInputTextFlags_EnterReturnsTrue);
			    const bool deactivated = ImGui::IsItemDeactivatedAfterEdit();
			    // user confirmed a new path
			    if ((enter || deactivated) && component.pathDraft != path)
			    {
				    if (!component.pathDraft.empty())
				    {
					    if (!component.createSound(component.pathDraft, component.isGameSFX()))
							component.pathDraft = path;
					    component.pathError.clear();
				    }
				    else
					    component.pathError = "Sound could not be loaded; previous sound source retained.";
			    }
			    if (!component.pathError.empty())
				    ImGui::TextWrapped("%s", component.pathError.c_str());

			    bool isGameSFX = component.isGameSFX();
				ImGui::BeginDisabled(path.empty());
			    if (ImGui::Checkbox("Is Game SFX", &isGameSFX))
			    {
				    component.createSound(component.getFilepath(), isGameSFX);
			    }
			    ImGui::EndDisabled();
				//for testing 
			    if (ImGui::Button("Play Sound", ImVec2(-1.0f, 0.0f)))
			    {
				    const glm::vec3 initPos = e.getComponent<Transform>()->getTranslation();
				    component.PlayMySound(initPos);
			    }
			    if (ImGui::Button("Stop Sound", ImVec2(-1.0f, 0.0f)))
			    {
				    component.StopMySound();
			    }
			});

		m_inspectors.registerComponent<AudioMusicComponent>("AudioMusicComponent", true,
		    [](Entity&, AudioMusicComponent& component)
		    {
			    const std::string path = component.getFilepath();
			    if (!component.drafInitialized)
			    {
				    component.pathDraft = path;
				    component.drafInitialized = true;
			    }
			    const bool enter = ImGui::InputText("Path", &component.pathDraft, ImGuiInputTextFlags_EnterReturnsTrue);
			    const bool deactivated = ImGui::IsItemDeactivatedAfterEdit();
			    // user confirmed a new path
			    if ((enter || deactivated) && component.pathDraft != path)
			    {
				    if (!component.pathDraft.empty())
				    {
					    if (!component.createTrack(component.pathDraft))
						    component.pathDraft = path;
					    component.pathError.clear();
				    }
				    else
					    component.pathError = "Track could not be loaded; previous sound source retained.";
			    }
			    if (!component.pathError.empty())
				    ImGui::TextWrapped("%s", component.pathError.c_str());

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

#pragma endregion RegisterComponentEditors


#pragma region EventSubscriptions

		m_EntitySelectedSubscription = Engine::Instance().getSubSystem<EventSystem>()->subscribe(EventType::EntitySelected,
		    [this](const Event& event)
		    {
			    const EntitySelected& entitySelectedEvent = static_cast<const EntitySelected&>(event);
			    m_selectedEntityId = entitySelectedEvent.id;
			    m_meshDraftInitialized = false;
			    m_meshPathError.clear();
		    });
		m_ActiveSceneModifiedSubscription = Engine::Instance().getSubSystem<EventSystem>()->subscribe(EventType::ActiveSceneModified,
		    [this](const Event& event)
		    {
			    const ActiveSceneModified& activeSceneModifiedEvent = static_cast<const ActiveSceneModified&>(event);
			    if (activeSceneModifiedEvent.modifytype == ActiveSceneModified::ModifyType::Remove && activeSceneModifiedEvent.id == m_selectedEntityId)
			    {
				    m_selectedEntityId = 0;
				    m_meshDraftInitialized = false;
			    }
		    });
		m_SceneChangedSubscription = Engine::Instance().getSubSystem<EventSystem>()->subscribe(EventType::SceneChanged,
		    [this](const Event& event)
		    {
			    m_selectedEntityId = 0;
			    m_meshDraftInitialized = false;
			    m_meshPathError.clear();
		    });

#pragma endregion EventSubscriptions
	}

	void PropertiesWidget::render()
	{
		if (!s_visible) return;

		Scene* scene = Engine::Instance().getSubSystem<SceneSystem>()->getCurrentScene();
		Entity entity{ 0, nullptr };
		std::string entityName = "None Selected";

		// Clear selections that no longer reference a good entity
		if (!scene || !scene->hasEntity(m_selectedEntityId) || scene->isEntityPendingRemoval(m_selectedEntityId))
			m_selectedEntityId = 0;

		// Get the name of the selected entity if it exists and is alive
		if (m_selectedEntityId)
		{
			entity = scene->getEntity(m_selectedEntityId);
			if (const auto* life = entity.getComponent<LifeComponent>(); life && !life->isAlive)
				m_selectedEntityId = 0;
			else if (const auto* name = entity.getComponent<NameComponent>())
				entityName = name->name;
		}
		// Window title
		const std::string windowTitle = std::format("Properties - {}###PropertiesWidget", entityName);
		if (!ImGui::Begin(windowTitle.c_str(), &s_visible))
		{
			ImGui::End();
			return;
		}
		// No entity selected text
		if (!m_selectedEntityId)
		{
			m_meshDraftInitialized = false;
			ImGui::TextUnformatted("No entity selected");
			ImGui::End();
			return;
		}

		ImGui::PushID(static_cast<int>(entity.id()));
		ImGui::Text("Entity ID: %u", entity.id());

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
			if (remove->type == typeid(MeshComponent))
			{
				m_meshDraftInitialized = false;
				m_meshPathError.clear();
			}
		}

		// Add component popup
		if (ImGui::Button("Add Component", ImVec2(-1.0, 0.0)))
			ImGui::OpenPopup("AddComponent");
		if (ImGui::BeginPopup("AddComponent"))
		{
			bool available = false;
			for (const auto& entry : m_inspectors.getEntries())
			{
				if (!entry.addable || entry.has(entity))
					continue;
				available = true;
				ImGui::PushID(&entry);
				if (ImGui::MenuItem(entry.name.c_str()))
					entry.add(entity);
				ImGui::PopID();
			}
			if (!available)
				ImGui::TextDisabled("No more components available");
			ImGui::EndPopup();
		}

		ImGui::PopID();
		ImGui::End();
	}

	void PropertiesWidget::shutdown() {}

} // namespace SE
#endif
