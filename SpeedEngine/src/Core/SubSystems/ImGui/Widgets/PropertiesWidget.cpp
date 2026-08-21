#include "pch.h"
#include "PropertiesWidget.h"
#include "Engine.h"

#include "../../Scenes/SceneSystem.h"

#include "imgui.h"

namespace SE {
	PropertiesWidget::PropertiesWidget() : Widget("PropertiesWidget") 
	{
		m_EntitySelectedSubscription = Engine::Instance().getSubSystem<EventSystem>()->subscribe(EventType::EntitySelected, [this](const Event& event) {
			const EntitySelected& entitySelectedEvent = static_cast<const EntitySelected&>(event);
			m_selectedEntityId = entitySelectedEvent.id;
			m_cachedSelectedEntity = Engine::Instance().getSubSystem<SceneSystem>()->getCurrentScene()->getEntity(m_selectedEntityId);
			});
		m_ActiveSceneModifiedSubscription = Engine::Instance().getSubSystem<EventSystem>()->subscribe(EventType::ActiveSceneModified, [this](const Event& event) {
			const ActiveSceneModified& activeSceneModifiedEvent = static_cast<const ActiveSceneModified&>(event);
			if (activeSceneModifiedEvent.modifytype == ActiveSceneModified::ModifyType::Remove && activeSceneModifiedEvent.id == m_selectedEntityId) {
				m_selectedEntityId = 0;
				m_cachedSelectedEntity = nullptr;
			}
			});
		m_SceneChangedSubscription = Engine::Instance().getSubSystem<EventSystem>()->subscribe(EventType::SceneChanged, [this](const Event& event) {
			const SceneChanged& sceneChangedEvent = static_cast<const SceneChanged&>(event);
			m_selectedEntityId = 0;
			m_cachedSelectedEntity = nullptr;
			});
	}

	void PropertiesWidget::render()
	{
		std::string entityName;
		if (m_cachedSelectedEntity && m_cachedSelectedEntity->isAlive())
			entityName = m_cachedSelectedEntity->getName();
		else
			entityName = "None Selected";

		std::string windowTitle = std::format("Properties - {}###PropertiesWidget", entityName);


		ImGui::Begin(windowTitle.c_str(), nullptr);

		if (m_cachedSelectedEntity)
		{
			for (const auto& component : m_cachedSelectedEntity->getComponents())
			{
				component->renderImGuiInterface();
			}
		}

		ImGui::End();
	}

	void PropertiesWidget::shutdown()
	{

	}
}