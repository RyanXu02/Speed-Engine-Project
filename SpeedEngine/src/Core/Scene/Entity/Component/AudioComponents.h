#pragma once
#include "Core/Features/Serializable.h"
#include "SubSystems/Audio/AudioSystem.h"

namespace SE
{
	//@brief Represents an audio component for SFX that can be attached to an entity.
	//Each SFX component has its own sound, so calling PlayMySound while its sound is 
	//active restarts it. Can be changed to deal with multiple handles in the future 
	//(i.e. with an automatic weapon or something with a similar audio playback pattern)
	class AudioSFXComponent
	{
	public:
		AudioSFXComponent();
		AudioSFXComponent(std::string_view filepath, bool isGameSFX);
		~AudioSFXComponent();

		//create sound with audio system
		bool createSound(const std::string& filepath, bool isGameSFX);
		void PlayMySound(const glm::vec3& initialPos);
		void StopMySound() const;

		void updateMyPosition(const glm::vec3& pos);

		bool isGameSFX() const { return m_isGameSFX; }
		const std::string& getFilepath() { return m_filepath; }

		DEFINE_TYPE(AudioSFXComponent, 
			member("filepath", &AudioSFXComponent::m_filepath), 
			member("isGameSFX", &AudioSFXComponent::m_isGameSFX));

	private:
		std::string m_filepath = "";
		SFXId m_myId;
		InstanceHandle m_myHandle;
		bool m_isGameSFX;
	};

	//@brief Represents an audio component for music that can be attached to an entity
	class AudioMusicComponent
	{
	public:
		AudioMusicComponent();
		AudioMusicComponent(std::string_view filepath);

		bool createTrack(const std::string& filepath);
		void PlayMyTrack();
		void StopMyTrack() const;

		const std::string& getFilepath() { return m_filepath; }

		DEFINE_TYPE(AudioMusicComponent, member("filepath", &AudioMusicComponent::m_filepath));

	private:
		std::string m_filepath;
		MusicId m_myId;
	};

} //namespace
