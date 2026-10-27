#pragma once

namespace SE
{
	enum class TextureType
	{
		None = -1,
		Albedo,
		Normal,
		Height,
		AmbOcc,
		Roughness,
		Emissive,
		Metallic
	};

	// number of texture types, remember to update when adding a new TextureType
	constexpr size_t NUM_TEXTURE_TYPES = static_cast<size_t>(TextureType::Metallic) + 1;

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

		// @brief Initializes the texture as a 1x1 texture of a single color
		void initSolid(unsigned char r, unsigned char g, unsigned char b, unsigned char a);

		// @brief Deletes the texture from GPU
		void destroy();

		TextureType getType() const { return m_type; }
		std::string getVarName() const { return name; }
		bool isLoaded() const { return m_type != TextureType::None; }

		// @brief Gets the shared 1x1 fallback texture bound when a material has no texture of this type
		static Texture& getDefault(TextureType type);
		// @brief Deletes the shared fallback textures from GPU
		static void destroyDefaults();

		unsigned int glTextureId = 0;

	private:
		// default fallback texture for objects that doesnt have the full pbr materials system
		static inline std::array<std::unique_ptr<Texture>, NUM_TEXTURE_TYPES> s_defaults;

		TextureType m_type{ TextureType::None };
		std::string name{ "" };
		int width, height;
		unsigned int format;
	};

} // namespace SE
