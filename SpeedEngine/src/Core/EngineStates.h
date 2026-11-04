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

		enum class InputMode { Game, Editor };
		static InputMode getInputMode() { return s_inputMode.load(std::memory_order_relaxed); }
		static void setInputMode(InputMode mode) { s_inputMode.store(mode, std::memory_order_relaxed); }
		static bool isGameInputActive() { return getInputMode() == InputMode::Game; }

		static void play() { s_state = State::Play; }
		static void pause() { s_state = State::Pause; }

		static State getState() { return s_state; }
		static bool isPlaying() { return s_state == State::Play; }
		static bool isPaused() { return s_state == State::Pause; }

		static bool isDebugRenderingEnabled() { return s_debugRenderingEnabled.load(std::memory_order_relaxed); }
		static void setDebugRenderingEnabled(bool enabled) { s_debugRenderingEnabled.store(enabled, std::memory_order_relaxed); }
		
		static CameraFrustum* setCamera(CameraFrustum* camera) { return s_appCamera = camera ? camera : &s_editorCamera; }
		static CameraFrustum* getActiveCamera() { return isPlaying() ? s_appCamera : &s_editorCamera; }
		
		static bool isEditorCameraActive() { return getActiveCamera() == &s_editorCamera; }
		static CameraFrustum& getEditorCamera() { return s_editorCamera; }
		
		static void releaseCamera(const CameraFrustum* camera)
		{
			if (s_appCamera == camera)
				setCamera(nullptr);
		}

	private:
		static inline std::atomic<InputMode> s_inputMode{
#if defined(COMPILE_WITH_EDITOR)
			InputMode::Editor
#else
			InputMode::Game
#endif
		};
		static inline std::atomic<bool> s_debugRenderingEnabled{ false };

		// Simulation owns camera selection
		static inline CameraFrustum s_editorCamera;
		static inline State s_state{ State::Play };
		static inline CameraFrustum* s_appCamera{ &s_editorCamera };
	};
}