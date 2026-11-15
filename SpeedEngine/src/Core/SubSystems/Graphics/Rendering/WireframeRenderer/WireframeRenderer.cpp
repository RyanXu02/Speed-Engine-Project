#include "pch.h"

#include "Core/Engine.h"
#include "Core/ResourceManagers/ResourceManager.h"
#include "Core/ResourceManagers/Shader/Shader.h"
#include "Core/SubSystems/Graphics/Rendering/RenderContext.h"
#include "WireframeRenderer.h"

namespace SE
{
	void WireframeRenderer::init()
	{
		m_logger.info("Initializing");

		auto* resources = Engine::Instance().getSubSystem<ResourceManager>();
		m_meshShaderId = resources->loadShader({ .vertexShaderPath = "Assets/Shaders/mesh_wireframe.vert", .fragmentShaderPath = "Assets/Shaders/mesh_wireframe.frag", .name = "meshWireframeShader" });
		m_terrainShaderId = resources->loadShader({ .vertexShaderPath = "Assets/Shaders/terrain_wireframe.vert", .fragmentShaderPath = "Assets/Shaders/terrain_wireframe.frag", .name = "terrainWireframeShader" });
		m_defaultTextureId = resources->loadTexture({ .path = "Assets/Textures/default-texture-128.png", .name = "defaultTexture" });
	}

	void WireframeRenderer::shutdown()
	{
		m_logger.info("Shutting down");
	}

	void WireframeRenderer::render(RenderContext& context)
	{
		if (context.frame.draws.empty())
			return;

		const CameraFrustum& camera = context.viewport.getCameraFrustum();

		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		// Get mesh shader
		Shader* meshShader = context.resources.getResource<Shader>(m_meshShaderId);
		if (!meshShader)
		{
			m_logger.critical("Mesh wireframe shader not found, cannot render");
			return;
		}

		// per frame uniforms
		meshShader->bind();
		meshShader->setMat4("projectionView", camera.viewProjectionMatrix);

		// Draw meshes
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

			meshShader->setMat4("model", interpolateTransform(draw.previous, draw.current, context.t)); // Set transform

			const std::vector<SubMesh>& subMeshes = meshResource->getSubMeshes();
			for (size_t i = 0; i < subMeshes.size(); ++i)
			{
				context.meshes.draw(meshResourceId, *meshResource, i);
			}
		}
		meshShader->unbind();


		// Get terrain shader
		Shader* terrainShader = context.resources.getResource<Shader>(m_terrainShaderId);
		if (!terrainShader)
		{
			m_logger.critical("Terrain wireframe shader not found, cannot render");
			return;
		}

		// per frame uniforms
		terrainShader->bind();
		terrainShader->setMat4("projectionView", camera.viewProjectionMatrix);

		// Draw terrain
		for (const auto& draw : context.frame.draws)
		{
			uint32_t terrainResourceId = draw.terrainResourceId;
			if (terrainResourceId == 0)
			{
				continue;
			}

			Terrain* terrain = context.resources.getResource<Terrain>(terrainResourceId);
			if (!terrain)
			{
				m_logger.warn("Terrain with ID {} not found, skipping entity {}", terrainResourceId, draw.entityId);
				continue;
			}

			terrainShader->setMat4("model", interpolateTransform(draw.previous, draw.current, context.t)); // Set transform

			// Load height map texture
			Texture* heightMapTexture = nullptr;
			if (const uint32_t heightMapTextureId = context.resources.loadTexture({ .path = terrain->getHeightMapPath(), .name = terrain->getHeightMapPath() }))
			{
				heightMapTexture = context.resources.getResource<Texture>(heightMapTextureId);
			}
			if (!heightMapTexture)
			{
				heightMapTexture = context.resources.getResource<Texture>(m_defaultTextureId);
			}

			const std::vector<SubMesh>& subMeshes = terrain->getSubMeshes();
			for (size_t i = 0; i < subMeshes.size(); ++i)
			{
				// Load terrain height map
				if (heightMapTexture)
				{
					glActiveTexture(GL_TEXTURE0 + 1);
					glBindTexture(GL_TEXTURE_2D, heightMapTexture->getTextureId());
					terrainShader->setInt("terrainHeightMap", 1);
				}

				// Set height scale
				terrainShader->setFloat("heightScale", terrain->getHeightScale());

				// Draw call
				context.meshes.draw(terrainResourceId, *terrain, i);

				if (heightMapTexture)
				{
					glActiveTexture(GL_TEXTURE0 + 1);
					glBindTexture(GL_TEXTURE_2D, 0);
				}
			}
		}

		terrainShader->unbind();

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
} // namespace SE
