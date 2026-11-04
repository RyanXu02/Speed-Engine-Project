#include "pch.h"

#include "Window.h"

#include "Core/Logger/Logger.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

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

	bool Window::shouldClose() const
	{
		return glfwWindowShouldClose(m_window) != 0;
	}

	void Window::waitForEvents(double timeout) const
	{
		glfwWaitEventsTimeout(timeout);
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
		if (!monitor)
		{
			m_logger->critical("Failed to get primary monitor");
			glfwTerminate();
			return false;
		}

		// Get monitor video mode
		const GLFWvidmode* mode = glfwGetVideoMode(monitor);
		if (!mode)
		{
			m_logger->critical("Failed to get video mode");
			glfwTerminate();
			return false;
		}

		glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
		glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
		// create window
		m_window = glfwCreateWindow(m_windowWidth, m_windowHeight, m_windowTitle.c_str(), NULL, NULL);
		if (!m_window)
		{
			m_logger->critical("Failed to create GLFW window");
			glfwTerminate();
			return false;
		}

		glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
		m_windowShared = glfwCreateWindow(m_windowWidth, m_windowHeight, m_windowTitle.c_str(), NULL, m_window);
		if (!m_windowShared)
		{
			m_logger->critical("Failed to create GLFW shared window");
			glfwTerminate();
			return false;
		}

		glfwMakeContextCurrent(m_window);
		glfwSetWindowUserPointer(m_window, &m_inputEvents);

		glfwSetWindowCloseCallback(m_window,
		    [](GLFWwindow* window)
		    {
				static_cast<InputEventQueue*>(glfwGetWindowUserPointer(window))->push({ InputEvent::Type::WindowClose });
		    });

		glfwSetCursorPosCallback(m_window,
		    [](GLFWwindow* window, double x, double y)
		    {
				static_cast<InputEventQueue*>(glfwGetWindowUserPointer(window))->push(
					{ InputEvent::Type::CursorPosition, 0, InputEvent::Action::Release, x, y });
		    });

		glfwSetKeyCallback(m_window,
		    [](GLFWwindow* window, int key, int scancode, int action, int mods)
		    {
				static_cast<InputEventQueue*>(glfwGetWindowUserPointer(window))->push(
					{ InputEvent::Type::Key, key, static_cast<InputEvent::Action>(action) });
			});

		glfwSetMouseButtonCallback(m_window,
			[](GLFWwindow* window, int button, int action, int mods)
			{
				static_cast<InputEventQueue*>(glfwGetWindowUserPointer(window))->push(
					{ InputEvent::Type::MouseButton, button, static_cast<InputEvent::Action>(action) });
			});

		glfwSetScrollCallback(m_window,
			[](GLFWwindow* window, double x, double y)
			{
				static_cast<InputEventQueue*>(glfwGetWindowUserPointer(window))->push(
					{ InputEvent::Type::Scroll, 0, InputEvent::Action::Release, x, y });
			});

		glfwSetWindowFocusCallback(m_window,
			[](GLFWwindow* window, int focused)
			{
				static_cast<InputEventQueue*>(glfwGetWindowUserPointer(window))->push(
					{ InputEvent::Type::Focus, 0, focused ? InputEvent::Action::Press : InputEvent::Action::Release });
		    });

		double x = 0.0, y = 0.0;
		glfwGetCursorPos(m_window, &x, &y);
		m_inputEvents.push({ InputEvent::Type::CursorPosition, 0, InputEvent::Action::Release, x, y });

		// init glad
		if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
		{
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

	void Window::setSize(int width, int height)
	{
		glfwSetWindowSize(m_window, width, height);
	}
} // namespace SE
