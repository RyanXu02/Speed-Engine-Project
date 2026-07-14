#pragma once
#include "glm/glm.hpp"

namespace SE
{
	class Window;

	struct Viewport
	{
		int x = 0;
		int y = 0;
		int width = 1280;
		int height = 720;

		float aspectRatio() const
		{
			return static_cast<float>(width) / static_cast<float>(height);
		}
	};

	struct CameraFrustum
	{
		glm::vec3 position = glm::vec3(0.0f, 0.0f, 5.0f);
		glm::vec3 target = glm::vec3(0.0f, 0.0f, 0.0f);
		glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

		float fov = 45.0f;
		float nearPlane = 0.1f;
		float farPlane = 100.0f;
		// cache
		glm::mat4 viewMatrix = glm::mat4(1.0f);
		glm::mat4 projectionMatrix = glm::mat4(1.0f);
		glm::mat4 viewProjectionMatrix = glm::mat4(1.0f);
	};

	struct Stats
	{
		uint32_t trianglesDrawn = 0;
		uint32_t drawCalls = 0;
		uint32_t batchesSubmitted = 0;
		double frameTimeMs = 0.0;
	};

	class RenderContext
	{
	public:
		RenderContext();
		~RenderContext();

		void setWindow(const Window* window) { m_window = window; }
		const Window* getWindow() const { return m_window; }

		void setViewport(const Viewport& viewport) { m_viewport = viewport; }
		const Viewport& getViewport() const { return m_viewport; }

		void setCameraFrustum(const CameraFrustum& cameraFrustum);
		const CameraFrustum& getCameraFrustum() const { return m_cameraFrustum; }
		CameraFrustum& getCameraFrustum() { return m_cameraFrustum; }

		glm::mat4 getViewMatrix() const { return m_cameraFrustum.viewMatrix; }
		glm::mat4 getProjectionMatrix() const { return m_cameraFrustum.projectionMatrix; }
		glm::mat4 getViewProjectionMatrix() const { return m_cameraFrustum.viewProjectionMatrix; }

		void setBackgroundColor(const glm::vec4& color) { m_backgroundColor = color; }
		const glm::vec4& getBackgroundColor() const { return m_backgroundColor; }

		void setWireFrameMode(bool enabled) { m_wireFrameMode = enabled; }
		const bool isWireFrameMode() const { return m_wireFrameMode; }

		void resetStats();
		const Stats& getStats() const { return m_stats; }

		void recordDrawCall(uint32_t triangles);
		void recordBatchSubmission();

	private:
		const Window* m_window;

		Viewport m_viewport;
		CameraFrustum m_cameraFrustum;

		glm::vec4 m_backgroundColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

		bool m_wireFrameMode = false;
		
		Stats m_stats;
	};
}

