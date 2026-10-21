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
	
	//helpers not in SE namespace
	namespace
	{
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
		freeSlots.reserve(MAX_ACTIVE_SFX); //probably way too many lol (50) but whatever its fine for now.
		sfxList.reserve(MAX_ACTIVE_SFX);
		//put all indices on free list. push back in reverse order so pop_back will get index @ front of sfxList
		for (uint32_t i = MAX_ACTIVE_SFX; i != 0; --i)
			freeSlots.push_back(i-1);
		//populate sfxList with empty sound slots
		for (int i = 0; i < MAX_ACTIVE_SFX; ++i)
			sfxList.push_back(SoundSlot());
		m_imp->initialized = true;
	}

	void AudioSystem::update(double dt) {
		//walk sfxList. for each item, if its inUse, check if the sound has ended.
		//if ended, free the slot.
		for (size_t i = 0; i < sfxList.size(); ++i)
		{
			auto& item = sfxList[i];
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
			sfxList.clear();

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

		UniquePtrSound sound;
		if (type == NONGAME_SFX)
		{
			//fully decode + NO SPATIALIZATION BECAUSE THESE ARE UI/OTHER SFX
			ma_uint32 flags = MA_SOUND_FLAG_DECODE | MA_SOUND_FLAG_NO_SPATIALIZATION;

			//init sound in nongame sfx group
			sound = makeSoundFromFile(&m_imp->engine, filepath, flags, &m_imp->sfxNonGameGrp);
			if (!sound)
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
			sound = makeSoundFromFile(&m_imp->engine, filepath, flags, &m_imp->sfxGameGrp);
			if (!sound)
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

		return MusicId{ returnId };
	}

	InstanceHandle AudioSystem::PlaySFX(SFXId id) {
		//check if active sfx cap is reached
		if (freeSlots.empty())
		{
			m_logger->warn("Cannot play SFX id {} due to max active SFX cap", id.index);
			return InstanceHandle{ 0xFFFFFFFF, 0 }; //uint32 max index and 0 generation is invalid instance handle.
		}
		if (id.index > m_imp->sfxLibrary.size())
			return InstanceHandle{ 0xFFFFFFFF, 0 }; //invalid instance handle because invalid SFXid

		UniquePtrSound sfx;
		if (id.type == NONGAME_SFX)
		{
			//NO SPATIALIZATION AS THIS IS NOT A GAME SFX
			ma_uint32 flags = MA_SOUND_FLAG_NO_SPATIALIZATION;

			//create sound copy from the template at id.index in sfxLibrary
			sfx = makeSoundCopy(&m_imp->engine, m_imp->sfxLibrary[id.index].get(), flags, &m_imp->sfxNonGameGrp);
			if (!sfx)
				m_logger->warn("Failure to copy NONGAME_SFX sound id {} from template", id.index);
			//ANY OTHE RTHINGS HERE....
		}
		else //id.type == GAME_SFX
		{
			//yes spatialization as this is a game sfx
			ma_uint32 flags = 0;

			//create sound copy from the template at id.index in sfxLibrary
			sfx = makeSoundCopy(&m_imp->engine, m_imp->sfxLibrary[id.index].get(), flags, &m_imp->sfxGameGrp);
			if (!sfx)
				m_logger->warn("Failure to copy GAME_SFX sound id {} from template", id.index);
			//ANY OTHE RTHINGS HERE....
		}
		if (!sfx)
			return InstanceHandle{ 0xFFFFFFFF, 0 }; //uint32 max index and 0 generation is invalid instance handle.

		//pop free slot from free list
		uint32_t freeIdx = freeSlots.back();
		freeSlots.pop_back();
		sfxList[freeIdx].sound = std::move(sfx); //give the copy ptr to the sfxList
		sfxList[freeIdx].inUse = true;
		ma_sound_start(sfxList[freeIdx].sound.get());

		//return an instance handle with the retrieved idx and the associated slot's generation so the ticket and slot match (VERY IMPORTANT)
		return InstanceHandle{ freeIdx, sfxList[freeIdx].generation };
		//m_imp->test.push_back(std::move(sfx));
		//ma_sound_start(m_imp->test.back().get());
	}

	void AudioSystem::StopSFX(InstanceHandle handle) {
		//validate the handle (coat check ticket)
		//if its valid, then proceed with stopping the sfx and freeing the slot.
		bool isValid = _validateHandle(handle);
		if (isValid)
		{
			auto& item = sfxList[handle.index];
			ma_result result = ma_sound_stop(item.sound.get());
			_freeSoundSlot(item, handle.index);
		}
	}

	void AudioSystem::PlayMusic(MusicId id)
	{
		//stream music and NO SPATIALIZATION. looping is there cuz usually game music loops
		ma_uint32 flags = MA_SOUND_FLAG_STREAM | MA_SOUND_FLAG_NO_SPATIALIZATION | MA_SOUND_FLAG_LOOPING;
		UniquePtrSound track = makeSoundFromFile(&m_imp->engine, m_imp->musicLibrary[id.index], flags, &m_imp->musicGrp);
		if (!track)
		{
			m_logger->warn("Failure to init MUSIC sound id {}", id.index);
			return;
		}
		m_imp->currentMusic = std::move(track);
		ma_sound_start(m_imp->currentMusic.get());
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

	void AudioSystem::_freeSoundSlot(SoundSlot& s, size_t i) {
		s.sound.reset(); //free sound. reset() will get the custom deleter to free the ma_sound
		++s.generation;
		s.inUse = false;
		freeSlots.push_back(static_cast<uint32_t>(i));
	}

	bool AudioSystem::_validateHandle(InstanceHandle handle) {
		//check if handle is invalid, return false
		if (handle.index >= sfxList.size())
			return false;
		//for the slot specified by the handle's index:
		// if it is in use AND its generation matches the handle's generation, we are good
		auto& item = sfxList[handle.index];
		if (item.inUse && handle.generation == item.generation)
			return true;
		return false;
	}
} //namespace SE
