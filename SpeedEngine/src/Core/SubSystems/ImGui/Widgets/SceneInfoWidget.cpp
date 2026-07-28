#include "pch.h"
#include "SceneInfoWidget.h"

#include "../../Scenes/SceneSystem.h"

#include "imgui/imgui.h"

namespace SE
{
    SceneInfoWidget::SceneInfoWidget() : Widget("Scene Info")
    {
        m_activeSceneModified = EventSystem::Instance().subscribe(EventType::ActiveSceneModified, [this](Event& event)
            {
                ActiveSceneModified& asme = static_cast<ActiveSceneModified&>(event);
                if (asme.adding) {
                    m_entityNameCache[asme.id] = asme.name;
                }
                else {
                    m_entityNameCache.erase(asme.id);
                }
            });

        m_sceneChanged = EventSystem::Instance().subscribe(EventType::SceneChanged, [this](Event& event)
            {
                SceneChanged& sce = static_cast<SceneChanged&>(event);
                m_entityNameCache = SceneSystem::Instance().getCurrentScene()->getEntityList();
            });
    }

	void SceneInfoWidget::render()
	{
        ImGui::SetNextWindowSize(ImVec2(500, 440), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Scene Info", nullptr))
        {
            for (auto [id, name] : m_entityNameCache )
            {
                char label[128];
                sprintf_s(label, name.c_str());
                if (ImGui::Selectable(label, false, ImGuiSelectableFlags_SelectOnNav))
                {
                    // onclick handle properties panel stuff
                }
            }
            
        }
        ImGui::End();
	}

    void SceneInfoWidget::shutdown()
    {
		m_entityNameCache.clear();
    }
}