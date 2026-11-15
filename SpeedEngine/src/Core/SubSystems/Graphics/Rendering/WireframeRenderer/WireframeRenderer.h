#pragma once
#include "Core/SubSystems/Graphics/Rendering/Renderer.h"

namespace SE
{
	// @brief Renders all meshes and terrain in wireframe mode
	class WireframeRenderer : public Renderer
	{
	public:
		explicit WireframeRenderer(Logger& logger)
		    : Renderer(logger, "WireframeRenderer")
		{}

		void init() override;
		void render(RenderContext& context) override;
		void shutdown() override;

	private:
		uint32_t m_meshShaderId = 0;
		uint32_t m_terrainShaderId = 0;
		uint32_t m_defaultMaterialId = 0;
		uint32_t m_defaultTextureId = 0;
	};
} // namespace SE
