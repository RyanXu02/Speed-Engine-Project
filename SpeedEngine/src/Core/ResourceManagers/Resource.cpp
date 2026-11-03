#include "pch.h"

#include "Resource.h"

namespace SE
{
	std::string normalizeResourcePath(std::string_view path)
	{
		if (path.empty())
			return {};
		std::error_code error;
		auto absolutePath = std::filesystem::absolute(std::filesystem::path(path), error);
		if (error)
			return {};
		return absolutePath.lexically_normal().generic_string();
	}
} // namespace SE
