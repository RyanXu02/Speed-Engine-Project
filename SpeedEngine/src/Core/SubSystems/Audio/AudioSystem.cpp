#include "pch.h"
#include "AudioSystem.h"
#include "Core/Logger/Logger.h"

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

/* TYPE EQUIVALENTS FOR STD CPP <-> MINIAUDIO
*  uint32_t = ma_uint32
*  
*/

namespace SE
{
	//needed for use of ma_sound with std::unique_ptr
	struct ma_sound_Deleter
	{
		void operator()(ma_sound* sound) const
		{
			if (sound != nullptr)
			{
				ma_sound_uninit(sound);
				delete sound;
			}
		}
	};

	//typedefs
	//since ma_sound is a struct, need to provide custom deleter so memory doesnt leak everywhere
	typedef std::unique_ptr<ma_sound, ma_sound_Deleter> UniquePtrSound;
	//"template" library of sounds. these sounds will never get played. copies will be made with generational handle setup
	//to actually play sfx sounds
	typedef std::vector<UniquePtrSound> SfxLibrary;

	//pimpl idiom (pointer implementation)
	//allows for the big ass miniaudio.h to only need including in this file, all audio-related needs
	//are accessed via including AudioSystem.h.
	struct AudioSystem::AudioImpl
	{
		ma_engine engine{};
		ma_sound_group musicGrp{}, sfxGrp{}, sfxNonGameGrp{}, sfxGameGrp{};
		SfxLibrary sfxLibrary;
		std::vector<std::string> musicLibrary;
		bool initialized = false;
		std::vector<UniquePtrSound> test;
	};

	struct AudioSystem::SoundSlot 
	{
		UniquePtrSound sound;
		uint32_t generation;
		bool inUse;
	};

	

	AudioSystem::AudioSystem()
	    : SubSystem("AudioSystem"),
	      m_imp(std::make_unique<AudioImpl>()),
	      isMuted(false)
	{}

	AudioSystem::~AudioSystem() = default;

	void AudioSystem::init() {
		SubSystem::init();

		ma_engine_config engineConfig = ma_engine_config_init();
		engineConfig.channels = 0; //use system default
		engineConfig.sampleRate = 0; //use system default
		//ANY OTHER ENGINE CONFIG THINGS THAT WANT TO BE CHANGED GO HERE
		//.....
		if (ma_engine_init(&engineConfig, &m_imp->engine) != MA_SUCCESS) {
			m_logger->critical("AUDIO ENGINE FAILED TO INITIALIZE, ABORT ABORT");
			m_imp->initialized = false;
			return;
		}
		//init music group
		if (ma_sound_group_init(&m_imp->engine, 0, nullptr, &m_imp->musicGrp) != MA_SUCCESS)
		{
			m_logger->critical("MUSIC GROUP FAILED TO INITIALIZE, ABORT ABORT");
			m_imp->initialized = false;
			return;
		}
		//init sfx groups
		if (!_initSfxGroups())
		{
			m_imp->initialized = false;
			return;
		}

		m_imp->initialized = true;
	}

	void AudioSystem::update(double dt) {
		//empty for now, idk what will go here
	}

	void AudioSystem::shutdown() {
		SubSystem::shutdown();

		//cleanup sounds(playing then library), then groups(children before parent), then engine in that order
		if (m_imp->initialized)
		{
			//cleanup ma_sounds here. MUST USE vector.clear() OR ELSE DOUBLE FREE WILL HAPPEN ON ENGINE SHUTDOWN
			m_imp->sfxLibrary.clear();
			m_imp->test.clear();
			ma_sound_group_uninit(&m_imp->musicGrp);
			ma_sound_group_uninit(&m_imp->sfxNonGameGrp);
			ma_sound_group_uninit(&m_imp->sfxGameGrp);
			ma_sound_group_uninit(&m_imp->sfxGrp);
			ma_engine_uninit(&m_imp->engine);
			m_imp->initialized = false;
		}
	}

	SFXId AudioSystem::registerSFX(const std::string& filepath, SFXType type) {
		//filepath exists in registry, meaning we already have template in library
		if (pathRegistry.contains(filepath))
		{
			uint32_t id = pathRegistry.find(filepath)->second;
			return SFXId{ id, type };
		}

		UniquePtrSound sound(new ma_sound());
		if (type == NONGAME_SFX)
		{
			//fully decode + NO SPATIALIZATION BECAUSE THESE ARE UI/OTHER SFX
			ma_uint32 flags = MA_SOUND_FLAG_DECODE | MA_SOUND_FLAG_NO_SPATIALIZATION;
			//init sound in nongame sfx group
			ma_result result = ma_sound_init_from_file(&m_imp->engine, filepath.c_str(), flags, &m_imp->sfxNonGameGrp, nullptr, sound.get());
			if (result != MA_SUCCESS)
			{
				m_logger->warn("Failed to init NONGAME_SFX sound from {}", filepath);
				return SFXId{ 0xFFFFFFFF, NONGAME_SFX }; //uint32 max, error return value
			}
		}
		else //GAME_SFX
		{
			//fully decode. spatialization is on by default so no no_spatialization flag here
			ma_uint32 flags = MA_SOUND_FLAG_DECODE;
			//init sound in game sfx group
			ma_result result = ma_sound_init_from_file(&m_imp->engine, filepath.c_str(), flags, &m_imp->sfxGameGrp, nullptr, sound.get());
			if (result != MA_SUCCESS)
			{
				m_logger->warn("Failed to init GAME_SFX sound from {}", filepath);
				return SFXId{ 0xFFFFFFFF, GAME_SFX }; //uint32 max, error return value
			}
		}
		uint32_t returnId = static_cast<uint32_t>(m_imp->sfxLibrary.size()); //new sfx will always go @ back, size gives correct index before we push_back
		pathRegistry.emplace(filepath, returnId); //make sure to add filepath to registry lol
		m_imp->sfxLibrary.push_back(std::move(sound));

		return SFXId{ returnId, type };
	}

	MusicId AudioSystem::registerMusic(const std::string& filepath) {
		if (pathRegistry.contains(filepath))
		{
			uint32_t id = pathRegistry.find(filepath)->second;
			return MusicId{ id };
		}
		uint32_t returnId = static_cast<uint32_t>(m_imp->musicLibrary.size()); //new music will always go @ back, size gives correct index before we push_back
		pathRegistry.emplace(filepath, returnId); //make sure to add filepath to registry
		m_imp->musicLibrary.push_back(filepath);

		return MusicId{ 0 };
	}

	void AudioSystem::PlaySFX(SFXId id) {
		UniquePtrSound sfx(new ma_sound());
		if (id.type == NONGAME_SFX)
		{
			//NO SPATIALIZATION AS THIS IS NOT A GAME SFX
			ma_uint32 flags = MA_SOUND_FLAG_NO_SPATIALIZATION;
			//create sound copy from the template at id.index in sfxLibrary
			ma_result result = ma_sound_init_copy(&m_imp->engine, m_imp->sfxLibrary.at(id.index).get(), flags, &m_imp->sfxNonGameGrp, sfx.get());
			if (result != MA_SUCCESS)
			{
				m_logger->warn("Failure to copy NONGAME_SFX sound id {} from template", id.index);
			}
			//ANY OTHE RTHINGS HERE....
		}
		else //id.type == GAME_SFX
		{
			//yes spatialization as this is a game sfx
			ma_uint32 flags = 0;
			//create sound copy from the template at id.index in sfxLibrary
			ma_result result = ma_sound_init_copy(&m_imp->engine, m_imp->sfxLibrary.at(id.index).get(), flags, &m_imp->sfxGameGrp, sfx.get());
			if (result != MA_SUCCESS)
			{
				m_logger->warn("Failure to copy GAME_SFX sound id {} from template", id.index);
			}
			//ANY OTHE RTHINGS HERE....
		}
		m_imp->test.push_back(std::move(sfx));
		ma_sound_start(m_imp->test.back().get());
	}

	void AudioSystem::PlaySound(std::string_view path) {
		ma_engine_play_sound(&m_imp->engine, path.data(), nullptr);
	}

	void AudioSystem::toggleMute() {
		if (isMuted)
		{
			//kill and uninit active sfx instances
			//ma_engine_stop(&m_imp->engine)
		}
		else
		{
			//ma_engine_start(&m_imp->engine)
		}
		isMuted = !isMuted;
	}

	bool AudioSystem::_initSfxGroups()
	{
		//init sfx parent group
		if (ma_sound_group_init(&m_imp->engine, 0, nullptr, &m_imp->sfxGrp) != MA_SUCCESS)
		{
			m_logger->critical("SFX PARENT GROUP FAILED TO INITIALIZE, ABORT ABORT");
			m_imp->initialized = false;
			return false;
		}
		//init nongame sfx group
		if (ma_sound_group_init(&m_imp->engine, 0, &m_imp->sfxGrp, &m_imp->sfxNonGameGrp) != MA_SUCCESS)
		{
			m_logger->critical("SFX NON-GAME GROUP FAILED TO INITIALIZE, ABORT ABORT");
			m_imp->initialized = false;
			return false;
		}
		//init game sfx group
		if (ma_sound_group_init(&m_imp->engine, 0, &m_imp->sfxGrp, &m_imp->sfxGameGrp) != MA_SUCCESS)
		{
			m_logger->critical("SFX GAME GROUP FAILED TO INITIALIZE, ABORT ABORT");
			m_imp->initialized = false;
			return false;
		}
		return true;
	}
} //namespace SE
