#include "pch.h"
#include "AudioSystem.h"
#include "Engine.h"
#include "Core/Logger/Logger.h"
#include "SubSystems/Scenes/SceneSystem.h"

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

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
	
	//helpers not in SE namespace
	namespace Helpers
	{
		enum class LoadState
		{
			READY,
			LOADING,
			FAILED
		};

		//init a sound from file with parameters. returns nullptr if failure or UniquePtrSound if success
		UniquePtrSound makeSoundFromFile(ma_engine* engine, const std::string& filepath, ma_uint32 flags, ma_sound_group* group) {
			std::unique_ptr<ma_sound> raw(new ma_sound());
			ma_result result = ma_sound_init_from_file(engine, filepath.c_str(), flags, group, nullptr, raw.get());
			if (result != MA_SUCCESS)
			{
				return nullptr;
			}
			return UniquePtrSound(raw.release());
		}

		//init a sound from copy with parameters. returns nullptr if failure or UniquePtrSound if success
		UniquePtrSound makeSoundCopy(ma_engine* engine, const ma_sound* src, ma_uint32 flags, ma_sound_group* group) {
			std::unique_ptr<ma_sound> raw(new ma_sound());
			ma_result result = ma_sound_init_copy(engine, src, flags, group, raw.get());
			if (result != MA_SUCCESS)
			{
				return nullptr;
			}
			return UniquePtrSound(raw.release());
		}

		LoadState getSoundLoadState(const ma_sound* sound)
		{
			auto& soundDataSrcPtr = sound->pResourceManagerDataSource;
			//technically should check if nullptr here but ts is only set for sounds created from file (which all of ones in this system are)
			ma_result result = ma_resource_manager_data_source_result(soundDataSrcPtr);
			switch (result) 
			{ 
			case MA_SUCCESS: return LoadState::READY;
			case MA_BUSY: return LoadState::LOADING;
			default: return LoadState::FAILED;
			}
		}
	}

	//pimpl idiom (pointer implementation)
	//allows for the big ass miniaudio.h to only need including in this file, all audio-related needs
	//are accessed via including AudioSystem.h.
	struct AudioSystem::AudioImpl
	{
		ma_engine engine{};
		ma_sound_group musicGrp{}, sfxGrp{}, sfxNonGameGrp{}, sfxGameGrp{};
		SfxLibrary sfxLibrary;
		std::vector<std::string> musicLibrary;
		UniquePtrSound currentMusic; //IF WE WANT MULTIPLE SONGS PLAYING AT ONCE, THIS CAN BE CHANGED TO A VECTOR
		//reusing PendingSound, so for music, index can be either 0x0 (not loading), 0x1 (loading), or 0xFFFFFFFF (load failed). init with 0x12345678 lol
		PendingSound currMusicLoadState = { "", 0x12345678 };
		bool initialized = false;
	};

	struct AudioSystem::SoundSlot 
	{
		UniquePtrSound sound;
		uint32_t generation = 1; //generation of 0 is invalid
		bool inUse = false;
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
		engineConfig.listenerCount = 1; //will only have 1 listener, the player. thus listener index will be 0
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

		//init generational handle setup
		m_freeSlots.reserve(MAX_ACTIVE_SFX); //probably way too many lol (50) but whatever its fine for now.
		m_sfxList.reserve(MAX_ACTIVE_SFX);
		//put all indices on free list. push back in reverse order so pop_back will get index @ front of m_sfxList
		for (uint32_t i = MAX_ACTIVE_SFX; i != 0; --i)
			m_freeSlots.push_back(i-1);
		//populate m_sfxList with empty sound slots
		for (int i = 0; i < MAX_ACTIVE_SFX; ++i)
			m_sfxList.push_back(SoundSlot());
		m_imp->initialized = true;
	}

	void AudioSystem::update(double dt) {
		//check pending list for sound that are done being loaded
		for (auto it = m_loadPendingList.begin(); it != m_loadPendingList.end();)
		{
			auto state = Helpers::getSoundLoadState(m_imp->sfxLibrary[(*it).index].get());
			if (state == Helpers::LoadState::READY)
			{
				it = m_loadPendingList.erase(it);
			}
			else if (state == Helpers::LoadState::FAILED)
			{
				m_logger->warn("SFX file {} with id {} failed to async load", (*it).filepath, (*it).index);
				it = m_loadPendingList.erase(it);
			}
			else
				++it;
		}
		//check current music load state as well
		if (m_imp->currMusicLoadState.index == 0x1) //sound is supposedly still loading, go check ts
		{
			auto musicState = Helpers::getSoundLoadState(m_imp->currentMusic.get());
			if (musicState == Helpers::LoadState::READY)
				m_imp->currMusicLoadState.index = 0; //no longer loading
			else if (musicState == Helpers::LoadState::FAILED)
			{
				m_logger->warn("Current music track {} failed to load, track is being dropped", m_imp->currMusicLoadState.filepath);
				m_imp->currMusicLoadState.index = 0xFFFFFFFF;
			}
		}

		//walk m_sfxList. for each item, if its inUse, check if the sound has ended.
		//if ended, free the slot.
		for (size_t i = 0; i < m_sfxList.size(); ++i)
		{
			auto& item = m_sfxList[i];
			if (!item.inUse)
				continue;
			//item is in use, poll sound
			if (ma_sound_at_end(item.sound.get()) == MA_TRUE)
				_freeSoundSlot(item, i);
		}
	}

	void AudioSystem::shutdown() {
		SubSystem::shutdown();

		//cleanup sounds(playing then library), then groups(children before parent), then engine in that order
		if (m_imp->initialized)
		{
			//cleanup ma_sounds here. MUST USE vector.clear() OR ELSE DOUBLE FREE WILL HAPPEN ON ENGINE SHUTDOWN
			m_imp->currentMusic.reset();
			m_imp->sfxLibrary.clear();
			m_sfxList.clear();

			ma_sound_group_uninit(&m_imp->musicGrp);
			ma_sound_group_uninit(&m_imp->sfxNonGameGrp);
			ma_sound_group_uninit(&m_imp->sfxGameGrp);
			ma_sound_group_uninit(&m_imp->sfxGrp);
			ma_engine_uninit(&m_imp->engine);
			m_imp->initialized = false;
		}
	}

	//functions used to register sounds with the Audio System
#pragma region RegistrationFunctions

	SFXId AudioSystem::registerSFX(const std::string& filepath, SFXType type) {
		//filepath exists in registry, meaning we already have template in library
		if (m_sfxPathRegistry.contains(filepath))
		{
			uint32_t id = m_sfxPathRegistry.find(filepath)->second;
			return SFXId{ id, type };
		}

		UniquePtrSound sound;
		if (type == NONGAME_SFX)
		{
			//fully decode + NO SPATIALIZATION BECAUSE THESE ARE UI/OTHER SFX
			ma_uint32 flags = MA_SOUND_FLAG_DECODE | MA_SOUND_FLAG_ASYNC | MA_SOUND_FLAG_NO_SPATIALIZATION;
#ifdef COMPILE_WITH_EDITOR
			flags |= MA_SOUND_FLAG_WAIT_INIT; //force synchronous loading with editor only
#endif

			//init sound in nongame sfx group
			sound = Helpers::makeSoundFromFile(&m_imp->engine, filepath, flags, &m_imp->sfxNonGameGrp);
			if (!sound)
			{
				m_logger->warn("Failed to init NONGAME_SFX sound from {}", filepath);
				return SFXId{ 0xFFFFFFFF, NONGAME_SFX }; //uint32 max, error return value
			}
		}
		else //GAME_SFX
		{
			//fully decode. spatialization is on by default so no no_spatialization flag here
			ma_uint32 flags = MA_SOUND_FLAG_DECODE | MA_SOUND_FLAG_ASYNC;
#ifdef COMPILE_WITH_EDITOR
			flags |= MA_SOUND_FLAG_WAIT_INIT; //force synchronous loading with editor only
#endif

			//init sound in game sfx group
			sound = Helpers::makeSoundFromFile(&m_imp->engine, filepath, flags, &m_imp->sfxGameGrp);
			if (!sound)
			{
				m_logger->warn("Failed to init GAME_SFX sound from {}", filepath);
				return SFXId{ 0xFFFFFFFF, GAME_SFX }; //uint32 max, error return value
			}
		}
		uint32_t returnId = static_cast<uint32_t>(m_imp->sfxLibrary.size()); //new sfx will always go @ back, size gives correct index before we push_back
		m_loadPendingList.push_back({ filepath, returnId }); //immediately put in pending list
		m_sfxPathRegistry.emplace(filepath, returnId); //make sure to add filepath to registry lol
		m_imp->sfxLibrary.push_back(std::move(sound));

		return SFXId{ returnId, type };
	}

	MusicId AudioSystem::registerMusic(const std::string& filepath) {
		if (m_musicPathRegistry.contains(filepath))
		{
			uint32_t id = m_musicPathRegistry.find(filepath)->second;
			return MusicId{ id };
		}
		uint32_t returnId = static_cast<uint32_t>(m_imp->musicLibrary.size()); //new music will always go @ back, size gives correct index before we push_back
		m_musicPathRegistry.emplace(filepath, returnId); //make sure to add filepath to registry
		m_imp->musicLibrary.push_back(filepath);

		return MusicId{ returnId };
	}

#pragma endregion RegistrationFunctions

	void AudioSystem::clearSoundLibraries() {
		//clear every in-use slot
		for (size_t i = 0; i < m_sfxList.size(); ++i)
		{
			auto& item = m_sfxList[i];
			if (item.inUse)
				_freeSoundSlot(item, i);
		}
		//clear the pending sounds list, sfx library, music library
		m_loadPendingList.clear();
		m_imp->sfxLibrary.clear();
		m_imp->musicLibrary.clear();
		//clear path registries
		m_sfxPathRegistry.clear();
		m_musicPathRegistry.clear();
	}

	bool AudioSystem::areSoundsLoaded() const {
		//if pending sound list is empty, then all sounds were successfully fully loaded into memory
		return m_loadPendingList.empty();
	}

	//nontrivial functions that other parts of the engine (e.g. components) call to interact with the Audio System
#pragma region CrucialInteractionFunctions

	InstanceHandle AudioSystem::PlaySFX(SFXId id, const glm::vec3& initialPos) {
		//check if active sfx cap is reached
		if (m_freeSlots.empty())
		{
			m_logger->warn("Cannot play SFX id {} due to max active SFX cap", id.index);
			return InstanceHandle{ 0xFFFFFFFF, 0 }; //uint32 max index and 0 generation is invalid instance handle.
		}
		if (id.index >= m_imp->sfxLibrary.size() || isMuted)
			return InstanceHandle{ 0xFFFFFFFF, 0 }; //invalid instance handle because invalid SFXid OR the engine is muted

		//check load state of sound
		switch (Helpers::getSoundLoadState(m_imp->sfxLibrary[id.index].get()))
		{ 
		case Helpers::LoadState::LOADING:
			{
				m_logger->warn("Template sound for id {} is still loading, dropping sound", id.index);
				return InstanceHandle{ 0xFFFFFFFF, 0 };
			}
		case Helpers::LoadState::FAILED:
			{
				m_logger->warn("Template sound for id {} failed to load", id.index);
				return InstanceHandle{ 0xFFFFFFFF, 0 };
			}
		case Helpers::LoadState::READY: break;
		}
		//here means template sound is loaded, can continue to copy

		UniquePtrSound sfx;
		if (id.type == NONGAME_SFX)
		{
			//NO SPATIALIZATION AS THIS IS NOT A GAME SFX
			ma_uint32 flags = MA_SOUND_FLAG_NO_SPATIALIZATION;

			//create sound copy from the template at id.index in sfxLibrary
			sfx = Helpers::makeSoundCopy(&m_imp->engine, m_imp->sfxLibrary[id.index].get(), flags, &m_imp->sfxNonGameGrp);
			if (!sfx)
				m_logger->warn("Failure to copy NONGAME_SFX sound id {} from template", id.index);
			//ANY OTHE RTHINGS HERE....
		}
		else //id.type == GAME_SFX
		{
			//yes spatialization as this is a game sfx
			ma_uint32 flags = 0;

			//create sound copy from the template at id.index in sfxLibrary
			sfx = Helpers::makeSoundCopy(&m_imp->engine, m_imp->sfxLibrary[id.index].get(), flags, &m_imp->sfxGameGrp);
			if (!sfx)
				m_logger->warn("Failure to copy GAME_SFX sound id {} from template", id.index);
			//ANY OTHE RTHINGS HERE....
			//set initial position of game sfx. attenuation model is inverse by default for spatialized sounds. still need to set min/max atten dist's
			ma_sound_set_position(sfx.get(), initialPos.x, initialPos.y, initialPos.z);
			ma_sound_set_min_distance(sfx.get(), GLOBAL_MIN_ATTEN_DIST);
			ma_sound_set_max_distance(sfx.get(), GLOBAL_MAX_ATTEN_DIST);
			//possibly rollof here as well
		}
		if (!sfx)
			return InstanceHandle{ 0xFFFFFFFF, 0 }; //uint32 max index and 0 generation is invalid instance handle.

		//pop free slot from free list
		uint32_t freeIdx = m_freeSlots.back();
		m_freeSlots.pop_back();
		m_sfxList[freeIdx].sound = std::move(sfx); //give the copy ptr to the m_sfxList
		m_sfxList[freeIdx].inUse = true;
		ma_sound_start(m_sfxList[freeIdx].sound.get());

		//return an instance handle with the retrieved idx and the associated slot's generation so the ticket and slot match (VERY IMPORTANT)
		return InstanceHandle{ freeIdx, m_sfxList[freeIdx].generation };
	}

	void AudioSystem::StopSFX(InstanceHandle handle) {
		//validate the handle (coat check ticket)
		//if its valid, then proceed with stopping the sfx and freeing the slot.
		bool isValid = _validateHandle(handle);
		if (isValid)
		{
			auto& item = m_sfxList[handle.index];
			ma_sound_stop(item.sound.get());
			_freeSoundSlot(item, handle.index);
		}
	}

	void AudioSystem::PlayMusic(MusicId id) {
		if (id.index >= m_imp->musicLibrary.size() || isMuted)
			return; //early return because invalid MusicId OR the engine is muted

		//stream music and NO SPATIALIZATION. looping is there cuz usually game music loops
		ma_uint32 flags = MA_SOUND_FLAG_STREAM | MA_SOUND_FLAG_NO_SPATIALIZATION | MA_SOUND_FLAG_ASYNC | MA_SOUND_FLAG_LOOPING;
		std::string filepath = m_imp->musicLibrary[id.index];
		UniquePtrSound track = Helpers::makeSoundFromFile(&m_imp->engine, filepath.c_str(), flags, &m_imp->musicGrp);
		m_imp->currMusicLoadState = { filepath, 1 }; //index = 1 is loading, index = 0 is not loading. set to 1 cuz track is loading right now
		m_imp->currentMusic = std::move(track);

		ma_sound_start(m_imp->currentMusic.get());
	}

	//subject to change if we decide to have multiple tracks playing at once.
	void AudioSystem::StopCurrentMusic() {
		ma_sound_stop(m_imp->currentMusic.get());
		m_imp->currentMusic.reset();
	}

	void AudioSystem::SetListener(const glm::vec3& pos, const glm::vec3& forward, const glm::vec3& up) {
		auto engine = &m_imp->engine;
		ma_engine_listener_set_position(engine, 0, pos.x, pos.y, pos.z);
		ma_engine_listener_set_direction(engine, 0, forward.x, forward.y, forward.z);
		//this one might change. idk if player moving over uneven terrain will make their upvec change.
		//if we don't have upvec change when going up sloped surfaces or something, then this func call can be moved to init()
		ma_engine_listener_set_world_up(engine, 0, up.x, up.y, up.z); // or 0,1,0 if static upvec
		//ma_engine_listener_set_velocity() //this is only used for doppler effect. i dont think we are going to have ts?
	}

	void AudioSystem::SetListenerCone(float innerAngleDegrees, float outerAngleDegrees, float outerGain)
	{
		ma_engine_listener_set_cone(&m_imp->engine, 0, glm::radians(innerAngleDegrees), glm::radians(outerAngleDegrees), glm::clamp(outerGain, 0.f, 1.f));
	}

	bool AudioSystem::SetSFXPosition(InstanceHandle handle, const glm::vec3& pos) {
		bool isValid = _validateHandle(handle);
		if (isValid)
		{
			auto& slot = m_sfxList[handle.index];
			ma_sound_set_position(slot.sound.get(), pos.x, pos.y, pos.z);
			return true;
		}
		return false;
	}

#pragma endregion CrucialInteractionFunctions

	//functions used in menus to control Audio System. Basically just volume control lol
#pragma region MenuControlFunctions

	//TODO: currently rips out looping sounds as well, will have to change this later
	void AudioSystem::toggleMute() {
		if (!isMuted)
		{
			//kill and uninit active sfx instances
			for (size_t i = 0; i < m_sfxList.size(); ++i)
			{
				auto& item = m_sfxList[i];
				if (item.inUse)
					_freeSoundSlot(item, i);
			}
			ma_engine_stop(&m_imp->engine);
		}
		else
		{
			ma_engine_start(&m_imp->engine);
		}
		isMuted = !isMuted;
	}

	void AudioSystem::changeNonGameSFXVolume(float volume) {
		ma_sound_group_set_volume(&m_imp->sfxNonGameGrp, volume);
	}

	void AudioSystem::changeGameSFXVolume(float volume) {
		ma_sound_group_set_volume(&m_imp->sfxGameGrp, volume);
	}

	void AudioSystem::changeMusicSFXVolume(float volume) {
		ma_sound_group_set_volume(&m_imp->musicGrp, volume);
	}

	void AudioSystem::changeMasterSFXVolume(float volume) {
		ma_sound_group_set_volume(&m_imp->sfxGrp, volume);
	}

	void AudioSystem::changeMasterVolume(float volume) {
		ma_engine_set_volume(&m_imp->engine, volume);
	}

#pragma endregion MenuControlFunctions

	bool AudioSystem::_initSfxGroups() {
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
		//groups are explicitly for volume control, spatialization is done on per-sound basis
		ma_sound_group_set_spatialization_enabled(&m_imp->sfxGameGrp, false);
		ma_sound_group_set_spatialization_enabled(&m_imp->sfxNonGameGrp, false);

		return true;
	}

	void AudioSystem::_freeSoundSlot(SoundSlot& s, size_t i) {
		s.sound.reset(); //free sound. reset() will get the custom deleter to free the ma_sound
		++s.generation;
		s.inUse = false;
		m_freeSlots.push_back(static_cast<uint32_t>(i));
	}

	bool AudioSystem::_validateHandle(InstanceHandle handle) {
		//check if handle is invalid, return false
		if (handle.index >= m_sfxList.size())
			return false;
		//for the slot specified by the handle's index:
		// if it is in use AND its generation matches the handle's generation, we are good
		auto& item = m_sfxList[handle.index];
		if (item.inUse && handle.generation == item.generation)
			return true;
		return false;
	}
} //namespace SE
