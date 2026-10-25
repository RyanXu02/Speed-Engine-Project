#pragma once
#if defined(COMPILE_WITH_EDITOR)

#include "../../Events/EventSubscription.h"
#include "../Widget.h"
#include "ComponentInspectorRegistry.h"


namespace SE
{
	class Entity;

	class PropertiesWidget : public VisibleWidget<PropertiesWidget>
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

		ComponentInspectorRegistry m_inspectors;
		std::string m_meshPathDraft;
		std::string m_meshPathError;
		bool m_meshDraftInitialized{ false };

	};
} // namespace SE
#endif
