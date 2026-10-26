#pragma once

#include "Core/Features/CameraFrustum.h"

namespace SE
{
	class EngineStates
	{
	public:
		enum class State
		{
			Play,
			Pause
		};

		static void play() { s_state = State::Play; }
		static void pause() { s_state = State::Pause; }

		static State getState() { return s_state; }
		static bool isPlaying() { return s_state == State::Play; }
		static bool isPaused() { return s_state == State::Pause; }
		static CameraFrustum* setCamera(CameraFrustum* camera) { return s_appCamera = camera ? camera : &s_editorCamera; }
		static CameraFrustum* getActiveCamera() { return isPlaying() ? s_appCamera : &s_editorCamera; }
		static bool isEditorCameraActive() { return getActiveCamera() == &s_editorCamera; }

	private:
		// engine has their own default editor camera, this can be reassigned to an in game authored scriptcamera
		// the renderer context do not own the engine camera, it only reference it
		static inline CameraFrustum s_editorCamera;
		static inline State s_state{ State::Play };
		static inline CameraFrustum* s_appCamera{ &s_editorCamera };
	};
}