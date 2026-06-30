#pragma once
#include "../SubSystems/SubSystem.h"
#include "../Logger/Logger.h"
#include "Resource.h"
#include "Manager.h"

#include "Shader/ShaderManager.h"
#include "Texture/TextureManager.h"

namespace SE
{

	class ResourceManager : public SubSystem
	{
	public:

		ResourceManager() : SubSystem("ResourceManager") {}
		

		static ResourceManager& Instance()
		{
			if (s_instance)
			{
				return *s_instance;
			}

			static ResourceManager instance;
			s_instance = &instance;
			return *s_instance;
		}

		void init() override;
		void shutdown() override;

		uint32_t generateId();

		std::string getString(std::string_view filePath);
		
		template<typename... Args>
		uint32_t addResource(ResourceType type, Args&&... args)
		{
			switch (type)
			{
				case ResourceType::Shader:
					return _addResourceImpl<ResourceType::Shader>(std::forward<Args>(args)...);
				case ResourceType::Texture:
					return _addResourceImpl<ResourceType::Texture>(std::forward<Args>(args)...);
			}
			m_logger->warn("ResourceManager::addResource: Unhandled ResourceType");
			return 0;
		}

		Resource* getResource(uint32_t);


	private:
		// static instance for public
		static ResourceManager* s_instance;

		// universal id for all resources in the engine
		std::atomic<uint32_t> m_ids{ 1 }; // 0 for invalid id

		// All registered managers
		std::vector<std::unique_ptr<Manager>> m_managers;

		// path -> file contents
		std::unordered_map<std::string, std::string> m_stringCache;
		// id -> ResourceType
		std::unordered_map<uint32_t, ResourceType> m_resourceTypes;

		// get manager of type T
		template<typename T>
		T* _getManager()
		{
			for (auto& manager : m_managers)
			{
				if (auto ptr = dynamic_cast<T*>(manager.get()))
				{
					return ptr;
				}
			}
			return nullptr;
		}

		template<ResourceType Type, typename... Args>
		uint32_t _addResourceImpl(Args&&... args)
		{
			if constexpr (Type == ResourceType::Shader)
			{
				return _getManager<ShaderManager>()->addShader(generateId(), std::forward<Args>(args)...); // expects 3 strings
			}
			else if constexpr (Type == ResourceType::Texture)
			{
				return _getManager<TextureManager>()->loadTexture(generateId(), std::forward<Args>(args)...); // expects 1 arg
			}
			else
			{
				static_assert(false, "Unhandled ResourceType in _addResourceImpl");
				return 0;
			}
		}

	};
}