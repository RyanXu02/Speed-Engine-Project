#pragma once
#include "../SubSystem.h"

struct GLFWwindow;

namespace SE
{

	class Window : public SubSystem
	{
	public:
		Window() : SubSystem("Window"), 
			m_windowTitle("Untitled"), m_vsyncEnabled(true),
			m_window(nullptr), m_windowShared(nullptr), 
			m_width(800), m_height(600), 
			m_lastFrameTime(0.0), m_deltaTime(0.0) {}
		Window(unsigned int width, unsigned int height, std::string windowTitle) : SubSystem("Window"), 
			m_windowTitle(windowTitle), m_vsyncEnabled(true),
			m_window(nullptr), m_windowShared(nullptr),
			m_width(width), m_height(height), 
			m_lastFrameTime(0.0), m_deltaTime(0.0) {}

		void init() override;
		void update(double deltaTime) override;
		void shutdown() override;

		GLFWwindow* getWindow() const { return m_window; }

	private:
		bool _initGLFW();

		bool m_vsyncEnabled;

		std::string m_windowTitle;
		
		GLFWwindow* m_window;
		GLFWwindow* m_windowShared;

		unsigned int m_width;
		unsigned int m_height;

		double m_lastFrameTime;
		double m_deltaTime;

	};
}

