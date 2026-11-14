#pragma once
#if defined(COMPILE_WITH_EDITOR)

#include "../../Events/EventSubscription.h"
#include "../Widget.h"
#include "../EditorSession.h"
#include "ComponentInspectorRegistry.h"


namespace SE
{
	class Entity;

	class PropertiesWidget : public VisibleWidget<PropertiesWidget>
	{
	public:
		explicit PropertiesWidget(EditorSession& session);

		void render() override;
		void shutdown() override;

	private:
		EditorSession& m_session;
		EventSubscription m_SceneChangedSubscription;

		uint32_t m_inspectedEntityId{ 0 };
		void resetDrafts();
		bool authoredEdit(bool changed);

		ComponentInspectorRegistry m_inspectors;

		std::string m_meshPathDraft;
		std::string m_meshPathError;
		bool m_meshDraftInitialized{ false };

		std::string m_terrainPathDraft;
		std::string m_terrainPathError;
		bool m_terrainDraftInitialized{ false };

		std::string m_physicsShapeMeshPathDraft;
		bool m_physicsShapeMeshDraftInitialized{ false };

		std::string m_soundPathDraft;
		std::string m_soundPathError;
		bool m_soundDraftInitialized{ false };

		std::string m_musicPathDraft;
		std::string m_musicPathError;
		bool m_musicDraftInitialized{ false };

	};
} // namespace SE
#endif
