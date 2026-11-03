#pragma once

#include "../Manager.h"
#include "Font.h"

struct FT_LibraryRec_;

namespace SE
{
	class FontManager : public Manager
	{
		friend class ResourceManager;

	public:
		FontManager() : Manager(ResourceType::Font, "FontManager") {}
		void init() override;
		void shutdown() override;

	private:
		// @brief Loads a font from the specified path at the given pixel height, and returns its unique ID
		// @param id The unique ID to assign to the font
		// @param path The path to the font file
		// @param pixelHeight The pixel height of the font
		// @param name The name of the font
		// @return The unique ID of the loaded font
		uint32_t addFont(uint32_t id, std::string_view path, uint32_t pixelHeight, std::string_view name);
		// @brief Retrieves the font with the specified ID
		// @param id The unique ID of the font
		// @return A pointer to the font, or nullptr if not found
		Font* getFont(uint32_t id);
		// @brief Removes the font with the specified ID
		// @param id The unique ID of the font
		// @return True if the font was removed, false if not found
		bool removeFont(uint32_t id);

		FT_LibraryRec_* m_library = nullptr;
		std::unordered_map<uint32_t, std::unique_ptr<Font>> m_fonts; // Maps font ID to Font object
		std::map<std::pair<std::string, uint32_t>, uint32_t> m_variants; // Maps (path, pixelHeight) to font ID
	};
} // namespace SE
