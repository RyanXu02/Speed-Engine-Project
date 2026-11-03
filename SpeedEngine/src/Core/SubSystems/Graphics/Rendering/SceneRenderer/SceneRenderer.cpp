#include "pch.h"

#include "SceneRenderer.h"

#include "Core/ResourceManagers/ResourceManager.h"
#include "Core/ResourceManagers/Shader/Shader.h"
#include "Core/SubSystems/Graphics/Rendering/RenderContext.h"
#include "Core/Engine.h"
#include <glad/glad.h>

namespace SE
{
	void SceneRenderer::init()
	{
		m_logger.info("Initializing");

		ResourceManager* resources = Engine::Instance().getSubSystem<ResourceManager>();
		m_defaultShaderId = resources->loadShader({ .vertexShaderPath = "Assets/Shaders/default.vert", .fragmentShaderPath = "Assets/Shaders/default.frag", .name = "defaultShader" });
		m_defaultMaterialId = resources->createMaterial({ .shaderId = m_defaultShaderId, .textures = {}, .name = "defaultMaterial" });
	}
	void SceneRenderer::shutdown()
	{
		m_logger.info("Shutting down");
	}
	void SceneRenderer::render(RenderContext& context)
	{
		// start from the input target, its depth lets transparents be occluded by the opaque geometry
		const FBO& input = context.getRenderTarget(m_inputTarget);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, input.getID());
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, context.output->getID());
		glBlitFramebuffer(0, 0, input.getWidth(), input.getHeight(),
			0, 0, context.output->getWidth(), context.output->getHeight(), GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);
		context.output->bind();

		if (context.frame.draws.empty()) {
			return;
		}

		const CameraFrustum& camera = context.viewport.getCameraFrustum();

		// collect transparent submeshes for drawing, those that are skipped by the deferredpass, opaque meshes are drawn by the deferred pass
		m_transparentItems.clear();
		for (const auto& draw : context.frame.draws)
		{
			MeshResource* meshResource = draw.meshResourceId ? context.resources.getResource<MeshResource>(draw.meshResourceId) : nullptr;
			if (!meshResource) {
				continue;
			}

			// NOTE: within the same model, all submeshes currently share the same position from transform matrix
			// this will likely cause issues with multiple overlapping transparent submeshes, or overlapping entities
			// demo mostly have one entity or one mesh with transparency only so this is probably fine fix until that's an issue
			const glm::vec3 cameraToObject = glm::vec3(interpolateTransform(draw.previous, draw.current, context.t)[3]) - camera.getPosition();
			const std::vector<PBRMaterial>& pbrMaterials = meshResource->getMaterials();
			const std::vector<SubMesh>& subMeshes = meshResource->getSubMeshes();
			for (size_t i = 0; i < subMeshes.size(); ++i) {
				const int32_t materialIndex = subMeshes[i].materialIndex;
				if (materialIndex >= 0 && static_cast<size_t>(materialIndex) < pbrMaterials.size() && pbrMaterials[materialIndex].albedoFactor.a < 1.0f) {
					m_transparentItems.push_back({ &draw, meshResource, i, glm::dot(cameraToObject, cameraToObject) });
				}
			}
		}

		if (m_transparentItems.empty()) {
			return;
		}

		// back to front so blending composites correctly, sorted per entity origin
		std::sort(m_transparentItems.begin(), m_transparentItems.end(),
			[](const TransparentItem& a, const TransparentItem& b) { return a.distanceSq > b.distanceSq; });

		// Get default shader
		Shader* defaultShader = context.resources.getResource<Shader>(m_defaultShaderId);
		if (!defaultShader)
		{
			m_logger.critical("Default shader not found, cannot render");
			return;
		}
		// Get default material
		Material* defaultMaterial = context.resources.getResource<Material>(m_defaultMaterialId);

		// test against the opaque depth copied by the deferred pass, readonly
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDepthMask(GL_FALSE);

		// per frame uniforms
		defaultShader->bind();
		defaultShader->setMat4("projectionView", camera.viewProjectionMatrix);
		defaultShader->setVec3("camPos", camera.getPosition());
		setLightUniforms(context, *defaultShader);

		for (const auto& item : m_transparentItems)
		{
			const uint32_t meshResourceId = item.draw->meshResourceId;
			const int32_t materialIndex = item.meshResource->getSubMeshes()[item.subMeshIndex].materialIndex;
			const PBRMaterial& pbr = item.meshResource->getMaterials()[materialIndex];

			const std::vector<uint32_t>& meshMaterials = _getMeshMaterials(context, meshResourceId, *item.meshResource);
			Material* material = context.resources.getResource<Material>(meshMaterials[materialIndex]);
			if (!material) {
				material = defaultMaterial;
			}

			// Set transform
			defaultShader->setMat4("model", interpolateTransform(item.draw->previous, item.draw->current, context.t));

			// Set material properties
			defaultShader->setVec4("albedoFactor", pbr.albedoFactor);
			defaultShader->setVec3("emissiveFactor", pbr.emissiveFactor);
			defaultShader->setFloat("roughnessFactor", pbr.roughnessFactor);
			defaultShader->setFloat("metallicFactor", pbr.metallicFactor);
			defaultShader->setFloat("aoStrength", pbr.aoStrength);
			defaultShader->setFloat("normalScale", pbr.normalScale);

			if (material) {
				material->bind();
			}

			context.meshes.draw(meshResourceId, *item.meshResource, item.subMeshIndex);

			if (material) {
				material->unbind();
			}
		}

		defaultShader->unbind();
	}

	void SceneRenderer::setLightUniforms(const RenderContext& context, Shader& shader)
	{
		const DirectionalLight& sun = context.sun;
		shader.setBool("useDirLight", sun.enabled);
		shader.setVec3("dirLightDirection", glm::normalize(sun.direction));
		shader.setVec3("dirLightColor", sun.color * sun.intensity);

		const auto& pointLights = context.frame.lights;
		const size_t numLights = std::min(pointLights.size(), MAX_LIGHTS);
		shader.setInt("numLights", static_cast<int>(numLights));
		for (size_t i = 0; i < numLights; ++i)
		{
			shader.setVec3(fmt::format("lightPositions[{}]", i), pointLights[i].position);
			shader.setVec3(fmt::format("lightColors[{}]", i), pointLights[i].radiance);
		}

		shader.setFloat("ambientStrength", m_lightingSettings.ambientStrength);
	}

	const std::vector<uint32_t>& SceneRenderer::_getMeshMaterials(RenderContext& context, uint32_t meshResourceId, const MeshResource& meshResource)
	{
		auto [it, inserted] = m_meshMaterials.try_emplace(meshResourceId);
		if (!inserted) {
			return it->second;
		}

		const std::vector<PBRMaterial>& pbrMaterials = meshResource.getMaterials();
		it->second.reserve(pbrMaterials.size());
		for (size_t i = 0; i < pbrMaterials.size(); ++i)
		{
			const PBRMaterial& pbr = pbrMaterials[i];

			std::vector<TextureBinding> textures;
			const std::pair<TextureType, const std::string&> candidates[] = {
				{ TextureType::Albedo, pbr.albedoPath },
				{ TextureType::Normal, pbr.normalPath },
				{ TextureType::AmbOcc, pbr.aoPath },
				{ TextureType::Emissive, pbr.emissivePath },
				{ TextureType::Roughness, pbr.roughnessPath },
				{ TextureType::Metallic, pbr.metallicPath },
			};
			for (const auto& [type, path] : candidates)
			{
				if (path.empty()) {
					continue;
				}
				if (const uint32_t textureId = context.resources.loadTexture({ .path = path, .name = path })) {
					textures.push_back({ .usage = type, .textureId = textureId });
				}
			}

			// only transparent materials, opaque ones are owned by the deferred pass
			uint32_t materialId = 0;
			if (!textures.empty() && pbr.albedoFactor.a < 1.0f)
			{
				std::string materialName = fmt::format("{}_{}_{}", meshResource.getResourceName(), i, pbr.name);
				materialId = context.resources.createMaterial({ .shaderId = m_defaultShaderId, .textures = textures, .name = materialName });
			}
			it->second.push_back(materialId);
		}

		m_logger.info("Created {} materials for mesh resource {}", it->second.size(), meshResource.getResourceName());
		return it->second;
	}

} // namespace SE
