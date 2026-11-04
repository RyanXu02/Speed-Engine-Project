#include "pch.h"

#if defined(COMPILE_WITH_EDITOR)
#include "../../Scenes/SceneSystem.h"
#include "Engine.h"
#include "SceneInfoWidget.h"
#include "MainWidget.h"
#include "imgui.h"

namespace SE
{
	using MT = ActiveSceneModified::ModifyType;

	SceneInfoWidget::SceneInfoWidget()
		: VisibleWidget("Scene Info")
	{
		m_activeSceneModified = Engine::Instance().getSubSystem<EventSystem>()->subscribe(EventType::ActiveSceneModified,
		    [this](Event& event)
		    {
			    ActiveSceneModified& asme = static_cast<ActiveSceneModified&>(event);
			    if (asme.modifytype == MT::Add)
			    {
				    m_entityNameCache[asme.id] = asme.name;
			    }
			    else if (asme.modifytype == MT::Remove)
			    {
				    m_entityNameCache.erase(asme.id);
			    }
			    else if (asme.modifytype == MT::Rename)
			    {
				    m_entityNameCache[asme.id] = asme.name;
			    }
		    });

		m_sceneChanged = Engine::Instance().getSubSystem<EventSystem>()->subscribe(EventType::SceneChanged,
		    [this](Event& event)
		    {
				m_selectedEntities.clear();
				m_renameEntityId = 0;
				m_focusRename = false;
				m_renameBuffer.clear();
				m_requestDelete = false;
			    if (Scene* scene = Engine::Instance().getSubSystem<SceneSystem>()->getCurrentScene())
				    m_entityNameCache = scene->getEntityList();
			    else
				    m_entityNameCache.clear();
		    });
	}

	void SceneInfoWidget::render()
	{
		if (!s_visible) return;

		ImGui::SetNextWindowSize(ImVec2(500, 440), ImGuiCond_FirstUseEver);
		if (!ImGui::Begin("Scene Info", &s_visible))
		{
			ImGui::End();
			return;
		}

		Scene* scene = Engine::Instance().getSubSystem<SceneSystem>()->getCurrentScene();
		if (!scene)
		{
			ImGui::End();
			return;
		}
		auto entities = scene->getEntitiesWith<NameComponent>();
		ImGui::BeginDisabled(MainWidget::isSceneEditingBlocked());

		m_requestDelete = false;
		for (auto& entity : entities)
		{
			NameComponent* nameComponent = entity.getComponent<NameComponent>();
			std::string name = nameComponent->name;

			int id = entity.id();
			ImGui::PushID(id);
			const bool isRenamingThis = m_renameEntityId > 0 && m_renameEntityId == id;
			if (isRenamingThis)
			{
				_renamingHandle(id);
			}
			else
			{
				_leftClickHandle(name, id);
				_rightClickMenu(name, id);
				_f2Handle(name, id);
			}

			ImGui::PopID();
		}

		if (m_requestDelete)
		{
			if (Scene* scene = Engine::Instance().getSubSystem<SceneSystem>()->getCurrentScene())
			{
				for (uint32_t id : m_selectedEntities)
				{
					Entity entity = scene->getEntity(id);
					auto* nameComponent = entity.getComponent<NameComponent>();
					if (!nameComponent)
						continue;
					auto name = nameComponent->name;

					if (!scene->removeEntity(id))
						continue;
					MainWidget::markSceneDirty();

					EventSystem* eventSystem = Engine::Instance().getSubSystem<EventSystem>();
					auto event = std::make_unique<ActiveSceneModified>(MT::Remove, id, name);
					eventSystem->publish(std::move(event));
				}
			}
			m_selectedEntities.clear();
		}

		ImGui::EndDisabled();
		ImGui::End();
	}

	void SceneInfoWidget::shutdown()
	{
		m_entityNameCache.clear();
	}

	void SceneInfoWidget::_rightClickMenu(const std::string& name, uint32_t id)
	{
		bool isSelected = m_selectedEntities.find(id) != m_selectedEntities.end();
		bool multiSelected = m_selectedEntities.size() > 1;
		if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
		{
			if (!isSelected)
			{
				m_selectedEntities.clear();
				m_selectedEntities.insert(id);
			}
		}

		if (ImGui::BeginPopupContextItem("EntityRightClickMenu"))
		{
			if (ImGui::MenuItem("Delete"))
			{
				m_requestDelete = true;
			}

			if (ImGui::MenuItem("Rename", "F2", false, !multiSelected))
			{
				m_renameEntityId = id;
				m_focusRename = true;
				m_renameBuffer.clear();

				m_renameBuffer = name;
				m_renameBuffer.resize(256);
			}

			// Optional future actions:
			// if (ImGui::MenuItem("Duplicate")) { ... }

			ImGui::EndPopup();
		}
	}

	void SceneInfoWidget::_leftClickHandle(const std::string& name, uint32_t id)
	{
		bool isSelected = m_selectedEntities.find(id) != m_selectedEntities.end();

		if (ImGui::Selectable(name.c_str(), isSelected, ImGuiSelectableFlags_SelectOnNav))
		{
			if (ImGui::GetIO().KeyCtrl)
			{
				// Toggle selection on Ctrl+Click
				if (isSelected)
					m_selectedEntities.erase(id);
				else
					m_selectedEntities.insert(id);
			}
			else
			{
				// Single selection on normal click
				m_selectedEntities.clear();
				m_selectedEntities.insert(id);
			}

			// Publish the EntitySelected event with the selected entity ID
			if (m_selectedEntities.size() == 1)
			{
				Engine::Instance().getSubSystem<EventSystem>()->publish(std::make_unique<EntitySelected>(*m_selectedEntities.begin()));
			}
			else
			{
				Engine::Instance().getSubSystem<EventSystem>()->publish(std::make_unique<EntitySelected>(0));
			}
		}
	}

	void SceneInfoWidget::_renamingHandle(uint32_t id)
	{
		if (m_focusRename && !MainWidget::isSceneEditingBlocked())
		{
			ImGui::SetKeyboardFocusHere();
			m_focusRename = false;
		}

		const ImGuiInputTextFlags flags = ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue;
		const bool enterPressed = ImGui::InputText("##EntityRename", m_renameBuffer.data(), m_renameBuffer.size(), flags);

		bool cancelRename = false;
		if (ImGui::IsItemActive() && ImGui::IsKeyPressed(ImGuiKey_Escape))
			cancelRename = true;
		const bool confirmRename = !cancelRename && (enterPressed || ImGui::IsItemDeactivated());

		if (cancelRename)
		{
			m_renameEntityId = 0;
			m_renameBuffer.clear();
		}
		else if (confirmRename)
		{
			const std::string newName(m_renameBuffer.c_str());

			if (Scene* scene = Engine::Instance().getSubSystem<SceneSystem>()->getCurrentScene())
			{
				if (scene->hasEntity(id))
				{
					Entity entity = scene->getEntity(id);
					NameComponent* name = entity.getComponent<NameComponent>();
					if (name && name->name != newName)
					{
						name->name = newName;
						MainWidget::markSceneDirty();
						EventSystem* eventSystem = Engine::Instance().getSubSystem<EventSystem>();
						auto event = std::make_unique<ActiveSceneModified>(MT::Rename, id, newName);
						eventSystem->publish(std::move(event));
					}
				}
			}

			m_renameEntityId = 0;
			m_renameBuffer.clear();
		}
	}

	void SceneInfoWidget::_f2Handle(const std::string& name, uint32_t id)
	{
		bool multiSelected = m_selectedEntities.size() > 1;
		bool isItemFocused = ImGui::IsItemFocused();
		bool isF2Pressed = ImGui::IsKeyPressed(ImGuiKey_F2);
		if (isItemFocused && isF2Pressed && !multiSelected)
		{
			m_renameEntityId = id;
			m_focusRename = true;
			m_renameBuffer.clear();

			m_renameBuffer = name;
			m_renameBuffer.resize(256);
		}
	}
} // namespace SE
#endif
