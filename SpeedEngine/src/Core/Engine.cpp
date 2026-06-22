#include "pch.h"
#include "Engine.h"

namespace SE
{
	Engine::Engine()
		: m_isRunning(false), m_deltaTime(0.0)
	{
	}
	Engine::~Engine()
	{
	}
	void Engine::init()
	{
		
	}
	void Engine::start()
	{
		m_isRunning = true;
	}
	void Engine::run()
	{
		while (m_isRunning)
		{
			printf("Engine is running...\n");
			// Update delta time
			m_deltaTime = 0.016; // Placeholder for actual delta time calculation
			// Handle input
			// Update game logic
			// Render
		}
	}
	void Engine::stop()
	{
		m_isRunning = false;
	}
}