#pragma once
#include "../SubSystems/SubSystem.h"
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
			static ResourceManager instance;
			s_instance = &instance;
			return *s_instance;
		}

		void init() override;
		void shutdown() override;

		uint32_t generateId();

		std::string getString(std::string_view filePath);
		
		template<typename... Args>
		uint32_t addResource(ResourceType type, Args... args)
		{
			switch (type)
			{
			case ResourceType::Texture:
				_getManager<TextureManager>()->loadTexture(args...);
				break;
			case ResourceType::Shader:
				_getManager<ShaderManager>()->addShader(generateId(), args...);
				break;
			}
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

	};
}