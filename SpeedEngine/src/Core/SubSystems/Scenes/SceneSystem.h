#pragma once

#include "Scene/Scene.h"
#include "SubSystems/SubSystem.h"

namespace SE
{
	// @brief Owns and manages the active scene
	class SceneSystem : public SubSystem
	{
		friend class Engine;
		friend class Scene;

	public:
		void init() override;
		void update(double deltaTime) override;
		void shutdown() override;

		// @brief Creates a new scene with the given name and sets it as the current scene
		// @param name The name of the new scene.
		void newScene(const std::string& name);
		// @brief Unloads the given scene if it is the current scene
		// @param scene The scene to unload.
		void unloadScene(Scene* scene);

		// @brief Loads a scene from a path and sets it as the current scene
		// @param path The path to the scene file.
		void setCurrentScene(const std::string& path);
		// @brief Returns the current active scene, or nullptr if no scene is active
		Scene* getCurrentScene() const { return m_currentScene.get(); }
		// @brief Returns the current scene revision, which is incremented every time the scene contents are replaced.
		// @brief This can be used to detect when the scene has changed and to invalidate any cached data that depends on the scene contents.
		uint64_t getSceneRevision() const { return m_sceneRevision; }

	private:
		SceneSystem()
		    : SubSystem("SceneSystem")
		{}

		// @brief Called when the contents of the scene have been replaced, either by loading a new scene or by reloading the current scene.
		// @param scene The scene whose contents have been replaced.
		void sceneContentsReplaced(const Scene* scene);

		std::unique_ptr<Scene> m_currentScene;
		uint64_t m_sceneRevision = 0;
	};
} // namespace SE
