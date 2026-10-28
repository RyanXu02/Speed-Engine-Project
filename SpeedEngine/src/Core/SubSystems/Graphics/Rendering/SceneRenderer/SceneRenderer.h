#pragma once
#include "../Renderer.h"

namespace SE
{
	// @brief Submits the completed frame's draw items
	class SceneRenderer : public Renderer
	{
	public:
		explicit SceneRenderer(Logger& logger) : Renderer(logger, "SceneRenderer") {}

		void init() override;
		void shutdown() override;

		void render(RenderContext& context) override;
		void setDefaultShaderId(uint32_t id) { m_defaultShaderId = id; }
		void setDefaultMaterialId(uint32_t id) { m_defaultMaterialId = id; }

	private:
		// Default shader for meshes without material
		uint32_t m_defaultShaderId = 0;
		uint32_t m_defaultMaterialId = 0;
	};
} // namespace SE
