#pragma once
#include "Core/SubSystems/Graphics/Rendering/Renderer.h"

namespace SE
{
	class Terrain;
	class Shader;
	class Texture;

	// @brief Deferred geometry pass, draws the opaque terrain mesh into the G-buffer output
	class TerrainRenderer : public Renderer
	{
	public:
		explicit TerrainRenderer(Logger& logger) : Renderer(logger, "TerrainRenderer") {}

		void init() override;
		void render(RenderContext& context) override;
		void shutdown() override;

	private:
		uint32_t m_defaultShaderId = 0;
		uint32_t m_defaultMaterialId = 0;
		uint32_t m_defaultTextureId = 0;

		const uint32_t m_splatMapSlot = 22; // pbr uses 3 * 0-6

		std::unordered_map<uint32_t, std::vector<uint32_t>> m_terrainMaterials;
		const std::vector<uint32_t>& _getTerrainMaterials(RenderContext& context, uint32_t terrainResourceId, const Terrain& terrain);
		void _bindSplatMapTexture(const Texture& texture, Shader& shader);
		void _unbindSplatMapTexture();
	};
} // namespace SE
