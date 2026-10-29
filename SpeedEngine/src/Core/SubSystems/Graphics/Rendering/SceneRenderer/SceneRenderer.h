#pragma once
#include "../Renderer.h"
#include "Core/Features/CameraFrustum.h"

namespace SE
{
	struct CameraFrustum;
	class MeshResource;
	class Scene;
	class Shader;

	// Note: shaders must match the max lights value defined here
	constexpr size_t MAX_LIGHTS = 16;

	struct LightingSettings
	{
		float ambientStrength = 0.03f;
		float gamma = 2.2f;
	};

	// @brief SceneRenderer is responsible for rendering all drawable entities in the scene
	class SceneRenderer : public Renderer
	{
	public:
		explicit SceneRenderer(Logger& logger) : Renderer(logger, "SceneRenderer") {}

		void init() override;
		void shutdown() override;
		void update(double deltaTime, RenderContext& context) override;

		void render(RenderContext& context) override;
		void setDefaultShaderId(uint32_t id) { m_defaultShaderId = id; }
		void setDefaultMaterialId(uint32_t id) { m_defaultMaterialId = id; }

		// @brief get scene preview camera sensitivity
		// @return reference to camera sensitivity, used for imgui widgets
		float& getCameraSensitivity() { return m_sensitivity; }
		// @brief get scene preview camera move speed
		// @return reference to camera move speed, used for imgui widgets
		float& getCameraMoveSpeed() { return m_cameraMoveSpeed; }
		// @brief get scene preview camera dolly speed
		// @return reference to camera dolly speed, used for imgui widgets
		float& getCameraDollySpeed() { return m_cameraDollySpeed; }

		LightingSettings& getLightingSettings() { return m_lightingSettings; }

	private:
		void _setLightUniforms(const RenderContext& context, Shader& shader);
		LightingSettings m_lightingSettings;

		// Default shader for meshes without material
		uint32_t m_defaultShaderId = 0;
		uint32_t m_defaultMaterialId = 0;

		std::unordered_map<uint32_t, std::vector<uint32_t>> m_meshMaterials;
		const std::vector<uint32_t>& _getMeshMaterials(RenderContext& context, uint32_t meshResourceId, const MeshResource& meshResource);

		// Camera controls
		void _updateCameraPosition(double deltaTime, CameraFrustum& camera);
		void _updateCameraTarget(double deltaTime, CameraFrustum& camera);
		void _updateCamera(double deltaTime, CameraFrustum& camera);
		float m_sensitivity = 0.005f;
		float m_cameraMoveSpeed = 6.0f;
		float m_cameraDollySpeed = 10.0f;
	};
} // namespace SE
