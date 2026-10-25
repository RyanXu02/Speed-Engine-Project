#pragma once
#include "glm/glm.hpp"
#include "Core/Features/CameraFrustum.h"

namespace SE
{
	// @brief Definition of world-up in the engine
	static constexpr glm::vec3 WORLD_UP = glm::vec3(0.0f, 1.0f, 0.0f);

	// @brief Represents a viewport in the rendering system
	class Viewport
	{
	public:
		Viewport(uint32_t width, uint32_t height);

		void resize(uint32_t width, uint32_t height);
		void updateMatrices();

		uint32_t getWidth() const { return m_width; }
		uint32_t getHeight() const { return m_height; }

		CameraFrustum& getCameraFrustum() { return *m_cameraFrustum; }

		void setActive(bool active) { m_isActive = active; }
		bool isActive() const { return m_isActive; }

	private:
		uint32_t m_width;
		uint32_t m_height;
		CameraFrustum* m_cameraFrustum;
		bool m_isActive;
	};

}; // namespace SE
