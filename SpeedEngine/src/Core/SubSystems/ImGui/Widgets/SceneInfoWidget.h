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

		EventSubscription m_activeSceneModified;
		EventSubscription m_sceneChanged;
	};
}
