#include "pch.h"
#include "ResourceBrowserWidget.h"

#include "../../../ResourceManagers/ResourceManager.h"
#include "../../Events/Event.h"

namespace SE 
{
	ResourceBrowserWidget::ResourceBrowserWidget() : Widget("ResourceBrowserWidget") {
		m_resourceCache = ResourceManager::Instance().getInitialRTMap();

		m_resourceChanged = EventSystem::Instance().subscribe(EventType::ResourceChanged, [this](Event& event) 
			{
				ResourceChanged& rce = static_cast<ResourceChanged&>(event);
				if (rce.adding) m_resourceCache[rce.id] = rce.type; //adding resource
				else m_resourceCache.erase(rce.id); //deleting resource
			});
	}

	void ResourceBrowserWidget::render() {

	}

	void ResourceBrowserWidget::shutdown() {

	}

}