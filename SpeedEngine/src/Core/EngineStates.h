#pragma once
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

private:
	static State s_state;
};
