#include "pch.h"

#include "ShaderSource.h"

namespace SE
{
	std::unique_ptr<ShaderSource> loadShaderSource(const std::string& path, const std::string& fragmentPath)
	{
		auto read = [](const std::string& filePath, std::string& target)
		{
			std::ifstream file(filePath, std::ios::binary);
			if (!file)
				return false;
			target.assign(std::istreambuf_iterator<char>(file), {});
			return !file.bad() && !target.empty();
		};

		auto data = std::make_unique<ShaderSource>();

		const auto extension = std::filesystem::path(path).extension();
		if (fragmentPath.empty() && extension == ".comp" && read(path, data->compute))
			return data;

		if (extension == ".vert" && std::filesystem::path(fragmentPath).extension() == ".frag"
			&& read(path, data->vertex) && read(fragmentPath, data->fragment))
			return data;

		return {};
	}
} // namespace SE