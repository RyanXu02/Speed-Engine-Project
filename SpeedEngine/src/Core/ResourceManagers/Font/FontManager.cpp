#include "pch.h"

#include "FontManager.h"
#include "Core/Logger/Logger.h"

#include <ft2build.h>
#include FT_FREETYPE_H

namespace SE
{
	void FontManager::init()
	{
		Manager::init();
		if (const auto error = FT_Init_FreeType(&m_library))
		{
			m_library = nullptr;
			m_logger->error("FreeType initialization failed: {}", error);
		}
	}

	void FontManager::shutdown()
	{
		m_fonts.clear();
		m_variants.clear();
		if (m_library)
			FT_Done_FreeType(m_library);
		m_library = nullptr;
		Manager::shutdown();
	}

	uint32_t FontManager::addFont(uint32_t id, std::string_view path, uint32_t pixelHeight, std::string_view name)
	{
		if (!m_library || !pixelHeight)
		{
			m_logger->error("Cannot load font '{}': FreeType unavailable or pixel height is zero", path);
			return 0;
		}
		const auto key = std::make_pair(std::string(path), pixelHeight);
		if (auto it = m_variants.find(key); it != m_variants.end())
			return it->second;

		FT_Face face = nullptr;
		if (const auto error = FT_New_Face(m_library, key.first.c_str(), 0, &face))
		{
			m_logger->error("Failed to open font '{}': {}", path, error);
			return 0;
		}
		auto error = FT_Select_Charmap(face, FT_ENCODING_UNICODE);
		if (!error)
			error = FT_Set_Pixel_Sizes(face, 0, pixelHeight);
		if (error)
		{
			m_logger->error("Failed to prepare font '{}' at {} pixels: {}", path, pixelHeight, error);
			FT_Done_Face(face);
			return 0;
		}

		auto data = std::make_shared<FontData>();
		data->pixelHeight = pixelHeight;
		data->ascent = face->size->metrics.ascender / 64.0f;
		data->descent = -face->size->metrics.descender / 64.0f;
		data->lineAdvance = std::max(face->size->metrics.height / 64.0f, data->ascent + data->descent);
		data->atlasWidth = 512;
		std::array<std::vector<unsigned char>, FontData::GlyphCount> bitmaps;
		std::array<FT_UInt, FontData::GlyphCount> indices{};
		for (unsigned int i = 0; i < FontData::GlyphCount; ++i)
		{
			indices[i] = FT_Get_Char_Index(face, FontData::FirstCharacter + i);
			error = FT_Load_Glyph(face, indices[i], FT_LOAD_RENDER | FT_LOAD_NO_BITMAP);
			if (error)
			{
				m_logger->error("Failed to rasterize character {} in '{}': {}", FontData::FirstCharacter + i, path, error);
				FT_Done_Face(face);
				return 0;
			}
			const auto& bitmap = face->glyph->bitmap;
			auto& glyph = data->glyphs[i];
			glyph.size = glm::vec2(bitmap.width, bitmap.rows);
			glyph.bearing = glm::vec2(face->glyph->bitmap_left, face->glyph->bitmap_top);
			glyph.advance = face->glyph->advance.x / 64.0f;
			data->atlasWidth = std::max(data->atlasWidth, bitmap.width + 2);
			bitmaps[i].resize(static_cast<size_t>(bitmap.width) * bitmap.rows);
			for (unsigned int row = 0; row < bitmap.rows && bitmap.width > 0; ++row)
			{
				const auto* source = bitmap.pitch >= 0 ? bitmap.buffer + row * bitmap.pitch
					: bitmap.buffer + (bitmap.rows - 1 - row) * -bitmap.pitch;
				std::memcpy(bitmaps[i].data() + static_cast<size_t>(row) * bitmap.width, source, bitmap.width);
			}
		}
		if (FT_HAS_KERNING(face))
		{
			for (unsigned int left = 0; left < FontData::GlyphCount; ++left)
				for (unsigned int right = 0; right < FontData::GlyphCount; ++right)
				{
					FT_Vector adjustment{};
					FT_Get_Kerning(face, indices[left], indices[right], FT_KERNING_DEFAULT, &adjustment);
					data->kerning[left * FontData::GlyphCount + right] = adjustment.x / 64.0f;
				}
		}
		FT_Done_Face(face);

		uint32_t x = 1, y = 1, rowHeight = 0;
		for (auto& glyph : data->glyphs)
		{
			const auto width = static_cast<uint32_t>(glyph.size.x);
			const auto height = static_cast<uint32_t>(glyph.size.y);
			if (x + width + 1 > data->atlasWidth)
			{
				x = 1;
				y += rowHeight + 1;
				rowHeight = 0;
			}
			glyph.uvMin = glm::vec2(x, y);
			glyph.uvMax = glyph.uvMin + glyph.size;
			x += width + 1;
			rowHeight = std::max(rowHeight, height);
		}
		data->atlasHeight = y + rowHeight + 1;
		data->atlasPixels.resize(static_cast<size_t>(data->atlasWidth) * data->atlasHeight, 0);
		const glm::vec2 atlasSize(data->atlasWidth, data->atlasHeight);
		for (unsigned int i = 0; i < FontData::GlyphCount; ++i)
		{
			auto& glyph = data->glyphs[i];
			const auto width = static_cast<uint32_t>(glyph.size.x);
			const auto height = static_cast<uint32_t>(glyph.size.y);
			const size_t start = static_cast<size_t>(glyph.uvMin.y) * data->atlasWidth + static_cast<size_t>(glyph.uvMin.x);
			for (uint32_t row = 0; row < height && width > 0; ++row)
				std::memcpy(data->atlasPixels.data() + start + static_cast<size_t>(row) * data->atlasWidth,
					bitmaps[i].data() + static_cast<size_t>(row) * width, width);
			glyph.uvMin /= atlasSize;
			glyph.uvMax /= atlasSize;
		}

		m_fonts.emplace(id, std::make_unique<Font>(id, std::string(name), std::move(data)));
		m_variants.emplace(key, id);
		m_logger->info("Loaded font '{}' at {} pixels: ID {}", path, pixelHeight, id);
		return id;
	}

	Font* FontManager::getFont(uint32_t id)
	{
		auto it = m_fonts.find(id);
		return it == m_fonts.end() ? nullptr : it->second.get();
	}

	bool FontManager::removeFont(uint32_t id)
	{
		if (!m_fonts.erase(id))
			return false;
		std::erase_if(m_variants, [id](const auto& entry) { return entry.second == id; });
		return true;
	}
} // namespace SE
