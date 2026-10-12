#include "pch.h"

#if defined(COMPILE_WITH_EDITOR)

#include "../../Scenes/SceneSystem.h"
#include "Engine.h"
#include "PropertiesWidget.h"
#include "ResourceManagers/ResourceManager.h"
#include "imgui.h"
#include "imgui_stdlib.h"

namespace SE
{
	PropertiesWidget::PropertiesWidget()
	    : Widget("PropertiesWidget")
	{
// REGISTER YOUR COMPONENT FOR THE PROPERTIES WIDGET HERE:
#pragma region RegisterComponentEditors

		// EXAMPLE:
		// Registers a component type without passing the editor function.
		// But provides an onEdited function that is called when the component is edited
		m_inspectors.registerComponent<NameComponent>("Name", false, {},
			[this](Entity& entity, NameComponent& component)
			{
				Engine::Instance().getSubSystem<EventSystem>()->publish(std::make_unique<ActiveSceneModified>(
					ActiveSceneModified::ModifyType::Rename, entity.id(), component.name));
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
						resource = Engine::Instance().getSubSystem<ResourceManager>()->addResource<ResourceType::MeshResource>(
							m_meshPathDraft, m_meshPathDraft);
					if (empty || resource != 0)
					{
						component.path = m_meshPathDraft;
						component.meshResourceID = resource;
						m_meshPathError.clear();
					}
					else
						m_meshPathError = "Mesh could not be loaded; previous mesh retained.";
				}
				if (!m_meshPathError.empty()) ImGui::TextWrapped("%s", m_meshPathError.c_str());
			});

		m_inspectors.registerComponent<PrefabComponent>("Prefab");

		m_inspectors.registerReadOnly<RelationshipComponent>("Relationship",
			[](Entity&, RelationshipComponent& component)
			{
				ImGui::Text("Parent ID: %u", component.parent);
				for (size_t i = 0; i < component.children.size(); ++i)
					ImGui::Text("Child %zu: %u", i, component.children[i]);
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
		if (!ImGui::Begin(windowTitle.c_str(), nullptr))
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
			if (!components->contains(entry.type)) continue;

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
				[&type](const auto& entry) { return entry.type == type; });
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
		if (ImGui::Button("Add Component")) ImGui::OpenPopup("AddComponent");
		if (ImGui::BeginPopup("AddComponent"))
		{
			bool available = false;
			for (const auto& entry : m_inspectors.getEntries())
			{
				if (!entry.addable || entry.has(entity)) continue;
				available = true;
				ImGui::PushID(&entry);
				if (ImGui::MenuItem(entry.name.c_str())) entry.add(entity);
				ImGui::PopID();
			}
			if (!available) ImGui::TextDisabled("No more components available");
			ImGui::EndPopup();
		}
		
		ImGui::PopID();
		ImGui::End();
	}

	void PropertiesWidget::shutdown()
	{
	}

} // namespace SE
#endif
