#pragma once
#include "Core/Logger/Logger.h"
#include "Core/SubSystems/SubSystem.h"

#include "Resource.h"

#include "Shader/ShaderManager.h"
#include "MeshResource/MeshResourceManager.h"
#include "Material/MaterialManager.h"
#include "Font/FontManager.h"

namespace SE
{
	struct ShaderLoadDesc
	{
		std::string_view vertexShaderPath;
		std::string_view fragmentShaderPath;
		std::string_view name;
	};

	struct MeshLoadDesc
	{
		std::string_view path;
		std::string_view name;
	};

	struct FontLoadDesc
	{
		std::string_view path;
		uint32_t pixelHeight;
		std::string_view name;
	};

	struct MaterialDesc
	{
		uint32_t shaderId;
		std::span<const std::pair<TextureType, std::string_view>> textures;
		std::string_view name;
	};

	// @brief Public interface for managing all resources in the engine.
	class ResourceManager : public SubSystem
	{
	public:
		friend class Engine;
		friend class ShaderManager;

		void init() override;
		void shutdown() override;

		//@brief generates a resource id. atomic and does not reuse ids
		//@returns an id (uint32)
		uint32_t generateId();

		//@brief retrieves the contents of a text file
		//@param filePath file to open and retrieve contents
		//@returns pointer to filePath's entire contents	
		const std::string* getString(std::string_view filePath);

		// @brief loads or reuses a shader by source paths. Name is only a display label
		// @param vertexShaderPath path to the vertex shader file
		// @param fragmentShaderPath path to the fragment shader file
		// @param name name of the shader
		uint32_t loadShader(const ShaderLoadDesc& desc);
		// @brief loads or reuses a mesh by file path. Name is only a display label
		// @param path path to the mesh file
		// @param name name of the mesh
		uint32_t loadMesh(const MeshLoadDesc& desc);
		// @brief loads or reuses a font by file path and pixel height. Name is only a display label
		// @param path path to the font file
		// @param pixelHeight height of the font in pixels
		// @param name name of the font
		uint32_t loadFont(const FontLoadDesc& desc);
		// @brief creates a new material, even if another material has the same name
		// @param shaderId id of the shader to use for the material
		// @param textures array of texture types and paths to use for the material
		// @param name name of the material
		uint32_t createMaterial(const MaterialDesc& desc);

		//@brief retrieves a resource of requested type given an id (Not prefered,
		// used the other one. It is callers responsibility to ensure the type is
		// correct)
		//@param id the id to retrieve
		//@returns the requested resource of the specified type if it exists, nullptr
		// if not
		template<typename T>
		T* getResource(uint32_t id)
		{
			auto it = m_resourceTypes.find(id);
			if (it != m_resourceTypes.end())
			{
				if constexpr (std::is_same_v<T, Shader>)
				{
					return m_shaderManager.getShader(id);
				}
				else if constexpr (std::is_same_v<T, Material>)
				{
					return m_materialManager.getMaterial(id);
				}
				else if constexpr (std::is_same_v<T, MeshResource>)
				{
					return m_meshManager.getMeshResource(id);
				}
				else if constexpr (std::is_same_v<T, Font>)
				{
					return m_fontManager.getFont(id);
				}
				// ...
				else
				{
					m_logger->warn("ResourceManager::getResource: Unhandled type T");
					return nullptr;
				}
			}
			return nullptr;
		}

		//@brief retrieves a resource given an id
		//@param id the id of a resource to retrieve
		//@returns a the resource as a Resource* if it exists, nullptr if not
		Resource* getResource(uint32_t id);

#if defined(COMPILE_WITH_EDITOR)
		//@brief gets the resource type map as a copy
		std::unordered_map<uint32_t, ResourceType> getInitialRTMap() const { return m_resourceTypes; }
#endif

		//@brief removes a resource given an id
		//@param id the id of the resource to remove
		//@returns true if successfully removed, false if not
		bool removeResource(uint32_t id);

	private:
		// @brief removes the cached shader source code
		// @param paths the pair of vertex and fragment shader paths to remove from cache
		void forgetShaderSources(const std::pair<std::string, std::string>& paths);

		// singleton instance, use singleton Engine to access its subsystems
		ResourceManager()
		    : SubSystem("ResourceManager")
		{}

		// universal id for all resources in the engine
		std::atomic<uint32_t> m_ids{ 1 }; // 0 for invalid id

		ShaderManager m_shaderManager;
		MaterialManager m_materialManager;
		MeshResourceManager m_meshManager;
		FontManager m_fontManager;

		// path -> file contents
		std::unordered_map<std::string, std::string> m_stringCache;
		// id -> ResourceType
		std::unordered_map<uint32_t, ResourceType> m_resourceTypes;
	};
} // namespace SE
