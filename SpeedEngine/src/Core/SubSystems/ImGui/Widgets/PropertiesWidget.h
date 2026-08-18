#pragma once
#include "../Widget.h"

#include "../../../Scene/Entity/Component/Component.h"

#include "../../Events/EventSubscription.h"

namespace SE {

    class PropertiesWidget : public Widget
    {
    public:
        PropertiesWidget();

        void render();
        void shutdown();
    private:
		EventSubscription m_EntitySelectedSubscription;
		EventSubscription m_ActiveSceneModifiedSubscription;
		EventSubscription m_SceneChangedSubscription;

		uint32_t m_selectedEntityId{ 0 };
		Entity* m_cachedSelectedEntity{ nullptr };
    };

}

