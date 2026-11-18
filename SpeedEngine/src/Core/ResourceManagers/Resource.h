#pragma once


namespace SE
{
	std::string normalizeResourcePath(std::string_view path);
	using ResourceID = uint32_t;

	enum class ResourceType
	{
		Material,
		Shader,
		MeshResource,
		Font,
		Texture,
		Terrain
	};
} // namespace SE
