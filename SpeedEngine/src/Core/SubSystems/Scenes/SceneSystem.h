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

		// @brief Records a transition for the next simulation update; the first pending request wins.
		void requestNewScene(const std::string& name);
		void requestOpenScene(const std::string& path);
		void requestReloadScene();
		bool hasPendingTransition() const { return m_pendingTransition.has_value(); }
		// @brief Returns the current active scene, or nullptr if no scene is active
		Scene* getCurrentScene() const { return m_currentScene.get(); }
		// @brief Returns the current scene revision, which is incremented every time the scene contents are replaced.
		// @brief This can be used to detect when the scene has changed and to invalidate any cached data that depends on the scene contents.
		uint64_t getSceneRevision() const { return m_sceneRevision; }

	private:
		SceneSystem()
		    : SubSystem("SceneSystem")
		{}

		void processPendingTransition();
		void newScene(const std::string& name);
		void unloadScene(Scene* scene);
		void setCurrentScene(const std::string& path);

		// @brief Called when the contents of the scene have been replaced, either by loading a new scene or by reloading the current scene.
		// @param scene The scene whose contents have been replaced.
		void sceneContentsReplaced(const Scene* scene);

		enum class TransitionType
		{
			New,
			Open,
			Reload
		};

		struct TransitionRequest
		{
			TransitionType type;
			std::string value;
		};

		std::optional<TransitionRequest> m_pendingTransition;
		std::unique_ptr<Scene> m_currentScene;
		uint64_t m_sceneRevision = 0;
	};
} // namespace SE
