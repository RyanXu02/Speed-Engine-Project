#pragma once
#include "Core/SubSystems/SubSystem.h"
#include "Core/SubSystems/Input/InputEventQueue.h"

struct GLFWwindow;

namespace SE
{
	// @brief Represents the window subsystem of the engine
	class Window : public SubSystem
	{
	public:
		Window()
		    : SubSystem("Window"),
		      m_windowTitle("Untitled"),
		      m_window(nullptr),
		      m_windowShared(nullptr),
		      m_windowWidth(800),
		      m_windowHeight(600)
		{}
		Window(unsigned int width, unsigned int height, std::string windowTitle)
		    : SubSystem("Window"),
		      m_windowTitle(windowTitle),
		      m_window(nullptr),
		      m_windowShared(nullptr),
		      m_windowWidth(width),
		      m_windowHeight(height)
		{}

		void init() override;
		void update(double deltaTime) override;
		void shutdown() override;

		void swapBuffers() const;

		bool shouldClose() const;

		void waitForEvents(double timeout) const;
		InputEventQueue& inputEvents() { return m_inputEvents; }

		GLFWwindow* getWindow() const { return m_window; }

		unsigned int getWidth() const { return m_windowWidth; }
		unsigned int getHeight() const { return m_windowHeight; }

		void setSize(int width, int height);

		double getCurrentTime() const;

	private:
		bool _initGLFW();

		bool m_vsyncEnabled{ false };
		InputEventQueue m_inputEvents; // Because Window runs on main thread

		std::string m_windowTitle;

		GLFWwindow* m_window;
		GLFWwindow* m_windowShared;

		unsigned int m_windowWidth;
		unsigned int m_windowHeight;
	};
} // namespace SE
