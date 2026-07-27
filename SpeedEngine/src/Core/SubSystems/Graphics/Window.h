#pragma once
#include "../SubSystem.h"

struct GLFWwindow;

namespace SE
{

	class Window : public SubSystem
	{
	public:
		Window() : SubSystem("Window"), 
			m_windowTitle("Untitled"),
			m_window(nullptr), m_windowShared(nullptr), 
			m_windowWidth(800), m_windowHeight(600) {}
		Window(unsigned int width, unsigned int height, std::string windowTitle) : SubSystem("Window"), 
			m_windowTitle(windowTitle),
			m_window(nullptr), m_windowShared(nullptr),
			m_windowWidth(width), m_windowHeight(height) {}

		void init() override;
		void update(double deltaTime) override;
		void shutdown() override;

		void swapBuffers() const;

		GLFWwindow* getWindow() const { return m_window; }

		unsigned int getWidth() const { return m_windowWidth; }
		unsigned int getHeight() const { return m_windowHeight; }

		double getFPS() const { return m_fps; }
		void setFPS(double fps) { m_fps = fps; }

		double getCurrentTime() const;

	private:
		bool _initGLFW();

		bool m_vsyncEnabled { true };

		std::string m_windowTitle;
		
		GLFWwindow* m_window;
		GLFWwindow* m_windowShared;

		unsigned int m_windowWidth;
		unsigned int m_windowHeight;

		double m_fps { 60.0 };
	};
}

