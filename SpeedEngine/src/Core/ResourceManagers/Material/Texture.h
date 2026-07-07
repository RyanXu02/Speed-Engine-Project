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

	class Texture
	{
	public:
		Texture(TextureType type);
		~Texture();

		std::string init(std::string_view filePath);
		void destroy();

		TextureType getType() const { return m_type; }
		std::string getVarName() const { return name; }
		bool isLoaded() const { return m_type != TextureType::None; }

		unsigned int glTextureId;
	private:
		TextureType m_type{ TextureType::None };
		std::string name{ "" };
		int width, height;
		unsigned int format;
	};

}

