#pragma once

namespace SE
{
	class Texture
	{
	public:
		Texture(TextureType type);
		~Texture();

		void init();
		void destroy();

		TextureType getType() const { return m_type; }
		std::string getVarName() const { return name; }
		bool isLoaded() const { return m_type != TextureType::None; }

		unsigned int glTextureId;
	private:
		TextureType m_type{ TextureType::None };
		std::string name{ "" };
	};

	enum class TextureType {
		None = -1,
		Albedo,
		Normal,
		Height,
		AmbOcc,
		Roughness
	};
}

