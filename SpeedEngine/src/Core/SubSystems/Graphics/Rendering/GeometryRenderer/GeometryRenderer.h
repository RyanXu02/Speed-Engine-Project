#pragma once
#include "Core/SubSystems/Graphics/Rendering/Renderer.h"

namespace SE
{
	class MeshResource;

	// @brief Deferred geometry pass, draws the opaque submeshes into the G-buffer output
	class GeometryRenderer : public Renderer
	{
	public:
		explicit GeometryRenderer(Logger& logger) : Renderer(logger, "GeometryRenderer") {}

		void init() override;
		void render(RenderContext& context) override;
		void shutdown() override;

	private:
		uint32_t m_defaultShaderId = 0;
		uint32_t m_defaultMaterialId = 0;

		std::unordered_map<uint32_t, std::vector<uint32_t>> m_meshMaterials;
		const std::vector<uint32_t>& _getMeshMaterials(RenderContext& context, uint32_t meshResourceId, const MeshResource& meshResource);
	};
} // namespace SE
