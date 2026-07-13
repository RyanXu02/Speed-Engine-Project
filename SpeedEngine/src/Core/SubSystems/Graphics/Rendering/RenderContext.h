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

	struct Camera
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
		RenderContext(const Window* window);
		~RenderContext();

		void setWindow(const Window* window) { m_window = window; }
		const Window* getWindow() const { return m_window; }

		void setViewport(const Viewport& viewport) { m_viewport = viewport; }
		const Viewport& getViewport() const { return m_viewport; }

		void setCamera(const Camera& camera);
		const Camera& getCamera() const { return m_camera; }
		Camera& getCamera() { return m_camera; }

		glm::mat4 getViewMatrix() const { return m_camera.viewMatrix; }
		glm::mat4 getProjectionMatrix() const { return m_camera.projectionMatrix; }
		glm::mat4 getViewProjectionMatrix() const { return m_camera.viewProjectionMatrix; }

		void setBackgroundColor(const glm::vec4& color) { m_backgroundColor = color; }
		const glm::vec4& getBackgroundColor() const { return m_backgroundColor; }

		void setDeltaTime(double deltaTime) { m_deltaTime = deltaTime; }
		const double getDeltaTime() const { return m_deltaTime; }

		void setFrameIndex(uint64_t frameIndex) { m_frameIndex = frameIndex; }
		const uint64_t getFrameIndex() const { return m_frameIndex; }

		void setElapsedTime(double elapsedTime) { m_elapsedTime = elapsedTime; }
		const double getElapsedTime() const { return m_elapsedTime; }

		void setWireFrameMode(bool enabled) { m_wireFrameMode = enabled; }
		const bool isWireFrameMode() const { return m_wireFrameMode; }

		void resetStats();
		const Stats& getStats() const { return m_stats; }

		void recordDrawCall(uint32_t triangles);
		void recordBatchSubmission();

	private:
		const Window* m_window;

		Viewport m_viewport;
		Camera m_camera;

		glm::vec4 m_backgroundColor;

		double m_deltaTime = 0.0;
		uint64_t m_frameIndex = 0;
		double m_elapsedTime = 0.0;

		bool m_wireFrameMode = false;
		
		Stats m_stats;
	};
}

