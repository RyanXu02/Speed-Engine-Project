#pragma once
#include "Core/ResourceManagers/Resource.h"

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

	struct TextureBinding
	{
		TextureType usage;
		ResourceID textureId;
	};

	struct Material
	{
		ResourceID shaderId = 0;
		std::vector<TextureBinding> textures;
	};
} // namespace SE
