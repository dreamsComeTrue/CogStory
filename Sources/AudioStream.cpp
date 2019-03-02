// Copyright 2017-2019 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "AudioStream.h"
#include "AudioManager.h"

namespace aga
{
	//---------------------------------------------------------------------------

	AudioStream::AudioStream (AudioManager* manager, const std::string& name, const std::string& path)
		: m_AudioManager (manager)
		, m_Name (name)
		, m_Stream (nullptr)
		, m_Gain (1.0f)
		, m_VolumeOverriden (false)
		, m_Looping (false)
		, m_FadeInCurrent (-1.f)
		, m_FadeInMax (-1.f)
		, m_FadeOutCurrent (-1.f)
		, m_FadeOutMax (-1.f)
		, m_CurrentPos (0)
		, m_PauseOnFinish (false)
	{
		if (al_filename_exists (path.c_str ()))
		{
			m_Stream = al_load_audio_stream (path.c_str (), 4, 1024);

			if (m_Stream)
			{
				al_set_audio_stream_playmode (m_Stream, ALLEGRO_PLAYMODE_LOOP);
			}
		}
	}

	//--------------------------------------------------------------------------------------------------

	AudioStream::~AudioStream ()
	{
		if (!IsDestroyed ())
		{
			Destroy ();
		}
	}

	//--------------------------------------------------------------------------------------------------

	bool AudioStream::Initialize () { return Lifecycle::Initialize (); }

	//--------------------------------------------------------------------------------------------------

	bool AudioStream::Destroy ()
	{
		if (m_Stream)
		{
			al_drain_audio_stream (m_Stream);
			al_destroy_audio_stream (m_Stream);

			m_Stream = nullptr;
		}

		m_AudioManager = nullptr;

		return Lifecycle::Destroy ();
	}

	//--------------------------------------------------------------------------------------------------

	void AudioStream::Play ()
	{
		if (m_Stream && m_AudioManager->IsStreamsEnabled ())
		{
			float volume = 0.f;

			if (m_VolumeOverriden)
			{
				volume = m_Gain;
			}
			else
			{
				volume = m_AudioManager->GetMasterVolume ();
			}

			al_attach_audio_stream_to_mixer (m_Stream, al_get_default_mixer ());
			al_set_audio_stream_playing (m_Stream, true);
			al_set_audio_stream_gain (m_Stream, m_Gain);
		}
	}

	//--------------------------------------------------------------------------------------------------

	void AudioStream::Stop ()
	{
		if (m_Stream)
		{
			m_CurrentPos = 0;

			al_set_audio_stream_playing (m_Stream, false);
			al_seek_audio_stream_secs (m_Stream, m_CurrentPos);
		}
	}

	//--------------------------------------------------------------------------------------------------

	void AudioStream::Pause ()
	{
		if (m_Stream && m_AudioManager->IsStreamsEnabled ())
		{
			m_CurrentPos = al_get_audio_stream_position_secs (m_Stream);
			al_set_audio_stream_playing (m_Stream, false);
		}
	}

	//--------------------------------------------------------------------------------------------------

	void AudioStream::Resume ()
	{
		if (m_Stream && m_AudioManager->IsStreamsEnabled ())
		{
			al_seek_audio_stream_secs (m_Stream, m_CurrentPos);
			al_set_audio_stream_playing (m_Stream, true);
		}
	}

	//--------------------------------------------------------------------------------------------------

	void AudioStream::Update (float deltaTime)
	{
		if (!m_AudioManager->IsStreamsEnabled ())
		{
			return;
		}

		if (m_FadeInCurrent >= 0.f)
		{
			m_FadeInCurrent += deltaTime * 1000.f;

			if (m_FadeInCurrent < m_FadeInMax)
			{
				SetVolume (m_FadeInCurrent / m_FadeInMax);
			}
			else
			{
				m_FadeInCurrent = -1;
			}
		}

		if (m_FadeOutCurrent >= 0.f)
		{
			m_FadeOutCurrent -= deltaTime * 1000.f;

			if (m_FadeOutCurrent > 0)
			{
				SetVolume (m_FadeOutCurrent / m_FadeOutMax);
			}
			else
			{
				m_FadeOutCurrent = -1;

				if (m_PauseOnFinish)
				{
					Pause ();
					m_PauseOnFinish = false;
				}
			}
		}
	}

	//--------------------------------------------------------------------------------------------------

	void AudioStream::SetLooping (bool looping)
	{
		if (m_Stream && m_AudioManager->IsStreamsEnabled ())
		{
			m_Looping = looping;

			al_set_audio_stream_playmode (m_Stream, m_Looping ? ALLEGRO_PLAYMODE_LOOP : ALLEGRO_PLAYMODE_ONCE);
		}
	}

	//--------------------------------------------------------------------------------------------------

	bool AudioStream::IsLooping () const { return m_Looping; }

	//--------------------------------------------------------------------------------------------------

	void AudioStream::SetVolume (float volume)
	{
		if (m_Stream && m_AudioManager->IsStreamsEnabled ())
		{
			m_Gain = volume;
			m_VolumeOverriden = true;

			al_set_audio_stream_gain (m_Stream, m_Gain);
		}
	}

	//--------------------------------------------------------------------------------------------------

	void AudioStream::SetSpeed (float speed)
	{
		if (m_Stream && m_AudioManager->IsStreamsEnabled ())
		{
			al_set_audio_stream_speed (m_Stream, speed);
		}
	}

	//--------------------------------------------------------------------------------------------------

	bool AudioStream::IsPlaying ()
	{
		if (m_Stream)
		{
			return al_get_audio_stream_playing (m_Stream);
		}

		return false;
	}

	//--------------------------------------------------------------------------------------------------

	void AudioStream::SetFadeIn (float milliSeconds)
	{
		m_FadeInMax = milliSeconds;
		m_FadeInCurrent = 0.f;
		m_FadeOutMax = -1.f;
		m_FadeOutCurrent = -1.f;

		SetVolume (0.f);
		Play ();
	}

	//--------------------------------------------------------------------------------------------------

	void AudioStream::SetFadeOut (float milliSeconds, bool pauseOnFinish)
	{
		if (!m_AudioManager->IsStreamsEnabled ())
		{
			m_FadeOutMax = 30.0f;
			m_FadeOutCurrent = m_FadeOutMax;
		}
		else
		{
			m_FadeOutMax = milliSeconds;
			m_FadeOutCurrent = m_FadeOutMax;
		}

		m_FadeInMax = -1.f;
		m_FadeInCurrent = -1.f;
		m_Gain = 1.f;
		m_PauseOnFinish = pauseOnFinish;
	}

	//--------------------------------------------------------------------------------------------------
}
