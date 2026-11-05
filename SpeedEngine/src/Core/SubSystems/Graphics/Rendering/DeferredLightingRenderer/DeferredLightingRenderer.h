#pragma once
#include "Core/ResourceManagers/Shader/Shader.h"
#include "Core/SubSystems/Graphics/Rendering/Renderer.h"

namespace SE
{
	// @brief Deferred lighting pass, shades the G-buffer into the output and copies the G-buffer depth over
	// so passes reading the output depth test against the opaque geometry
	class DeferredLightingRenderer : public Renderer
	{
	public:
		// @param gBufferTarget the name of the G-buffer render target written by the GeometryRenderer
		// @param shadowMapTarget the name of the depth only render target written by the ShadowRenderer
		DeferredLightingRenderer(Logger& logger, std::string gBufferTarget, std::string shadowMapTarget);

		void init() override;
		void render(RenderContext& context) override;
		void shutdown() override;

		float phaseG = 0.7;
		float scatteringScale = 1.0f;
		float fogMaxDistance = 50.0f;	// march length for sky pixels and cap for far geometry

	private:
		std::string m_gBufferTarget;
		std::string m_shadowMapTarget;
		Shader m_shader;
		unsigned int m_VAO = 0;
		uint32_t m_blueNoiseTextureId = 0; // resource id, tiled over the screen to rotate the PCSS samples
	};
} // namespace SE
