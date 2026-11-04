#include "pch.h"

#include "ShadowRenderer.h"
#include "Core/Engine.h"
#include "Core/ResourceManagers/ResourceManager.h"
#include "Core/ResourceManagers/Shader/Shader.h"
#include "Core/SubSystems/Graphics/Rendering/RenderContext.h"
#include "Core/SubSystems/Graphics/Rendering/RendererManager.h"

#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>

namespace SE
{
	void ShadowRenderer::init()
	{
		m_logger.info("Initializing");

		auto* resources = Engine::Instance().getSubSystem<ResourceManager>();
		m_shadowShaderId = resources->loadShader({ .vertexShaderPath = "Assets/Shaders/shadow.vert", .fragmentShaderPath = "Assets/Shaders/shadow.frag", .name = "shadowShader" });
	}

	void ShadowRenderer::render(RenderContext& context)
	{
		// first pass of the frame, so the lighting pass sees the same sun this frame
		_updateSun();
		m_lightSpaceMatrix = _computeLightSpaceMatrix(context);

		if (!context.sun.enabled || context.frame.draws.empty())
			return;

		Shader* shadowShader = context.resources.getResource<Shader>(m_shadowShaderId);
		if (!shadowShader)
		{
			m_logger.critical("Shadow shader not found, cannot render");
			return;
		}

		// push depth back a little so lit surfaces don't shadow themselves (acne)
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(2.0f, 4.0f);

		shadowShader->bind();
		shadowShader->setMat4("lightSpaceMatrix", m_lightSpaceMatrix);

		for (const auto& draw : context.frame.draws)
		{
			if (draw.meshResourceId == 0)
				continue;

			MeshResource* meshResource = context.resources.getResource<MeshResource>(draw.meshResourceId);
			if (!meshResource)
				continue;

			shadowShader->setMat4("model", interpolateTransform(draw.previous, draw.current, context.t));

			const std::vector<PBRMaterial>& materials = meshResource->getMaterials();
			const std::vector<SubMesh>& subMeshes = meshResource->getSubMeshes();
			for (size_t i = 0; i < subMeshes.size(); ++i)
			{
				// transparent submeshes don't cast shadows, same split as the GeometryRenderer
				const int32_t materialIndex = subMeshes[i].materialIndex;
				if (materialIndex >= 0 && static_cast<size_t>(materialIndex) < materials.size() && materials[materialIndex].albedoFactor.a < 1.0f)
					continue;

				context.meshes.draw(draw.meshResourceId, *meshResource, i);
			}
		}

		shadowShader->unbind();
		glDisable(GL_POLYGON_OFFSET_FILL);
	}

	void ShadowRenderer::shutdown()
	{
		m_logger.info("Shutting down");
	}

	void ShadowRenderer::_updateSun()
	{
		const auto now = std::chrono::steady_clock::now();
		// clamp so a long hitch or the first frame doesn't jump the sun
		const float dt = m_lastRenderTime == std::chrono::steady_clock::time_point{}
			? 0.0f
			: std::min(std::chrono::duration<float>(now - m_lastRenderTime).count(), 0.1f);
		m_lastRenderTime = now;

		DirectionalLight& sun = Engine::Instance().getSubSystem<RendererManager>()->getSun();

		// start from whatever direction the sun was given so the first frame doesn't snap
		if (!m_sunAnglesInitialized) {
			const glm::vec3 toSun = -glm::normalize(sun.direction);
			sunElevation = glm::degrees(std::asin(glm::clamp(toSun.y, -1.0f, 1.0f)));
			sunAzimuth = glm::degrees(std::atan2(toSun.x, toSun.z));
			m_sunAnglesInitialized = true;
		}

		const float minElevation = std::min(sunMinElevation, sunMaxElevation);
		const float maxElevation = std::max(sunMinElevation, sunMaxElevation);
		if (animateSun) {
			// pick the phase that matches the current elevation so turning it on doesn't jump
			if (!m_wasAnimating) {
				const float range = std::max(maxElevation - minElevation, 0.001f);
				const float t = glm::clamp((sunElevation - minElevation) / range, 0.0f, 1.0f);
				m_elevationPhase = glm::degrees(std::asin(t * 2.0f - 1.0f));
			}

			sunAzimuth = std::remainder(sunAzimuth + sunRotateSpeed * dt, 360.0f);
			m_elevationPhase = std::fmod(m_elevationPhase + sunElevationSpeed * dt, 360.0f);
			const float t = 0.5f + 0.5f * std::sin(glm::radians(m_elevationPhase));
			sunElevation = glm::mix(minElevation, maxElevation, t);
		}
		m_wasAnimating = animateSun;

		const float azimuth = glm::radians(sunAzimuth);
		const float elevation = glm::radians(sunElevation);
		const glm::vec3 toSun{ std::cos(elevation) * std::sin(azimuth), std::sin(elevation), std::cos(elevation) * std::cos(azimuth) };
		sun.direction = -toSun; // stored as the way the light travels
	}

	glm::mat4 ShadowRenderer::_computeLightSpaceMatrix(const RenderContext& context) const
	{
		const glm::vec3 lightDir = glm::normalize(context.sun.direction);
		const glm::vec3 center = context.viewport.getCameraFrustum().getPosition();

		// lookAt breaks when the light points straight up or down
		const glm::vec3 up = std::abs(lightDir.y) > 0.99f ? glm::vec3(0.0f, 0.0f, 1.0f) : glm::vec3(0.0f, 1.0f, 0.0f);
		const glm::mat4 lightView = glm::lookAt(center - lightDir * lightDistance, center, up);
		const glm::mat4 lightProjection = glm::ortho(-orthoHalfSize, orthoHalfSize, -orthoHalfSize, orthoHalfSize, 0.1f, lightDistance * 2.0f);

		return lightProjection * lightView;
	}
} // namespace SE
