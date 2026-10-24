#include "pch.h"
#include "AudioComponents.h"
#include "Engine.h"

namespace SE
{
	//AUDIO SFX COMPONENT
	AudioSFXComponent::AudioSFXComponent()
	    : m_filepath(""),
	      m_isGameSFX(false),
	      m_myId({ 0xFFFFFFFF, NONGAME_SFX }),
	      m_myHandle({ 0xFFFFFFFF, 0 })
	{}

	AudioSFXComponent::AudioSFXComponent(std::string_view filepath, bool isGameSFX)
	    : m_filepath(filepath),
	      m_isGameSFX(isGameSFX),
	      m_myId({ 0xFFFFFFFF, NONGAME_SFX }),
	      m_myHandle({ 0xFFFFFFFF, 0 })
	{
		//just call ts with member var params
		//this func has params because we need to recreate sound if filepath is changed in editor
		createSound(m_filepath, m_isGameSFX);
	}

	AudioSFXComponent::~AudioSFXComponent()
	{
		//will only implement this if needed, but we dont really want sounds lingering in template library
		//if there is nothing using them in the engine. orphan sounds should be fine for now though.
		//Here is where we would remove a sound from template library, but would need dedicated func in AudioSystem
		//or actually this would probably happen in an Entity::_onSFXComponentRemoved func.
	}

	bool AudioSFXComponent::createSound(const std::string& filepath, bool isGameSFX)
	{
		//register sound with audio system and give back an SFXId
		m_myId = Engine::Instance().getSubSystem<AudioSystem>()->registerSFX(filepath, isGameSFX ? GAME_SFX : NONGAME_SFX);
		if (m_myId.index == 0xFFFFFFFF) //load failed
			return false;
		m_filepath = filepath;
		m_isGameSFX = isGameSFX;
		return true;
	}

	void AudioSFXComponent::PlayMySound(const glm::vec3& initialPos)
	{
		m_myHandle = Engine::Instance().getSubSystem<AudioSystem>()->PlaySFX(m_myId, initialPos);
	}

	void AudioSFXComponent::StopMySound() const
	{
		Engine::Instance().getSubSystem<AudioSystem>()->StopSFX(m_myHandle);
	}

	void AudioSFXComponent::updateMyPosition(const glm::vec3& pos) { 
		Engine::Instance().getSubSystem<AudioSystem>()->SetSFXPosition(m_myHandle, pos);
	}

// ======= AUDIO MUSIC COMPONENT =======================================================================================================================================
	AudioMusicComponent::AudioMusicComponent()
	    : m_filepath(""),
	      m_myId({ 0xFFFFFFFF })
	{}
	AudioMusicComponent::AudioMusicComponent(std::string_view filepath)
	    : m_filepath(filepath),
	      m_myId({ 0xFFFFFFFF })
	{
		createTrack(m_filepath);
	}

	bool AudioMusicComponent::createTrack(const std::string& filepath) {
		//register sound with audio system and give back an SFXId
		m_myId = Engine::Instance().getSubSystem<AudioSystem>()->registerMusic(filepath);
		if (m_myId.index == 0xFFFFFFFF) //load failed
			return false;
		m_filepath = filepath;
		return true;
	}

	void AudioMusicComponent::PlayMyTrack() {
		Engine::Instance().getSubSystem<AudioSystem>()->PlayMusic(m_myId);
	}

	void AudioMusicComponent::StopMyTrack() const {
		Engine::Instance().getSubSystem<AudioSystem>()->StopCurrentMusic();
	}
} //namespace