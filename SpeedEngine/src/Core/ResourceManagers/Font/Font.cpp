#include "pch.h"

#include "Font.h"

namespace SE
{
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
