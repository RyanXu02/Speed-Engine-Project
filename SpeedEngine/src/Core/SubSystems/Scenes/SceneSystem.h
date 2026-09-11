#pragma once

#include "../SubSystem.h"
#include "../../Scene/Scene.h"

namespace SE
{
	// @brief Manages all scenes in the engine
	class SceneSystem : public SubSystem
	{
		friend class Engine;
	public:

		void init() override;
		void update(double deltaTime) override;
		void shutdown() override;

		void addScene(std::unique_ptr<Scene> scene);
		void newScene(const std::string& name);
		void unloadScene(Scene* scene);

		void setCurrentScene(const std::string& name);
		Scene* getCurrentScene() const { return m_currentScene; }
	private:
		SceneSystem() : SubSystem("SceneSystem") {}

		std::vector<std::unique_ptr<Scene>> m_scenes;
		Scene* m_currentScene = nullptr;
	};
}

