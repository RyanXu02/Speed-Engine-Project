#pragma once
#include "Core/SubSystems/Subsystem.h"

namespace SE
{
	enum SFXType
	{
		NONGAME_SFX,
		GAME_SFX
	};
	struct InstanceHandle
	{
		uint32_t index;
		uint32_t generation;
	};
	struct SFXId
	{
		uint32_t index;
		SFXType type;
	};
	struct MusicId
	{
		uint32_t index;
	};

	constexpr uint8_t MAX_ACTIVE_SFX = 100;

	class AudioSystem : public SubSystem
	{
		friend class Engine;

	public:
		//need to explicitly declare destructor for pimpl
		~AudioSystem();

		void init() override;
		void update(double dt) override;
		void shutdown() override;

		//registers a sfx sound into the sfx template library. returns an SFXId. If initing sound failed, returns 0xFFFFFFFF as SFXId.index
		//type parameter is also stored in SFXId for use in PlaySFX
		SFXId registerSFX(const std::string& filepath, SFXType type);
		//void unregisterSFX(const std::string& filepath);

		//registers a music sound into the music library. returns a MusicId
		//VERY IMPORTANT: since music library only holds strings (not ma_sound ptrs), all this func does is add filepath to the library.
		//ma_sound gets created in PlayMusic because we have to stream from disk
		MusicId registerMusic(const std::string& filepath);
		//void unregisterMusic(const std::string& filepath);

		//clears all sound libraries. stops active sfx and clears loading sfx. DOES NOT STOP CURRENT MUSIC PLAYING (this can be changed later easily)
		void clearSoundLibraries();
		//checks if all sounds being asynchronously loaded are done loading. returns false if >0 sounds still loading
		bool areSoundsLoaded() const;

		InstanceHandle PlaySFX(SFXId id);
		void StopSFX(InstanceHandle handle);
		void PlayMusic(MusicId id);
		void StopCurrentMusic();

		void toggleMute();
		void changeNonGameSFXVolume(float volume);
		void changeGameSFXVolume(float volume);
		void changeMusicSFXVolume(float volume);
		void changeMasterSFXVolume(float volume);
		void changeMasterVolume(float volume);

		SFXId testsfxid{};
		MusicId testmusicid{};
	private:
		AudioSystem();
		
		//TODO: NEED A WAY OF SERIALIZING AUDIO SYSTEM(?) TO MAKE MUTE + VOLUME SETTINGS PERSISTENT.
		//may have to create sound group volume member vars for this, but should be pretty easy future work lol
		bool isMuted;

		//pimpl idiom
		struct AudioImpl;
		std::unique_ptr<AudioImpl> m_imp;
		//sfx library inside AudioImpl

		//maps filepaths->id for registry caching
		std::unordered_map<std::string, uint32_t> m_sfxPathRegistry;
		std::unordered_map<std::string, uint32_t> m_musicPathRegistry;

		struct PendingSound
		{
			std::string filepath;
			uint32_t index;
		};
		std::vector<PendingSound> m_loadPendingList;
		//TODO: if we need to, will add an areAssetsLoaded-esque function to make sure all sounds are loaded before we start simulating
		
		//generational handle setup
		//used for short-life sounds (sfx)
		struct SoundSlot;
		std::vector<SoundSlot> m_sfxList;
		std::vector<uint32_t> m_freeSlots;

		bool _initSfxGroups();

		//frees a sound slot @ index i. 
		//specifically, releases audio memory, increments generation, resets inUse flag, and puts idx back on free list
		void _freeSoundSlot(SoundSlot& s, size_t i);

		//checks if a handle is still valid in the sfx slots list
		bool _validateHandle(InstanceHandle handle);
	};
} //namespace SE

/* sound init for sfxgame pass no 3d flag, 3d by default
* sound init for sfxnongame + music, pass no_spatialization flag
*/
