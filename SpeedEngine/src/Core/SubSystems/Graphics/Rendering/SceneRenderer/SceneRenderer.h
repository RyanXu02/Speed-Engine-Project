#pragma once
#include "../Renderer.h"

namespace SE
{
	struct MeshResource;
	class Shader;
	struct DrawItem;

	// Note: shaders must match the max lights value defined here
	constexpr size_t MAX_LIGHTS = 16;

	struct LightingSettings
	{
		float ambientStrength = 0.03f;
	};

	// @brief Forward pass, copies the input target's color and depth then draws the transparent submeshes sorted back to front
	class SceneRenderer : public Renderer
	{
	public:
		// @param inputTarget the name of the render target to draw on top of, usually the deferred output
		SceneRenderer(Logger& logger, std::string inputTarget) : Renderer(logger, "SceneRenderer"), m_inputTarget(std::move(inputTarget)) {}

		void init() override;
		void shutdown() override;

		void render(RenderContext& context) override;

		LightingSettings& getLightingSettings() { return m_lightingSettings; }

		//NOTE: shared with the deferred pass, not the most ideal for scale but good enough for now
		void setLightUniforms(const RenderContext& context, Shader& shader);

	private:
		std::string m_inputTarget;
		LightingSettings m_lightingSettings;

		// Default shader for meshes without material
		uint32_t m_defaultShaderId = 0;
		uint32_t m_defaultMaterialId = 0;

		std::unordered_map<uint32_t, std::vector<uint32_t>> m_meshMaterials;
		const std::vector<uint32_t>& _getMeshMaterials(RenderContext& context, uint32_t meshResourceId, const MeshResource& meshResource);

		struct TransparentItem
		{
			const DrawItem* draw;
			const MeshResource* meshResource;
			size_t subMeshIndex;
			float distanceSq; // distance from the camera, for sorting
		};
		std::vector<TransparentItem> m_transparentItems;
	};
} // namespace SE
