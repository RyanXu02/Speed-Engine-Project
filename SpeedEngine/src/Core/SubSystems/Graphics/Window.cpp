#include "pch.h"
#include "Window.h"
#include "../../Logger/Logger.h"

#include "glad/gl.h"
#include "GLFW/glfw3.h"

#include "../Events/EventSystem.h"

namespace SE
{
	void Window::init()
	{
		SubSystem::init();

		if (!_initGLFW())
		{
			m_logger->critical("Failed to initialize GLFW.");
			assert(false);
		}

		glfwSwapInterval(m_vsyncEnabled ? 1 : 0);
	}

	void Window::update(double deltaTime)
	{
		SubSystem::update(deltaTime);

		glfwPollEvents();
	}

	void Window::shutdown()
	{
		SubSystem::shutdown();

		glfwMakeContextCurrent(nullptr);
		glfwDestroyWindow(m_window);
		glfwDestroyWindow(m_windowShared);
		glfwTerminate();
	}

	void Window::swapBuffers() const
	{
		glfwSwapBuffers(m_window);
	}

	bool Window::_initGLFW()
	{
		// Set GLFW window hints for OpenGL version and profile
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		if (!glfwInit())
			return false;

		// Get primary monitor
		GLFWmonitor* monitor = glfwGetPrimaryMonitor();
		if (!monitor) {
			m_logger->critical("Failed to get primary monitor");
			glfwTerminate();
			return false;
		}

		// Get monitor video mode
		const GLFWvidmode* mode = glfwGetVideoMode(monitor);
		if (!mode) {
			m_logger->critical("Failed to get video mode");
			glfwTerminate();
			return false;
		}


		glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
		// create window
		m_window = glfwCreateWindow(m_windowWidth, m_windowHeight, m_windowTitle.c_str(), NULL, NULL);
		if (!m_window) {
			m_logger->critical("Failed to create GLFW window");
			glfwTerminate();
			return false;
		}
		glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
		m_windowShared = glfwCreateWindow(m_windowWidth, m_windowHeight, m_windowTitle.c_str(), NULL, m_window);
		if (!m_windowShared) {
			m_logger->critical("Failed to create GLFW shared window");
			glfwTerminate();
			return false;
		}

		glfwMakeContextCurrent(m_window);

		glfwSetWindowCloseCallback(m_window, [](GLFWwindow* window) {
			EventSystem::Instance().publish(std::make_unique<WindowClose>());
			});

		// init glad
		if (!gladLoadGL(glfwGetProcAddress)) {
			m_logger->critical("Failed to initialize GLAD for main context");
			glfwDestroyWindow(m_window);
			glfwDestroyWindow(m_windowShared);
			glfwTerminate();
			return false;
		}

		return true;
	}

	double Window::getCurrentTime() const
	{
		return glfwGetTime();
	}
}