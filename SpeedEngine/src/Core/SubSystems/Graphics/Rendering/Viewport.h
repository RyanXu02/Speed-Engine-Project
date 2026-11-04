#pragma once
#include "glm/glm.hpp"
#include "Core/Features/CameraFrustum.h"
#include <cstdint>

namespace SE
{
	// @brief Represents a viewport in the rendering system
	class Viewport
	{
	public:
		Viewport(uint32_t width, uint32_t height);

		void setCamera(const CameraState& camera);
		void resize(uint32_t width, uint32_t height);
		void updateMatrices();

		uint32_t getWidth() const { return m_width; }
		uint32_t getHeight() const { return m_height; }

		const CameraFrustum& getCameraFrustum() const { return m_cameraFrustum; }

		void setActive(bool active) { m_isActive = active; }
		bool isActive() const { return m_isActive; }

	private:
		uint32_t m_width;
		uint32_t m_height;
		CameraFrustum m_cameraFrustum;
		bool m_isActive;
	};

}; // namespace SE
