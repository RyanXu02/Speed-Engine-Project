#pragma once

#include <glm/vec2.hpp>

namespace SE
{
	class Logger;

	struct TextMetrics
	{
		glm::vec2 extent{ 0.0f };
		glm::vec2 inkMin{ 0.0f };
		glm::vec2 inkMax{ 0.0f };
		float ascent = 0.0f;
		float descent = 0.0f;
		float lineAdvance = 0.0f;
		float baseline = 0.0f;
		uint32_t lineCount = 0;
		bool hasInk = false;
	};

	struct FontGlyph
	{
		glm::vec2 size{ 0.0f };
		glm::vec2 bearing{ 0.0f };
		glm::vec2 uvMin{ 0.0f };
		glm::vec2 uvMax{ 0.0f };
		float advance = 0.0f;
	};

	struct FontData
	{
		static constexpr unsigned int FirstCharacter = 32;
		static constexpr unsigned int GlyphCount = 95;
		using GlyphVisitor = std::function<void(const FontGlyph&, const glm::vec2&)>;

		TextMetrics measure(std::string_view text) const { return layout(text); }
		// @brief Measures the text and invokes the visitor for each glyph in the text
		// @param text The text to measure and layout
		// @param visitor A function that will be called for each glyph in the text, with the glyph and its position
		// @return The overall metrics of the text, including extent, ink bounds, ascent, descent, line advance, and line count
		TextMetrics layout(std::string_view text, const GlyphVisitor& visitor = {}) const;

		uint32_t pixelHeight = 0;
		float ascent = 0.0f;
		float descent = 0.0f;
		float lineAdvance = 0.0f;
		std::array<FontGlyph, GlyphCount> glyphs{};
		std::array<float, GlyphCount * GlyphCount> kerning{};
		uint32_t atlasWidth = 0;
		uint32_t atlasHeight = 0;
		std::vector<unsigned char> atlasPixels;
	};

	std::unique_ptr<FontData> loadFontData(std::string_view path, uint32_t pixelHeight, Logger& logger);
} // namespace SE
