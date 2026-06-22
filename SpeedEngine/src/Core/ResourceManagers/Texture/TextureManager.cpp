#include "pch.h"
#include "TextureManager.h"
#include "../../Logger/Logger.h"

namespace SE
{
	void TextureManager::init()
	{
		ResourceManager::init();
		m_logger->warn("TextureManager initialized.");
	}

	void TextureManager::shutdown()
	{
		m_logger->warn("TextureManager shutting down.");
		ResourceManager::shutdown();
	}
}