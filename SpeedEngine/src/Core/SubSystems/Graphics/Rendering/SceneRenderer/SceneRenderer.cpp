#include "pch.h"
#include "Engine.h"

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

#include "../../../Input/InputSystem.h"



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
		FBO& fbo = m_viewport->getFBO();
		CameraFrustum& camera = m_viewport->getCameraFrustum();
		_updateCamera(deltaTime, camera, fbo);
	}

	void SceneRenderer::render() const
	{
		// get viewport stuff
		FBO& fbo = m_viewport->getFBO();
		CameraFrustum& camera = m_viewport->getCameraFrustum();

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
		Shader* defaultShader = Engine::Instance().getSubSystem<ResourceManager>()->getResource<Shader>(m_defaultShaderId);
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

			const std::vector<Vertex>& positions = mesh->getPositionsToDraw();
			const std::vector<SubMesh>& subMeshes = mesh->getSubMeshesToDraw();
			const std::vector<uint32_t>& indices = mesh->getIndicesToDraw();
			if (indices.empty() || positions.empty()) {
				continue;
			}

			auto it = m_meshCache.find(meshResourceId);
			if (it == m_meshCache.end() || it->second.needsUpdate) {
				uploadMeshToGPU(meshResourceId, positions, subMeshes, indices);
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
		for (const auto& entity : Engine::Instance().getSubSystem<SceneSystem>()->getCurrentScene()->getEntities()) {
			if (entity.get()->getComponent<Mesh>()) {
				retlist.push_back(entity.get());
			}
		}
		return retlist;
	}

	void SceneRenderer::uploadMeshToGPU(
		uint32_t meshId,
		const std::vector<Vertex>& positions,
		const std::vector<SubMesh>& subMeshes,
		const std::vector<uint32_t>& indices) const
	{
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
			positions.size() * sizeof(Vertex),
			positions.data(),
			GL_STATIC_DRAW);

		// Upload index data
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gpuData.EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER,
			indices.size() * sizeof(uint32_t),
			indices.data(),
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
		gpuData.indexCount = indices.size();
		gpuData.needsUpdate = false;

		m_logger.info("Uploaded mesh {} to GPU: {} vertices, {} indices",
			meshId, positions.size(), indices.size());
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

	void SceneRenderer::_updateCameraPosition(double deltaTime, CameraFrustum& camera) {
		auto IS = Engine::Instance().getSubSystem<InputSystem>();

		// x-y plane movement
		float movespeed = m_cameraMoveSpeed * static_cast<float>(deltaTime);
		glm::vec3 cameraLookDir = glm::normalize(camera.getTarget() - camera.getPosition());
		cameraLookDir.y = 0.0f;
		glm::vec3 cameraRight = glm::normalize(glm::cross(cameraLookDir, worldUp));
		glm::vec3 CameraMoveDir(0.0f);
		if (IS->isKeyDown(KeyCodes::KEY_W))
			CameraMoveDir += cameraLookDir;
		if (IS->isKeyDown(KeyCodes::KEY_S))
			CameraMoveDir += -cameraLookDir;
		if (IS->isKeyDown(KeyCodes::KEY_A))
			CameraMoveDir += -cameraRight;
		if (IS->isKeyDown(KeyCodes::KEY_D))
			CameraMoveDir += cameraRight;

		if (glm::length(CameraMoveDir) > 0.0f) {
			CameraMoveDir = glm::normalize(CameraMoveDir);
			camera.move(CameraMoveDir * movespeed);
		}

		// camera dolly movement (to and from target)
		float dollySpeed = m_cameraDollySpeed * static_cast<float>(deltaTime);
		glm::vec3 lookDir = glm::normalize(camera.getTarget() - camera.getPosition());
		if (IS->getMouseScroll().yoffset > 0.0)
		{
			camera.move(lookDir * dollySpeed);
		}
		if (IS->getMouseScroll().yoffset < 0.0)
		{
			camera.move(-lookDir * dollySpeed);
		}

		// camera y-axis movement
		if (IS->isKeyDown(KeyCodes::KEY_SPACE))
			camera.move(worldUp * movespeed);
		if (IS->isKeyDown(KeyCodes::KEY_LEFT_CONTROL))
			camera.move(-worldUp * movespeed);
	}

	void SceneRenderer::_updateCameraTarget(double deltaTime, CameraFrustum& camera) {
		auto IS = Engine::Instance().getSubSystem<InputSystem>();

		if (IS->isMouseDown(MouseCodes::MOUSE_BUTTON_3))
		{
			glm::vec2 mouseDelta = glm::vec2{ IS->getMouseDelta().x, IS->getMouseDelta().y };

			glm::vec3 cameraDirection = glm::normalize(camera.getTarget() - camera.getPosition());
			glm::vec3 cameraRight = glm::normalize(glm::cross(cameraDirection, worldUp));
			glm::vec3 cameraUp = glm::normalize(glm::cross(cameraRight, cameraDirection));

			float yawAngle = -mouseDelta.x * m_sensitivity;
			glm::mat4 yawRotation = glm::rotate(glm::mat4(1.0f), yawAngle, cameraUp);
			cameraDirection = glm::vec3(yawRotation * glm::vec4(cameraDirection, 0.0f));
			float pitchAngle = -mouseDelta.y * m_sensitivity;
			cameraDirection = glm::vec3(glm::rotate(glm::mat4(1.0f), pitchAngle, cameraRight) * glm::vec4(cameraDirection, 0.0f));


			float distance = glm::length(camera.getTarget() - camera.getPosition());
			glm::vec3 cameraTarget = camera.getPosition() + cameraDirection * distance;
			camera.setTarget(cameraTarget);
		}
	}

	void SceneRenderer::_updateCamera(double deltaTime, CameraFrustum& camera, FBO& fbo) {
		auto IS = Engine::Instance().getSubSystem<InputSystem>();
		if (IS->isKeyRising(KeyCodes::KEY_LEFT_SHIFT)) {
			m_cameraMoveSpeed *= 2.0f;
			m_cameraDollySpeed *= 2.0f;
		}
		else if (IS->isKeyFalling(KeyCodes::KEY_LEFT_SHIFT))
		{
			m_cameraMoveSpeed /= 2.0f;
			m_cameraDollySpeed /= 2.0f;
		}
		_updateCameraPosition(deltaTime, camera);
		_updateCameraTarget(deltaTime, camera);
		if (camera.isDirty) {
			camera.viewMatrix = glm::lookAt(camera.getPosition(), camera.getTarget(), worldUp); // 0,1,0 is up vector
			float aspectRatio = static_cast<float>(fbo.getWidth()) / static_cast<float>(fbo.getHeight());
			camera.projectionMatrix = glm::perspective(glm::radians(camera.getFOV()), aspectRatio, camera.getNearPlane(), camera.getFarPlane());
			camera.viewProjectionMatrix = camera.projectionMatrix * camera.viewMatrix;
			camera.isDirty = false;
		}
	}
}
