#pragma once
#if defined(COMPILE_WITH_EDITOR)
#include "../../Events/EventSubscription.h"
#include "../Widget.h"
#include "Core/SubSystems/ImGui/EditorSession.h"

namespace SE
{
	class SceneInfoWidget : public VisibleWidget<SceneInfoWidget>
	{
	public:
		explicit SceneInfoWidget(EditorSession& session);

		void render() override;
		void shutdown() override;

	private:
		EditorSession& m_session;

		EventSubscription m_sceneChanged;

		uint32_t m_renameEntityId = 0;
		std::vector<uint32_t> m_renameTargets;
		bool m_focusRename = false;
		std::string m_renameBuffer{};

		bool m_requestDelete = false;

		bool _rightClickMenu();
		void _beginRename(const std::string& name, uint32_t id);
		void _renamingHandle();
	};
} // namespace SE
#endif
