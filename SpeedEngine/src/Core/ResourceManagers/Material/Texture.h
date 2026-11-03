#pragma once

#include "../Resource.h"

namespace SE
{
	// @brief GPU texture owned by ResourceManager
	class Texture : public Resource
	{
	public:
		Texture(uint32_t id, std::string_view name)
			: Resource(id, std::string(name))
		{}
		~Texture();

		Texture(const Texture&) = delete;
		Texture& operator=(const Texture&) = delete;
		Texture(Texture&&) = delete;
		Texture& operator=(Texture&&) = delete;

		// @brief Initializes the texture by loading it into GPU
		// @param filePath The file path of the texture file
		// @returns an empty string if successful, or an error message if failed
		std::string init(std::string_view filePath);

		// @brief Returns the OpenGL texture ID
		unsigned int getTextureId() const { return m_textureId; }

	private:
		unsigned int m_textureId = 0; // OpenGL texture ID, 0 if not initialized
	};

} // namespace SE
