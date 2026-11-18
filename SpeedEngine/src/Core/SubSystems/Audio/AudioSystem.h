#pragma once
#include "Core/SubSystems/Subsystem.h"

//for vec3
#include <glm/vec3.hpp>

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
	//these are temp values for now, will probably need to add min/max attenuation distance params to sfx component eventually
	constexpr float GLOBAL_MIN_ATTEN_DIST = 1.f;
	constexpr float GLOBAL_MAX_ATTEN_DIST = 15.f;


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

		//initialPos is so sound starts playing in correct location instead of suddenly teleporting to the right location from next position update
		//is only relevant for game sfx. does nothing for nongame sfx
		InstanceHandle PlaySFX(SFXId id, const glm::vec3& initialPos);
		void StopSFX(InstanceHandle handle);
		void PlayMusic(MusicId id);
		void StopCurrentMusic();

		//sets the listener data.
		//@param pos the position of the listener
		//@param forward the forward vector of the listener
		//@param the up vector of the listener. see comment inside func def about this one
		void SetListener(const glm::vec3& pos, const glm::vec3& forward, const glm::vec3& up);

		//sets the listener's directional attenuation cone. innerAngleDegrees determines the inner cone angle and outerAngleDegrees
		// the outer cone angle, centered on the listener's forward vector. outerGain is the gain sounds outside the outer cone will have
		// Ex: SetListenerCone(90.f,180.f,0.3f) will set the inner cone to 90 deg and outer cone to 180 deg, so 45 deg and 90 deg will be on either side of the listener's
		// forward vector respectively. What this means is that sounds within the inner cone will have full gain, sounds outside outer cone will have outerGain level of gain,
		// and sounds in between inner and outer cone will blend between the two accordingly.
		//@param innerAngleDegrees inner cone angle in degrees, centered on listener forward vec
		//@param outerAngleDegrees outer cone angle in degrees, centered on listener forward vec
		//@param outerGain gain sounds will have outside outer cone. clamped from 0.f to 1.f
		void SetListenerCone(float innerAngleDegrees, float outerAngleDegrees, float outerGain);

		//sets the position of the sound referenced by the instance handle to pos.
		//@returns true if success, false if failed. Will only fail if the handle is stale
		bool SetSFXPosition(InstanceHandle handle, const glm::vec3& pos);

		void toggleMute();
		void changeNonGameSFXVolume(float volume);
		void changeGameSFXVolume(float volume);
		void changeMusicSFXVolume(float volume);
		void changeMasterSFXVolume(float volume);
		void changeMasterVolume(float volume);
	private:
		AudioSystem();
		

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

		//TODO: NEED A WAY OF SERIALIZING AUDIO SYSTEM(?) TO MAKE MUTE + VOLUME SETTINGS PERSISTENT.
		//may have to create sound group volume member vars for this, but should be pretty easy future work lol
		bool isMuted;

		bool _initSfxGroups();

		//frees a sound slot @ index i. 
		//specifically, releases audio memory, increments generation, resets inUse flag, and puts idx back on free list
		void _freeSoundSlot(SoundSlot& s, size_t i);

		//checks if a handle is still valid in the sfx slots list
		bool _validateHandle(InstanceHandle handle);
	};
} //namespace SE