#pragma once
#include "../../Events/EventSubscription.h"
#include "../Widget.h"


namespace SE
{
	class Entity;

	class PropertiesWidget : public Widget
	{
	public:
		PropertiesWidget();

		void render() override;
		void shutdown() override;

	private:
		EventSubscription m_EntitySelectedSubscription;
		EventSubscription m_ActiveSceneModifiedSubscription;
		EventSubscription m_SceneChangedSubscription;

		uint32_t m_selectedEntityId{ 0 };
	};

} // namespace SE
