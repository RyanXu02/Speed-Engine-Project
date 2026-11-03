#pragma once
#include "Core/SubSystems/Graphics/Rendering/Renderer.h"

#include <glm/glm.hpp>

namespace SE
{
	// @brief Directional shadow pass, draws the opaque submeshes depth only from the sun's point of view
	class ShadowRenderer : public Renderer
	{
	public:
		explicit ShadowRenderer(Logger& logger)
		    : Renderer(logger, "ShadowRenderer")
		{}

		void init() override;
		void render(RenderContext& context) override;
		void shutdown() override;

		// @brief World space to light clip space, valid after this frame's shadow pass
		const glm::mat4& getLightSpaceMatrix() const { return m_lightSpaceMatrix; }

		float orthoHalfSize = 30.0f;	// half size of the ortho box around the camera, bigger covers more but blurs the shadows
		float lightDistance = 50.0f;	// how far back from the camera the light sits, also the depth range is twice this
		float depthBias = 0.002f;		// depth bias at grazing angles, fights shadow acne
		float normalOffset = 0.05f;		// world units to push the lookup along the normal, also fights acne

		// sun position in degrees, written to the sun's direction every frame
		// azimuth is around world up starting at +Z, elevation is height above the horizon
		float sunAzimuth = 0.0f;
		float sunElevation = 45.0f;

		bool animateSun = false;			// spins the azimuth and swings the elevation between min and max
		float sunRotateSpeed = 20.0f;		// azimuth degrees per second
		float sunElevationSpeed = 15.0f;	// degrees per second through the up and down cycle, 360 is one full swing
		float sunMinElevation = 5.0f;
		float sunMaxElevation = 75.0f;

	private:
		glm::mat4 _computeLightSpaceMatrix(const RenderContext& context) const;
		void _updateSun();

		uint32_t m_shadowShaderId = 0;
		std::chrono::steady_clock::time_point m_lastRenderTime{};
		bool m_sunAnglesInitialized = false;
		bool m_wasAnimating = false;
		float m_elevationPhase = 0.0f; // degrees, elevation follows sin of this
		glm::mat4 m_lightSpaceMatrix{ 1.0f };
	};
} // namespace SE
