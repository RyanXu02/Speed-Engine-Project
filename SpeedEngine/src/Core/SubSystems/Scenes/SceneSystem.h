#pragma once

#include "../SubSystem.h"
#include "../../Scene/Scene.h"

namespace SE
{
	class SceneSystem : public SubSystem
	{
	public:
		SceneSystem() : SubSystem("SceneSystem") {}
		~SceneSystem() override = default;
		void init() override;
		void update(double deltaTime) override;
		void shutdown() override;

		void addScene(std::unique_ptr<Scene> scene);
		void removeScene(Scene* scene);

		void setCurrentScene(std::unique_ptr<Scene> scene);
		Scene* getCurrentScene() const { return m_currentScene; }
	private:
		std::vector<std::unique_ptr<Scene>> m_scenes;
		Scene* m_currentScene = nullptr;
	};
}

