#pragma once
#include "../SubSystems/SubSystem.h"

namespace SE
{
	enum class ResourceType
	{
		Texture,
		Shader,
	};

	class ResourceManager : public SubSystem
	{
	class ShaderManager;
	public:
		ResourceManager() : SubSystem("ResourceManager") {}
		

		static ResourceManager& Instance()
		{
			static ResourceManager instance;
			s_instance = &instance;
			return *s_instance;
		}

		void registerManager(Manager& manager);

		uint32_t generateId();

		std::string getString(std::string_view filePath);
		
		template<typename... Args>
		uint32_t addResource(ResourceType type, Args... args)
		{
			switch (type)
			{
			case Texture:
				break;
			case Shader:
				m_shaderManager->_addShader(generateId(), args...);
			}
		}

		//Resource* getResource(uint32_t);


	private:
		static ResourceManager* s_instance;

		ShaderManager* m_shaderManager;

		std::atomic<uint32_t> m_ids{ 1 }; // 0 for invalid id

		// path -> file contents
		std::unordered_map<std::string, std::string> m_stringCache;
	};
}