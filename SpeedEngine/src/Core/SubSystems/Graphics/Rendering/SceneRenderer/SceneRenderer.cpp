#include "pch.h"

#include "SceneRenderer.h"

#include "Core/ResourceManagers/ResourceManager.h"
#include "Core/ResourceManagers/Shader/Shader.h"

#include "Core/SubSystems/Graphics/Rendering/RenderContext.h"

namespace SE
{
	void SceneRenderer::init()
	{
		m_logger.info("Initializing");
	}
	void SceneRenderer::shutdown()
	{
		m_logger.info("Shutting down");
	}
	void SceneRenderer::render(RenderContext& context)
	{
		if (context.frame.draws.empty())
			return;

		const CameraFrustum& camera = context.viewport.getCameraFrustum();

		// Get default shader
		Shader* defaultShader = context.resources.getResource<Shader>(m_defaultShaderId);
		if (!defaultShader)
		{
			m_logger.critical("Default shader not found, cannot render");
			return;
		}
		// Get default material
		Material* defaultMaterial = context.resources.getResource<Material>(m_defaultMaterialId);

		// per frame uniforms
		defaultShader->bind();
		defaultShader->setMat4("projectionView", camera.viewProjectionMatrix);
		defaultShader->setVec3("camPos", camera.getPosition());
		_setLightUniforms(context, *defaultShader);

		for (const auto& draw : context.frame.draws)
		{
			uint32_t meshResourceId = draw.meshResourceId;
			if (meshResourceId == 0)
			{
				continue;
			}

			MeshResource* meshResource = context.resources.getResource<MeshResource>(meshResourceId);
			if (!meshResource)
			{
				m_logger.warn("Mesh resource with ID {} not found, skipping entity {}", meshResourceId, draw.entityId);
				continue;
			}

			defaultShader->setMat4("model", draw.modelMatrix);	// Set transform

			// each submesh can have different set of material so get material for each mesh
			const std::vector<uint32_t>& meshMaterials = _getMeshMaterials(context, meshResourceId, *meshResource);
			const std::vector<SubMesh>& subMeshes = meshResource->getSubMeshes();
			for (size_t i = 0; i < subMeshes.size(); ++i)
			{
				const int32_t materialIndex = subMeshes[i].materialIndex;
				const bool hasPBR = materialIndex >= 0 && static_cast<size_t>(materialIndex) < meshMaterials.size();
				const PBRMaterial* pbr = hasPBR ? &meshResource->getMaterials()[materialIndex] : nullptr;

				Material* material = hasPBR ? context.resources.getResource<Material>(meshMaterials[materialIndex]) : nullptr;
				if (!material) {
					material = defaultMaterial;
				}

				// Set material properties
				defaultShader->setVec4("albedoFactor", pbr ? pbr->albedoFactor : glm::vec4(0.8f, 0.8f, 0.8f, 1.0f));
				defaultShader->setVec3("emissiveFactor", pbr ? pbr->emissiveFactor : glm::vec3(0.0f));
				defaultShader->setFloat("roughnessFactor", pbr ? pbr->roughnessFactor : 1.0f);
				defaultShader->setFloat("metallicFactor", pbr ? pbr->metallicFactor : 0.0f);
				defaultShader->setFloat("aoStrength", pbr ? pbr->aoStrength : 1.0f);
				defaultShader->setFloat("normalScale", pbr ? pbr->normalScale : 1.0f);

				if (material) {
					material->bind();
				}

				context.meshes.draw(meshResourceId, *meshResource, i);

				if (material) {
					material->unbind();
				}
			}
		}

		defaultShader->unbind();
	}

	void SceneRenderer::_setLightUniforms(const RenderContext& context, Shader& shader)
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
		shader.setFloat("gamma", m_lightingSettings.gamma);
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

			std::vector<std::pair<TextureType, std::string_view>> textures;
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
				if (!path.empty()) {
					textures.emplace_back(type, path);
				}
			}

			uint32_t materialId = 0;
			if (!textures.empty())
			{
				std::string materialName = fmt::format("{}_{}_{}", meshResource.getResourceName(), i, pbr.name);
				materialId = context.resources.addResource<ResourceType::Material>(m_defaultShaderId, std::span<const std::pair<TextureType, std::string_view>>(textures), materialName);
			}
			it->second.push_back(materialId);
		}

		m_logger.info("Created {} materials for mesh resource {}", it->second.size(), meshResource.getResourceName());
		return it->second;
	}

} // namespace SE
