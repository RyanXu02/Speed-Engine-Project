#pragma once
#include "glm/glm.hpp"
#include "FBO.h"
#include "../Rendering/Renderer.h"

namespace SE
{
	// @brief Represents the camera frustum for a viewport
	struct CameraFrustum
	{
	private:
		glm::vec3 position = glm::vec3(0.0f, 2.0f, 5.0f);
		glm::vec3 target = glm::vec3(0.0f, 0.0f, 0.0f);
		glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

		float fov = 90.0f;
		float nearPlane = 0.1f;
		float farPlane = 1000.0f;
	public:
		void move(const glm::vec3& delta) { position += delta; target += delta; isDirty = true; }

		const glm::vec3& getPosition() const { return position; }
		void setPosition(const glm::vec3& pos) { position = pos; isDirty = true; }
		const glm::vec3& getTarget() const { return target; }
		void setTarget(const glm::vec3& tgt) { target = tgt; isDirty = true; }
		const glm::vec3& getUp() const { return up; }
		void setUp(const glm::vec3& u) { up = u; isDirty = true; }

		const float getFOV() const { return fov; }
		void setFOV(float f) { fov = f; isDirty = true; }
		const float getNearPlane() const { return nearPlane; }
		void setNearPlane(float n) { nearPlane = n; isDirty = true; }
		const  float getFarPlane() const { return farPlane; }
		void setFarPlane(float f) { farPlane = f; isDirty = true; }
		// cache
		bool isDirty = true;
		glm::mat4 viewMatrix = glm::mat4(1.0f);
		glm::mat4 projectionMatrix = glm::mat4(1.0f);
		glm::mat4 viewProjectionMatrix = glm::mat4(1.0f);
	};

	// @brief Represents a viewport in the rendering system
	class Viewport
	{
    public:
        Viewport(uint32_t width, uint32_t height)
            : m_FBO(std::make_unique<FBO>(width, height))
            , m_cameraFrustum()
            , m_isActive(true)
        {}

        FBO& getFBO() { return *m_FBO; }

		CameraFrustum& getCameraFrustum() { return m_cameraFrustum; }
		void setCameraFrustum(const CameraFrustum& frustum) { m_cameraFrustum = frustum; }
		
		void setActive(bool active) { m_isActive = active; }
		bool isActive() const { return m_isActive; }

    private:
        std::unique_ptr<FBO> m_FBO;  // Viewport owns FBO
		//MIGHT NEED MULTIPLE FBOS IN THE FUTURE
		//ex: vector of fbos, with enum {SCENE, DEBUG, UI} and index into vector like
		//vec[SCENE] = scene fbo

		CameraFrustum m_cameraFrustum;
        bool m_isActive;
	};

};

