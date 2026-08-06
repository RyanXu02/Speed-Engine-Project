#pragma once
#include "glm/glm.hpp"
#include "FBO.h"
#include "../Rendering/Renderer.h"

namespace SE
{
	struct CameraFrustum
	{
		glm::vec3 position = glm::vec3(0.0f, 20.0f, 50.0f);
		glm::vec3 target = glm::vec3(0.0f, 0.0f, 0.0f);
		glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

		float fov = 100.0f;
		float nearPlane = 0.1f;
		float farPlane = 1000.0f;
		// cache
		glm::mat4 viewMatrix = glm::mat4(1.0f);
		glm::mat4 projectionMatrix = glm::mat4(1.0f);
		glm::mat4 viewProjectionMatrix = glm::mat4(1.0f);
	};

	class Viewport
	{
    public:
        Viewport(uint32_t width, uint32_t height, Renderer* renderer)
            : m_FBO(std::make_unique<FBO>(width, height))
            , m_cameraFrustum()
            , m_isActive(true)
			, m_assignedRenderer(renderer)
        {}

        FBO& getFBO() { return *m_FBO; }
		CameraFrustum& getCameraFrustum() { return m_cameraFrustum; }
		bool isAssignedTo(Renderer* rndr) const {
			return rndr == m_assignedRenderer;
		}

    private:
		const Renderer* const m_assignedRenderer;
        std::unique_ptr<FBO> m_FBO;  // Viewport owns FBO
		//MIGHT NEED MULTIPLE FBOS IN THE FUTURE
		//ex: vector of fbos, with enum {SCENE, DEBUG, UI} and index into vector like
		//vec[SCENE] = scene fbo
		CameraFrustum m_cameraFrustum;
        bool m_isActive;
	};

};

