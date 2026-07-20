#pragma once
#include "../Widget.h"
#include "../../Events/EventSubscription.h"

namespace SE 
{
	class ResourceBrowserWidget : public Widget
	{
	public:
		ResourceBrowserWidget();

		void render() override;
		void shutdown() override;

	private:
		std::unordered_map<uint32_t, ResourceType> m_resourceCache;
		EventSubscription m_resourceChanged;
	};

}

