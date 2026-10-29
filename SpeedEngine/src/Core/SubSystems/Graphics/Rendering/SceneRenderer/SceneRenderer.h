#pragma once
#include "../Renderer.h"

namespace SE
{
	class MeshResource;
	class Shader;

	// Note: shaders must match the max lights value defined here
	constexpr size_t MAX_LIGHTS = 16;

	struct LightingSettings
	{
		float ambientStrength = 0.03f;
		float gamma = 2.2f;
	};

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

		LightingSettings& getLightingSettings() { return m_lightingSettings; }

	private:
		void _setLightUniforms(const RenderContext& context, Shader& shader);
		LightingSettings m_lightingSettings;

		// Default shader for meshes without material
		uint32_t m_defaultShaderId = 0;
		uint32_t m_defaultMaterialId = 0;

		std::unordered_map<uint32_t, std::vector<uint32_t>> m_meshMaterials;
		const std::vector<uint32_t>& _getMeshMaterials(RenderContext& context, uint32_t meshResourceId, const MeshResource& meshResource);
	};
} // namespace SE
