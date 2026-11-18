#pragma once


namespace SE
{
	struct TextureData
	{
		int width = 0;
		int height = 0;
		int channels = 0;
		std::vector<unsigned char> pixels;
		std::vector<float> hdrPixels;
	};

	std::unique_ptr<TextureData> loadTextureData(const std::string& path);
} // namespace SE
