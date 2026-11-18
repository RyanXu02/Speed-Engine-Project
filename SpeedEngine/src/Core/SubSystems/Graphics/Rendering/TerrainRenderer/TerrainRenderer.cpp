#include "pch.h"

#include "TerrainRenderer.h"
#include "Core/Engine.h"
#include "Core/ResourceManagers/ResourceManager.h"
#include "Core/ResourceManagers/Shader/Shader.h"
#include "Core/SubSystems/Graphics/Rendering/RenderContext.h"

namespace SE
{
	void TerrainRenderer::init()
	{
		m_logger.info("Initializing");

		auto* resources = Engine::Instance().getSubSystem<ResourceManager>();
		m_defaultShaderId = resources->requestShader({ "Assets/Shaders/terrain_gpass.vert", "Assets/Shaders/terrain_gpass.frag", "terrainGBufferShader" });
		m_defaultMaterialId = resources->createMaterial({ .shaderId = m_defaultShaderId, .textures = {}, .name = "defaultGBufferMaterial" });
		m_defaultTextureId = resources->requestTexture({ "Assets/Textures/default-texture-128.png", "defaultTexture" });
	}

	void TerrainRenderer::shutdown()
	{
		m_logger.info("Shutting down");
	}

	void TerrainRenderer::render(RenderContext& context)
	{
		if (context.frame.draws.empty())
			return;

		const CameraFrustum& camera = context.viewport.getCameraFrustum();

		// Get default shader
		Shader* defaultShader = context.gpu.shader(context.resources, m_defaultShaderId, m_logger);
		if (!defaultShader)
			return;
		// Get default material
		const auto* defaultMaterial = context.resources.getResource<Material>(m_defaultMaterialId);

		// per frame uniforms
		defaultShader->bind();
		defaultShader->setMat4("projectionView", camera.viewProjectionMatrix);

		for (const auto& draw : context.frame.draws)
		{
			uint32_t terrainResourceId = draw.terrainResourceId;
			if (terrainResourceId == 0)
			{
				continue;
			}

			const auto* terrain = context.resources.getResource<Terrain>(terrainResourceId);
			if (!terrain)
				continue;

			defaultShader->setMat4("model", interpolateTransform(draw.previous, draw.current, context.t));	// Set transform

			// Load splat map texture
			const auto textureOrDefault = [&](const std::string& path)
			{
				const auto id = path.empty() ? 0 : context.resources.requestTexture({ path, path });
				const auto texture = context.gpu.texture(context.resources, id);
				return texture ? texture : context.gpu.texture(context.resources, m_defaultTextureId);
			};
			const auto splatMapTexture = textureOrDefault(terrain->getSplatMapPath());
			const auto heightMapTexture = textureOrDefault(terrain->getHeightMapPath());
			if (!splatMapTexture || !heightMapTexture)
				continue;

			const std::vector<uint32_t>& terrainMaterials = _getTerrainMaterials(context, terrainResourceId, *terrain);
			const std::vector<SubMesh>& subMeshes = terrain->getSubMeshes();
			for (size_t i = 0; i < subMeshes.size(); ++i)
			{
				// If terrain doesn't have the 3 required pbr textures, don't render
				if (terrain->getMaterials().size() < 3)
					continue;

				// Load the 3 PBR Materials
				for (size_t j = 0; j < 3; ++j)
				{
					const PBRMaterial* pbr = &terrain->getMaterials()[j];

					const auto* material = context.resources.getResource<Material>(terrainMaterials[j]);
					if (!material)
					{
						material = defaultMaterial;
					}

					// Set material properties
					std::string albedoFactor = "albedoFactor" + std::to_string(j);
					std::string emissiveFactor = "emissiveFactor" + std::to_string(j);
					std::string roughnessFactor = "roughnessFactor" + std::to_string(j);
					std::string metallicFactor = "metallicFactor" + std::to_string(j);
					std::string aoStrength = "aoStrength" + std::to_string(j);
					std::string normalScale = "albedoFactor" + std::to_string(j);
					defaultShader->setVec4(albedoFactor, pbr ? pbr->albedoFactor : glm::vec4(0.8f, 0.8f, 0.8f, 1.0f));
					defaultShader->setVec3(emissiveFactor, pbr ? pbr->emissiveFactor : glm::vec3(0.0f));
					defaultShader->setFloat(roughnessFactor, pbr ? pbr->roughnessFactor : 1.0f);
					defaultShader->setFloat(metallicFactor, pbr ? pbr->metallicFactor : 0.0f);
					defaultShader->setFloat(aoStrength, pbr ? pbr->aoStrength : 1.0f);
					defaultShader->setFloat(normalScale, pbr ? pbr->normalScale : 1.0f);

					if (material)
					{
						context.gpu.bindMaterial(context.resources, *material, *defaultShader, j);
					}
				}

				// Load terrain specific textures
				if (splatMapTexture) {
					_bindTexture(splatMapTexture, *defaultShader, m_splatMapSlot, "terrainSplatMap");
				}
				if (heightMapTexture)
				{
					_bindTexture(heightMapTexture, *defaultShader, m_heightMapSlot, "terrainHeightMap");
				}

				// Set height scale
				defaultShader->setFloat("heightScale", terrain->getHeightScale());

				// Draw call
				context.meshes.draw(terrainResourceId, *terrain, i);

				// Unbind 3 PBR materials
				for (size_t j = 0; j < 3; ++j)
					context.gpu.unbindMaterial(j);

				// Unbind terrain textures
				if (splatMapTexture)
				{
					_unbindTexture(m_splatMapSlot);
				}
				if (heightMapTexture)
				{
					_unbindTexture(m_heightMapSlot);
				}

			}
		}

		defaultShader->unbind();
	}

	const std::vector<uint32_t>& TerrainRenderer::_getTerrainMaterials(RenderContext& context, uint32_t terrainResourceId, const Terrain& terrain)
	{
		auto [it, inserted] = m_terrainMaterials.try_emplace(terrainResourceId);
		if (!inserted) {
			return it->second;
		}

		const std::vector<PBRMaterial>& pbrMaterials = terrain.getMaterials();
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
				if (const auto textureId = context.resources.requestTexture({ path, path })) {
					textures.push_back({ .usage = type, .textureId = textureId });
				}
			}

			// only opaque materials, transparent ones are owned by the SceneRenderer
			uint32_t materialId = 0;
			if (!textures.empty() && pbr.albedoFactor.a >= 1.0f)
			{
				std::string materialName = fmt::format("{}_{}_{}", terrainResourceId, i, pbr.name);
				materialId = context.resources.createMaterial({ .shaderId = m_defaultShaderId, .textures = textures, .name = materialName });
			}
			it->second.push_back(materialId);
		}

		m_logger.info("Created {} materials for terrain resource {}", it->second.size(), terrainResourceId);
		return it->second;
	}

	void TerrainRenderer::_bindTexture(unsigned int texture, Shader& shader, const uint32_t slot, std::string_view name)
	{
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_2D, texture);
		shader.setInt(name, slot);
	}

	void TerrainRenderer::_unbindTexture(const uint32_t slot)
	{
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

} // namespace SE
