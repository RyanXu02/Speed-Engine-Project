#pragma once

#include "../SubSystem.h"
#include "../../Scene/Scene.h"

namespace SE
{
	class SceneSystem : public SubSystem
	{
	public:
		SceneSystem() : SubSystem("SceneSystem") {}

		static SceneSystem& Instance()
		{
			if (s_instance)
			{
				return *s_instance;
			}

			static SceneSystem instance;
			s_instance = &instance;
			return *s_instance;
		}

		void init() override;
		void update(double deltaTime) override;
		void shutdown() override;

		void addScene(std::unique_ptr<Scene> scene);
		void newScene(const std::string& name);
		void unloadScene(Scene* scene);

		void setCurrentScene(const std::string& name);
		Scene* getCurrentScene() const { return m_currentScene; }
	private:
		static SceneSystem* s_instance;

		std::vector<std::unique_ptr<Scene>> m_scenes;
		Scene* m_currentScene = nullptr;
	};
}

