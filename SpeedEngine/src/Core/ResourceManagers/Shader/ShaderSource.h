#pragma once

namespace SE
{
	// @brief CPU shader source for a shader program
	struct ShaderSource
	{
		std::string vertex;
		std::string fragment;
		std::string compute;
	};

	std::unique_ptr<ShaderSource> loadShaderSource(const std::string& path, const std::string& fragmentPath);
} // namespace SE