#pragma once
#include "../Widget.h"

#include "../../../Scene/Entity/Entity.h"
#include "../../Events/EventSubscription.h"


namespace SE
{
	class SceneInfoWidget : public Widget
	{
	public:
		SceneInfoWidget();

		void render() override;
		void shutdown() override;


	private:
		std::unordered_map<uint32_t, std::string> m_entityNameCache;

		std::unordered_set<uint32_t> m_selectedEntities;

		EventSubscription m_activeSceneModified;
		EventSubscription m_sceneChanged;

		uint32_t m_renameEntityId = 0;
		std::string m_renameBuffer{};

		bool m_requestDelete = false;

		void _rightClickMenu(const std::string& name,uint32_t id);
		void _leftClickHandle(const std::string& name, uint32_t id);
		void _f2Handle(const std::string& name, uint32_t id);
		void _renamingHandle(uint32_t id);
	};
}
