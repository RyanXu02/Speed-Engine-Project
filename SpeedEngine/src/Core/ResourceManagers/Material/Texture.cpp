#include "pch.h"

#include "Texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace SE
{
	std::unique_ptr<TextureData> loadTextureData(const std::string& path)
	{
		stbi_set_flip_vertically_on_load_thread(true);
		auto data = std::make_unique<TextureData>();
		if (stbi_is_hdr(path.c_str()))
		{
			int channels = 0;
			const std::unique_ptr<float, decltype(&stbi_image_free)> image(
				stbi_loadf(path.c_str(), &data->width, &data->height, &channels, 3), stbi_image_free);
			if (!image)
				return {};
			data->channels = 3;
			data->hdrPixels.assign(image.get(), image.get() + static_cast<size_t>(data->width) * data->height * 3);
			return data;
		}
		const std::unique_ptr<unsigned char, decltype(&stbi_image_free)> image(
			stbi_load(path.c_str(), &data->width, &data->height, &data->channels, 0), stbi_image_free);
		if (!image)
			return {};
		const auto size = static_cast<size_t>(data->width) * data->height * data->channels;
		data->pixels.assign(image.get(), image.get() + size);
		return data;
	}
} // namespace SE
