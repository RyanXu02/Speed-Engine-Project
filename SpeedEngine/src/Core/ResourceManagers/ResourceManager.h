#pragma once
#include "../SubSystems/SubSystem.h"

namespace SE
{
	// baseclass
	class ResourceManager : public SubSystem
	{
	public:
		ResourceManager() : SubSystem("ResourceManager") {}
		void init() override;
		
		uint32_t generateId();

		static ResourceManager& Instance()
		{
			return *s_instance;
		}

		std::string getString(std::string_view filePath);

		template<typename... Args>
		uint32_t addResource(ResourceType type, Args... args)
		{

		}

		Resource* getResource(uint32_t);


	private:
		static ResourceManager* s_instance;

		std::atomic<uint32_t> m_ids{ 1 }; // 0 for invalid id

		// path -> file contents
		std::unordered_map<std::string, std::string> m_stringCache;
	};

	enum class ResourceType
	{
		Texture,
		Shader,
	};

	class ResourceRegistry
	{
	public:
		void registerManager(ResourceType, ResourceManager* manager);
		void registerResource(uint32_t id, ResourceType type);
	private:
		std::unordered_map<ResourceType, ResourceManager*> m_managers;
		std::unordered_map<uint32_t, ResourceType> m_resourceTypes;
	};
}