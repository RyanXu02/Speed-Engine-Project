#pragma once
#include "../SubSystems/SubSystem.h"
#include "../Logger/Logger.h"
#include "Resource.h"
#include "Manager.h"

#include "Shader/ShaderManager.h"
#include "Material/MaterialManager.h"

namespace SE
{

	class ResourceManager : public SubSystem
	{
	public:

		ResourceManager() : SubSystem("ResourceManager") {}
		
		//@brief gets static instance (meyers singleton)
		//@returns reference to instance
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

		//@brief generates a resource id. atomic and does not reuse ids
		//@returns an id (uint32)
		uint32_t generateId();

		//@brief retrieves the contents of a text file
		//@param filePath file to open and retrieve contents
		//@returns filePath's entire contents
		std::string getString(std::string_view filePath);
		
		//@brief adds a resource to the Resource Manager
		//@param type the type of resource being added
		//@param args arguments to pass for type of resource being added (filepaths, names, etc)
		//@returns id of newly added resource
		template<typename... Args>
		uint32_t addResource(ResourceType type, Args&&... args)
		{
			switch (type)
			{
				case ResourceType::Shader:
					return _addResourceImpl<ResourceType::Shader>(std::forward<Args>(args)...);
				case ResourceType::Material:
					return _addResourceImpl<ResourceType::Material>(std::forward<Args>(args)...);
			}
			m_logger->warn("ResourceManager::addResource: Unhandled ResourceType");
			return 0;
		}

		//@brief retrieves a resource of requested type given an id
		//@param id the id to retrieve
		//@returns the requested resource of the specified type if it exists, nullptr if not
		template<typename T>
		T* getResource(uint32_t id) {
			auto it = m_resourceTypes.find(id);
			if (it != m_resourceTypes.end()) {
				if constexpr (std::is_same_v<T, Shader>) {
					return _getManager<ShaderManager>()->getShader(id);
				}
				else if constexpr (std::is_same_v<T, Material>) {
					return _getManager<MaterialManager>()->getMaterial(id);
				}
			}
			return nullptr;
		}


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
				if (auto ptr = static_cast<T*>(manager.get()))
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
				return _getManager<ShaderManager>()->addShader(generateId(), std::forward<Args>(args)...);
			}
			else if constexpr (Type == ResourceType::Material)
			{
				return _getManager<MaterialManager>()->addMaterial(generateId(), std::forward<Args>(args)...);
			}
			else
			{
				static_assert(false, "Unhandled ResourceType in _addResourceImpl");
				return 0;
			}
		}

	};
}