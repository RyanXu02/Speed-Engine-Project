#pragma once

namespace SE
{
	enum class TextureType {
		None = -1,
		Albedo,
		Normal,
		Height,
		AmbOcc,
		Roughness
	};

	// @brief Represents a texture that can be used in materials
	// A texture is owned by a material
	class Texture
	{
	public:
		Texture(TextureType type);
		~Texture();

		// @brief Initializes the texture by loading it into GPU
		// @param filePath The file path of the texture file
		// @returns an empty string if successful, or an error message if failed
		std::string init(std::string_view filePath);

		// @brief Deletes the texture from GPU
		void destroy();

		TextureType getType() const { return m_type; }
		std::string getVarName() const { return name; }
		bool isLoaded() const { return m_type != TextureType::None; }

		unsigned int glTextureId = 0;
	private:
		TextureType m_type{ TextureType::None };
		std::string name{ "" };
		int width, height;
		unsigned int format;
	};

}

