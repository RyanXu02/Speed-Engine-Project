#include "pch.h"

#if defined(COMPILE_WITH_EDITOR)
#include "../../Scenes/SceneSystem.h"
#include "Engine.h"
#include "SceneInfoWidget.h"
#include "MainWidget.h"
#include <imgui.h>
#include <imgui_stdlib.h>

namespace SE
{
	using MT = ActiveSceneModified::ModifyType;

	SceneInfoWidget::SceneInfoWidget(EditorSession& session)
		: VisibleWidget("Scene Info")
		, m_session(session)
	{
		m_sceneChanged = Engine::Instance().getSubSystem<EventSystem>()->subscribe(EventType::SceneChanged,
		    [this](Event&)
		    {
				m_renameEntityId = 0;
				m_renameTargets.clear();
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
		std::ranges::sort(entities, {}, [](const Entity& entity) { return entity.id(); });
		ImGui::BeginDisabled(MainWidget::isSceneEditingBlocked());

		struct SelectionContext
		{
			EditorSession& session;
			std::vector<Entity>& entities;
		} context{ m_session, entities };
		ImGuiSelectionExternalStorage selection;
		selection.UserData = &context;
		selection.AdapterSetItemSelected = [](ImGuiSelectionExternalStorage* storage, int index, bool selected)
		{
			auto& context = *static_cast<SelectionContext*>(storage->UserData);
			const uint32_t id = context.entities[index].id();
			if (context.session.selectedEntities().contains(id) != selected)
				context.session.toggle(id);
		};

		m_requestDelete = false;
		uint32_t renameEntityId = 0;
		std::string renameName;
		ImGui::PushID(static_cast<int>(m_session.sceneRevision()));
		ImGui::PushID(static_cast<int>(entities.size()));
		if (ImGui::BeginChild("Entities"))
		{
			const ImGuiMultiSelectFlags selectionFlags = m_renameEntityId
				? ImGuiMultiSelectFlags_NoAutoClear | ImGuiMultiSelectFlags_NoAutoSelect | ImGuiMultiSelectFlags_NoSelectAll
				: ImGuiMultiSelectFlags_ClearOnEscape | ImGuiMultiSelectFlags_ClearOnClickVoid;
			ImGuiMultiSelectIO* selectionIO = ImGui::BeginMultiSelect(selectionFlags,
				static_cast<int>(m_session.selectedEntities().size()), static_cast<int>(entities.size()));
			selection.ApplyRequests(selectionIO);
			for (int index = 0; index < static_cast<int>(entities.size()); ++index)
			{
				auto& entity = entities[index];
				const auto& name = entity.getComponent<NameComponent>()->name;
				const uint32_t id = entity.id();
				ImGui::PushID(static_cast<int>(id));
				if (m_renameEntityId == id)
					_renamingHandle();
				else
				{
					ImGui::SetNextItemSelectionUserData(index);
					ImGui::Selectable((name + "###Entity").c_str(), m_session.selectedEntities().contains(id));
					const bool f2 = ImGui::IsItemFocused() && ImGui::IsKeyPressed(ImGuiKey_F2);
					if (_rightClickMenu() || f2)
					{
						renameEntityId = id;
						renameName = name;
					}
				}
				ImGui::PopID();
			}
			if (m_requestDelete)
				selectionIO->RangeSrcReset = true;
			selection.ApplyRequests(ImGui::EndMultiSelect());
		}
		ImGui::EndChild();
		ImGui::PopID();
		ImGui::PopID();
		if (renameEntityId)
			_beginRename(renameName, renameEntityId);

		if (m_requestDelete)
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
			m_session.clearSelection();
			m_renameEntityId = 0;
			m_renameTargets.clear();
			m_renameBuffer.clear();
			m_focusRename = false;
		}

		ImGui::EndDisabled();
		ImGui::End();
	}

	void SceneInfoWidget::shutdown()
	{
	}

	bool SceneInfoWidget::_rightClickMenu()
	{
		bool rename = false;
		if (ImGui::BeginPopupContextItem("EntityRightClickMenu"))
		{
			if (ImGui::MenuItem("Delete"))
				m_requestDelete = true;
			rename = ImGui::MenuItem("Rename", "F2");
			ImGui::EndPopup();
		}
		return rename;
	}

	void SceneInfoWidget::_beginRename(const std::string& name, uint32_t id)
	{
		m_renameEntityId = id;
		m_renameTargets.assign(m_session.selectedEntities().begin(), m_session.selectedEntities().end());
		m_focusRename = true;
		m_renameBuffer = name;
	}

	void SceneInfoWidget::_renamingHandle()
	{
		if (m_focusRename && !MainWidget::isSceneEditingBlocked())
		{
			ImGui::SetKeyboardFocusHere();
			m_focusRename = false;
		}

		const ImGuiInputTextFlags flags = ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue;
		ImGui::SetNextItemWidth(-1.0f);
		const bool enterPressed = ImGui::InputText("##EntityRename", &m_renameBuffer, flags);

		const bool cancelRename = ImGui::IsKeyPressed(ImGuiKey_Escape);
		const bool confirmRename = !cancelRename && (enterPressed || ImGui::IsItemDeactivated());

		if (cancelRename)
		{
			m_renameEntityId = 0;
			m_renameTargets.clear();
			m_renameBuffer.clear();
		}
		else if (confirmRename)
		{
			const std::string& newName = m_renameBuffer;

			if (Scene* scene = Engine::Instance().getSubSystem<SceneSystem>()->getCurrentScene())
			{
				for (uint32_t id : m_renameTargets)
				{
					if (!scene->hasEntity(id))
						continue;
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
			m_renameTargets.clear();
			m_renameBuffer.clear();
		}
	}
} // namespace SE
#endif
