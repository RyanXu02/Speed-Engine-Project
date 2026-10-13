#include "pch.h"
#include "AudioSystem.h"
#include "Core/Logger/Logger.h"

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

namespace SE
{
	//pimpl idiom (pointer implementation)
	//allows for the big ass miniaudio.h to only need including in this file, all audio-related needs
	//are accessed via including AudioSystem.h.
	struct AudioSystem::AudioImpl
	{
		ma_engine engine{};
		ma_engine_config engineConfig{};
		bool initialized = false;
	};

	AudioSystem::AudioSystem()
	    : SubSystem("AudioSystem"),
	      m_aimpl(std::make_unique<AudioImpl>())
	{}

	AudioSystem::~AudioSystem() = default;

	void AudioSystem::init() {
		SubSystem::init();

		m_aimpl->engineConfig = ma_engine_config_init();
		//ANY OTHER ENGINE CONFIG THINGS THAT WANT TO BE CHANGED GO HERE
		//.....
		if (ma_engine_init(&m_aimpl->engineConfig, &m_aimpl->engine) != MA_SUCCESS) {
			m_logger->critical("AUDIO ENGINE FAILED TO INITIALIZE, ABORT ABORT");
		}
		m_aimpl->initialized = true;
	}

	void AudioSystem::update(double dt) {
		//empty for now, idk what will go here
	}

	void AudioSystem::shutdown() {
		SubSystem::shutdown();

		if (m_aimpl->initialized)
		{
			ma_engine_uninit(&m_aimpl->engine);
			m_aimpl->initialized = false;
		}
	}

	void AudioSystem::PlaySound(std::string_view path) {
		ma_engine_play_sound(&m_aimpl->engine, path.data(), nullptr);
	}

} //namespace SE
