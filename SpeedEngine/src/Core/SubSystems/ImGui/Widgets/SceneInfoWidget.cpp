#include "pch.h"
#include "SceneInfoWidget.h"

#include "../../Scenes/SceneSystem.h"

#include "imgui/imgui.h"

namespace SE
{
    using MT = ActiveSceneModified::ModifyType;


    SceneInfoWidget::SceneInfoWidget() : Widget("Scene Info")
    {
        m_activeSceneModified = EventSystem::Instance().subscribe(EventType::ActiveSceneModified, [this](Event& event)
            {
                ActiveSceneModified& asme = static_cast<ActiveSceneModified&>(event);
                if (asme.modifytype == MT::Add) {
                    m_entityNameCache[asme.id] = asme.name;
                }
                else if (asme.modifytype == MT::Remove) {
                    m_entityNameCache.erase(asme.id);
                }
                else if (asme.modifytype == MT::Rename) {
                    m_entityNameCache[asme.id] = asme.name;
                }
            });

        m_sceneChanged = EventSystem::Instance().subscribe(EventType::SceneChanged, [this](Event& event)
            {
                if (Scene* scene = SceneSystem::Instance().getCurrentScene())
                    m_entityNameCache = scene->getEntityList();
                else
                    m_entityNameCache.clear();
            });
    }

    void SceneInfoWidget::render()
    {
        ImGui::SetNextWindowSize(ImVec2(500, 440), ImGuiCond_FirstUseEver);
        if (!ImGui::Begin("Scene Info", nullptr))
        {
            ImGui::End();
            return;
        }

        m_requestDelete = false;
        for (auto& [id, name] : m_entityNameCache)
        {
            ImGui::PushID(static_cast<int>(id));
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
            if (Scene* scene = SceneSystem::Instance().getCurrentScene())
            {
                for (uint32_t id : m_selectedEntities)
                    scene->removeEntity(id);
            }
            m_selectedEntities.clear();
        }

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
        }
    }

    void SceneInfoWidget::_renamingHandle(uint32_t id)
    {
        ImGui::SetKeyboardFocusHere();

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

            if (Scene* scene = SceneSystem::Instance().getCurrentScene())
            {
                if (Entity* entity = scene->getEntity(id))
                {
                    entity->setName(newName);
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
            m_renameBuffer.clear();

            m_renameBuffer = name;
            m_renameBuffer.resize(256);
        }
    }
}