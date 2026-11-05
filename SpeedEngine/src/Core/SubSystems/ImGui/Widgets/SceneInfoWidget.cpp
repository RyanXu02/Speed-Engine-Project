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

	SceneInfoWidget::SceneInfoWidget(EditorSession& session)
		: VisibleWidget("Scene Info")
		, m_session(session)
	{
		m_sceneChanged = Engine::Instance().getSubSystem<EventSystem>()->subscribe(EventType::SceneChanged,
		    [this](Event& event)
		    {
				m_renameEntityId = 0;
				m_focusRename = false;
				m_renameBuffer.clear();
				m_requestDelete = false;
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
				for (uint32_t id : m_session.selectedEntities())
				{
					Entity entity = scene->getEntity(id);
					auto* nameComponent = entity.getComponent<NameComponent>();
					if (!nameComponent)
						continue;
					auto name = nameComponent->name;

					if (!scene->removeEntity(id))
						continue;
					m_session.markDirty();

					EventSystem* eventSystem = Engine::Instance().getSubSystem<EventSystem>();
					auto event = std::make_unique<ActiveSceneModified>(MT::Remove, id, name);
					eventSystem->publish(std::move(event));
				}
			}
			m_session.clearSelection();
		}

		ImGui::EndDisabled();
		ImGui::End();
	}

	void SceneInfoWidget::shutdown()
	{
	}

	void SceneInfoWidget::_rightClickMenu(const std::string& name, uint32_t id)
	{
		bool isSelected = m_session.selectedEntities().contains(id);
		if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
		{
			if (!isSelected)
				m_session.select(id);
		}
		bool multiSelected = m_session.selectedEntities().size() > 1;

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
		bool isSelected = m_session.selectedEntities().contains(id);

		if (ImGui::Selectable(name.c_str(), isSelected, ImGuiSelectableFlags_SelectOnNav))
		{
			if (ImGui::GetIO().KeyCtrl)
			{
				// Toggle selection on Ctrl+Click
				m_session.toggle(id);
			}
			else
			{
				// Single selection on normal click
				m_session.select(id);
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
						m_session.markDirty();
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
		bool multiSelected = m_session.selectedEntities().size() > 1;
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
