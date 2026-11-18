#include "pch.h"

#include "Font.h"
#include "Core/Logger/LoggerProxy.h"

#include <ft2build.h>
#include FT_FREETYPE_H

namespace SE
{
	std::unique_ptr<FontData> loadFontData(std::string_view path, uint32_t pixelHeight, Logger& logger)
	{
		LoggerProxy log(logger, "Font");

		FT_Library library = nullptr;
		if (!pixelHeight || FT_Init_FreeType(&library))
		{
			log.error("Cannot load font '{}': FreeType unavailable or pixel height is zero", path);
			return {};
		}

		std::unique_ptr<FT_LibraryRec_, decltype(&FT_Done_FreeType)> libraryOwner(library, FT_Done_FreeType);
		FT_Face face = nullptr;
		if (const auto error = FT_New_Face(library, std::string(path).c_str(), 0, &face))
		{
			log.error("Failed to open font '{}': {}", path, error);
			return {};
		}

		std::unique_ptr<FT_FaceRec_, decltype(&FT_Done_Face)> faceOwner(face, FT_Done_Face);
		auto error = FT_Select_Charmap(face, FT_ENCODING_UNICODE);
		if (!error)
			error = FT_Set_Pixel_Sizes(face, 0, pixelHeight);
		if (error)
		{
			log.error("Failed to prepare font '{}' at {} pixels: {}", path, pixelHeight, error);
			return {};
		}

		auto data = std::make_unique<FontData>();
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
				log.error("Failed to rasterize character {} in '{}': {}", FontData::FirstCharacter + i, path, error);
				return {};
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
		return data;
	}

	// @brief Reads the next UTF-8 codepoint from the text starting at offset, and advances the offset
	// @param text The text to read from
	// @param offset The current offset in the text, which will be updated to point to the next codepoint
	// @return The next codepoint, or '?' if the UTF-8 sequence is invalid
	static uint32_t nextCodepoint(std::string_view text, size_t& offset)
	{
		const auto first = static_cast<unsigned char>(text[offset++]);
		if (first < 0x80)
			return first;

		const int length = first >= 0xC2 && first <= 0xDF ? 2
			: first >= 0xE0 && first <= 0xEF ? 3
			: first >= 0xF0 && first <= 0xF4 ? 4 : 0;
		if (!length || offset + length - 1 > text.size())
			return '?';

		uint32_t codepoint = first & (0x7F >> length);
		for (int i = 1; i < length; ++i)
		{
			const auto byte = static_cast<unsigned char>(text[offset]);
			if ((byte & 0xC0) != 0x80)
				return '?';
			++offset;
			codepoint = (codepoint << 6) | (byte & 0x3F);
		}
		if ((length == 3 && codepoint < 0x800) || (length == 4 && codepoint < 0x10000)
			|| (codepoint >= 0xD800 && codepoint <= 0xDFFF) || codepoint > 0x10FFFF)
			return '?';
		return codepoint;
	}

	TextMetrics FontData::layout(std::string_view text, const GlyphVisitor& visitor) const
	{
		TextMetrics result;
		result.ascent = ascent;
		result.descent = descent;
		result.lineAdvance = lineAdvance;
		result.baseline = ascent;
		if (text.empty())
			return result;

		result.lineCount = 1;
		glm::vec2 pen{ 0.0f, ascent };
		int previous = -1;
		for (size_t offset = 0; offset < text.size();)
		{
			uint32_t codepoint = nextCodepoint(text, offset);
			if (codepoint == '\r' || codepoint == '\n')
			{
				if (codepoint == '\r' && offset < text.size() && text[offset] == '\n')
					++offset;
				result.extent.x = std::max(result.extent.x, pen.x);
				pen.x = 0.0f;
				pen.y += lineAdvance;
				++result.lineCount;
				previous = -1;
				continue;
			}
			if (codepoint < FirstCharacter || codepoint >= FirstCharacter + GlyphCount)
				codepoint = '?';
			const auto index = codepoint - FirstCharacter;
			if (previous >= 0)
				pen.x += kerning[previous * GlyphCount + index];
			const auto& glyph = glyphs[index];
			const glm::vec2 topLeft{ pen.x + glyph.bearing.x, pen.y - glyph.bearing.y };
			if (glyph.size.x > 0.0f && glyph.size.y > 0.0f)
			{
				const glm::vec2 bottomRight = topLeft + glyph.size;
				if (!result.hasInk)
				{
					result.inkMin = topLeft;
					result.inkMax = bottomRight;
					result.hasInk = true;
				}
				else
				{
					result.inkMin.x = std::min(result.inkMin.x, topLeft.x);
					result.inkMin.y = std::min(result.inkMin.y, topLeft.y);
					result.inkMax.x = std::max(result.inkMax.x, bottomRight.x);
					result.inkMax.y = std::max(result.inkMax.y, bottomRight.y);
				}
				if (visitor)
					visitor(glyph, topLeft);
			}
			pen.x += glyph.advance;
			previous = static_cast<int>(index);
		}
		result.extent.x = std::max(result.extent.x, pen.x);
		result.extent.y = result.lineCount * lineAdvance;
		return result;
	}
} // namespace SE
