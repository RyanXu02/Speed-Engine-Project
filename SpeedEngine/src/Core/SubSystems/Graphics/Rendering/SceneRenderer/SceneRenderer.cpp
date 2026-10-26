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
		if (!defaultMaterial)
		{
			m_logger.critical("Default material not found, cannot render");
			return;
		}

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

			defaultShader->bind();
			defaultMaterial->bind();

			// Set camera matrices
			defaultShader->setMat4("projectionView", camera.viewProjectionMatrix);

			// Set transform
			defaultShader->setMat4("model", draw.modelMatrix);

			// Set material properties
			defaultShader->setVec4("color", glm::vec4(0.8f, 0.8f, 0.8f, 1.0f));
			defaultShader->setBool("useTexture", defaultMaterial != nullptr);
			defaultShader->setFloat("intensity", 1.0f);
			defaultShader->setInt("frameIndex", 0);
			defaultShader->setFloat("numRows", 1.0f);
			defaultShader->setFloat("numCols", 1.0f);

			context.meshes.draw(meshResourceId, *meshResource);

			defaultMaterial->unbind();
			defaultShader->unbind();
		}
	}

} // namespace SE
