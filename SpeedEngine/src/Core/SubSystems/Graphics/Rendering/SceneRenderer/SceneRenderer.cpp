#include "pch.h"
#include "SceneRenderer.h"
#include <glad/gl.h>
#include <glm/gtc/matrix_transform.hpp>

#include "../RendererManager.h"



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
		glClearColor(1.f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// enable depth test
		glEnable(GL_DEPTH_TEST);

		// render scene here
		/*
		for (auto& entity : scene.getRenderables())
		{
			auto* mesh = entity.getMesh();
			auto* material = entity.getMaterial();
			auto& transform = entity.getTransform();

			if (!mesh || !material) continue;

			// Bind shader
			material->getShader()->bind();

			// Set uniforms
			material->getShader()->setMat4("u_ViewProjection", camera.viewProjectionMatrix);
			material->getShader()->setMat4("u_Model", transform.getMatrix());

			// Bind material textures
			material->bind();

			// Draw mesh
			mesh->draw();
		}
		*/

		// unbind fbo
		fbo.unbind();
	}
}
