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

		//registers a music sound into the music library. returns a MusicId
		//VERY IMPORTANT: since music library only holds strings (not ma_sound ptrs), all this func does is add filepath to the library.
		//ma_sound gets created in PlayMusic because we have to stream from disk
		MusicId registerMusic(const std::string& filepath);

		InstanceHandle PlaySFX(SFXId id);
		void StopSFX(InstanceHandle handle);
		void PlayMusic(MusicId id);

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
		
		bool isMuted;

		//pimpl idiom
		struct AudioImpl;
		std::unique_ptr<AudioImpl> m_imp;
		//sfx library inside AudioImpl

		//maps filepaths->id for registry caching
		//currently, both sfx and music use this registery. THIS CAN BE CHANGED IN THE FUTURE
		//SO SFX AND MUSIC HAVE SEPARATE PATH REGISTRIES.
		std::unordered_map<std::string, uint32_t> pathRegistry;
		
		//generational handle setup
		//used for short-life sounds (sfx)
		struct SoundSlot;
		std::vector<SoundSlot> sfxList;
		std::vector<uint32_t> freeSlots;

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
