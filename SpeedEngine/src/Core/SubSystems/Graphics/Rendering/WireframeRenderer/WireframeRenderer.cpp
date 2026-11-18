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
		m_meshShaderId = resources->requestShader({ "Assets/Shaders/mesh_wireframe.vert", "Assets/Shaders/mesh_wireframe.frag", "meshWireframeShader" });
		m_terrainShaderId = resources->requestShader({ "Assets/Shaders/terrain_wireframe.vert", "Assets/Shaders/terrain_wireframe.frag", "terrainWireframeShader" });
		m_defaultTextureId = resources->requestTexture({ "Assets/Textures/default-texture-128.png", "defaultTexture" });
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

		Shader* meshShader = context.gpu.shader(context.resources, m_meshShaderId, m_logger);
		Shader* terrainShader = context.gpu.shader(context.resources, m_terrainShaderId, m_logger);
		if (!meshShader || !terrainShader)
			return;
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

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

			const auto* meshResource = context.resources.getResource<MeshResource>(meshResourceId);
			if (!meshResource)
			{
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

			const auto* terrain = context.resources.getResource<Terrain>(terrainResourceId);
			if (!terrain)
				continue;

			terrainShader->setMat4("model", interpolateTransform(draw.previous, draw.current, context.t)); // Set transform

			// Load height map texture
			const auto& path = terrain->getHeightMapPath();
			const auto id = path.empty() ? 0 : context.resources.requestTexture({ path, path });
			auto heightMapTexture = context.gpu.texture(context.resources, id);
			if (!heightMapTexture)
				heightMapTexture = context.gpu.texture(context.resources, m_defaultTextureId);
			if (!heightMapTexture)
				continue;

			const std::vector<SubMesh>& subMeshes = terrain->getSubMeshes();
			for (size_t i = 0; i < subMeshes.size(); ++i)
			{
				// Load terrain height map
				if (heightMapTexture)
				{
					glActiveTexture(GL_TEXTURE0 + 1);
					glBindTexture(GL_TEXTURE_2D, heightMapTexture);
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
