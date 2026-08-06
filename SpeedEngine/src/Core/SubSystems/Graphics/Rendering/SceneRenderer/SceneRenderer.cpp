#include "pch.h"
#include "SceneRenderer.h"
#include "../../../../Scene/Entity/Entity.h"
#include "../../../../Scene/Entity/Component/Mesh.h"
#include "../../../../Scene/Entity/Component/Transform.h"

#include <glad/gl.h>
#include <glm/gtc/matrix_transform.hpp>

#include "../RendererManager.h"
#include "../../../Scenes/SceneSystem.h"
#include "../../../../ResourceManagers/ResourceManager.h"
#include "../../../../ResourceManagers/Shader/Shader.h"
#include "../../../../ResourceManagers/Material/Material.h"



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
	void SceneRenderer::update(double deltaTime)
	{
	}

	void SceneRenderer::render(Viewport& viewport) const
	{
		// get viewport stuff
		FBO& fbo = viewport.getFBO();
		CameraFrustum& camera = viewport.getCameraFrustum();

		// update camera matrices
		camera.viewMatrix = glm::lookAt(camera.position, camera.target, camera.up);
		float aspectRatio = static_cast<float>(fbo.getWidth()) / static_cast<float>(fbo.getHeight());
		camera.projectionMatrix = glm::perspective(glm::radians(camera.fov), aspectRatio, camera.nearPlane, camera.farPlane);
		camera.viewProjectionMatrix = camera.projectionMatrix * camera.viewMatrix;

		//bind fbo
		fbo.bind();

		//clear framebuffer
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// enable depth test
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);

		// Enable backface culling
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glFrontFace(GL_CCW);

		// Get default shader
		Shader* defaultShader = ResourceManager::Instance().getResource<Shader>(m_defaultShaderId);
		if (!defaultShader) {
			m_logger.critical("Default shader not found, cannot render");
			fbo.unbind();
			return;
		}

		for (auto& entity : getDrawableEntities()) {
			Mesh* mesh = entity->getComponent<Mesh>();
			Transform* transform = entity->getComponent<Transform>();

			uint32_t meshResourceId = mesh->getMeshResourceId();
			if (meshResourceId == 0) continue;

			DrawData drawData = mesh->getDrawData();
			if (drawData.indices.empty() || drawData.positions.empty()) {
				continue;
			}

			auto it = m_meshCache.find(meshResourceId);
			if (it == m_meshCache.end() || it->second.needsUpdate) {
				uploadMeshToGPU(meshResourceId, drawData);
			}

			const MeshGPUData& gpuData = m_meshCache[meshResourceId];
			if (gpuData.VAO == 0) continue;

			defaultShader->bind();

			// Set camera matrices
			defaultShader->setMat4("projectionView", camera.viewProjectionMatrix);

			// Set model matrix (use transform if available, otherwise identity)
			glm::mat4 modelMatrix = glm::mat4(1.0f);
			if (transform) {
				modelMatrix = transform->getTransform();
			}
			defaultShader->setMat4("model", modelMatrix);

			// Set default material properties
			defaultShader->setVec4("color", glm::vec4(0.8f, 0.8f, 0.8f, 1.0f));
			defaultShader->setBool("useTexture", false);
			defaultShader->setFloat("intensity", 1.0f);
			defaultShader->setInt("frameIndex", 0);
			defaultShader->setFloat("numRows", 1.0f);
			defaultShader->setFloat("numCols", 1.0f);

			// Bind VAO and draw
			glBindVertexArray(gpuData.VAO);
			glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(gpuData.indexCount), GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);

			defaultShader->unbind();
		}

		// unbind fbo
		fbo.unbind();
	}

	std::vector<Entity*> SceneRenderer::getDrawableEntities() const {
		std::vector<Entity*> retlist;
		for (const auto& entity : SceneSystem::Instance().getCurrentScene()->getEntities()) {
			if (entity.get()->getComponent<Mesh>()) {
				retlist.push_back(entity.get());
			}
		}
		return retlist;
	}

	void SceneRenderer::uploadMeshToGPU(uint32_t meshId, const DrawData& drawData) const {
		MeshGPUData& gpuData = m_meshCache[meshId];

		// Delete old buffers if they exist
		if (gpuData.VAO != 0) {
			cleanupMeshGPU(meshId);
		}

		// Generate buffers
		glGenVertexArrays(1, &gpuData.VAO);
		glGenBuffers(1, &gpuData.VBO);
		glGenBuffers(1, &gpuData.EBO);

		// Bind VAO
		glBindVertexArray(gpuData.VAO);

		// Upload vertex data
		glBindBuffer(GL_ARRAY_BUFFER, gpuData.VBO);
		glBufferData(GL_ARRAY_BUFFER,
			drawData.positions.size() * sizeof(Vertex),
			drawData.positions.data(),
			GL_STATIC_DRAW);

		// Upload index data
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gpuData.EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER,
			drawData.indices.size() * sizeof(uint32_t),
			drawData.indices.data(),
			GL_STATIC_DRAW);

		// Set vertex attributes
		// Position attribute (location = 0)
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));

		// Normal attribute (location = 2, matching shader)
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

		// UV attribute (location = 1, matching shader)
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));

		// Unbind
		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		// Update cache
		gpuData.indexCount = drawData.indices.size();
		gpuData.needsUpdate = false;

		m_logger.info("Uploaded mesh {} to GPU: {} vertices, {} indices",
			meshId, drawData.positions.size(), drawData.indices.size());
	}

	void SceneRenderer::cleanupMeshGPU(uint32_t meshId) const {
		auto it = m_meshCache.find(meshId);
		if (it == m_meshCache.end()) return;

		MeshGPUData& gpuData = it->second;

		if (gpuData.EBO != 0) {
			glDeleteBuffers(1, &gpuData.EBO);
			gpuData.EBO = 0;
		}
		if (gpuData.VBO != 0) {
			glDeleteBuffers(1, &gpuData.VBO);
			gpuData.VBO = 0;
		}
		if (gpuData.VAO != 0) {
			glDeleteVertexArrays(1, &gpuData.VAO);
			gpuData.VAO = 0;
		}

		m_logger.info("Cleaned up GPU data for mesh {}", meshId);
	}
}
