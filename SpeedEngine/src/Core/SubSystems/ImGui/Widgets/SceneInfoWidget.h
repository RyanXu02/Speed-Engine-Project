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
		bool m_focusRename = false;
		std::string m_renameBuffer{};

		bool m_requestDelete = false;

		void _rightClickMenu(const std::string& name, uint32_t id);
		void _leftClickHandle(const std::string& name, uint32_t id);
		void _f2Handle(const std::string& name, uint32_t id);
		void _renamingHandle(uint32_t id);
	};
} // namespace SE
#endif
