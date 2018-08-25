// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "AudioStream.h"
#include "AudioManager.h"

namespace aga
{
    //---------------------------------------------------------------------------

    AudioStream::AudioStream (AudioManager* manager, const std::string& name, const std::string& path)
        : m_AudioManager (manager)
        , m_Name (name)
        , m_FilePath (path)
        , m_Stream (nullptr)
        , m_Gain (1.0f)
        , m_Looping (false)
        , m_FadeInCurrent (-1.f)
        , m_FadeInMax (-1.f)
        , m_FadeOutCurrent (-1.f)
        , m_FadeOutMax (-1.f)
        , m_CurrentPos (0)
        , m_PauseOnFinish (false)
    {
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

    bool AudioStream::Initialize ()
    {
        Lifecycle::Initialize ();

        if (al_filename_exists (m_FilePath.c_str ()))
        {
            m_Stream = al_load_audio_stream (m_FilePath.c_str (), 4, 1024);
        }

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    bool AudioStream::Destroy ()
    {
        if (m_Stream)
        {
            al_drain_audio_stream (m_Stream);
            al_destroy_audio_stream (m_Stream);
        }

        return Lifecycle::Destroy ();
    }

    //--------------------------------------------------------------------------------------------------

    void AudioStream::Play ()
    {
        if (m_Stream && m_AudioManager->IsEnabled ())
        {
            float volume = m_Gain;
            float masterVolume = m_AudioManager->GetMasterVolume ();

            if (!AreSame (masterVolume, 1.0))
            {
                volume = masterVolume;
            }

            al_attach_audio_stream_to_mixer (m_Stream, al_get_default_mixer ());
            al_set_audio_stream_playing (m_Stream, true);
        }
    }

    //--------------------------------------------------------------------------------------------------

    void AudioStream::Stop ()
    {
        if (m_Stream)
        {
            m_CurrentPos = 0.f;

            al_seek_audio_stream_secs (m_Stream, m_CurrentPos);
            al_set_audio_stream_playing (m_Stream, false);
        }
    }

    //--------------------------------------------------------------------------------------------------

    void AudioStream::Pause ()
    {
        if (m_Stream && m_AudioManager->IsEnabled ())
        {
            al_set_audio_stream_playing (m_Stream, false);
        }
    }

    //--------------------------------------------------------------------------------------------------

    void AudioStream::Resume ()
    {
        if (m_Stream && m_AudioManager->IsEnabled ())
        {
            al_set_audio_stream_playing (m_Stream, true);
        }
    }

    //--------------------------------------------------------------------------------------------------

    void AudioStream::Update (float deltaTime)
    {
        if (!m_AudioManager->IsEnabled ())
        {
            return;
        }

        float gain = m_Gain;

        if (m_FadeInMax > 0.f)
        {
            m_FadeInCurrent += deltaTime * 1000.f;

            if (m_FadeInCurrent < m_FadeInMax)
            {
                gain = m_FadeInCurrent / m_FadeInMax;
                SetVolume (gain);
            }
        }

        if (m_FadeOutMax > 0.f)
        {
            m_FadeOutCurrent -= deltaTime * 1000.f;

            if (m_FadeOutCurrent > 0)
            {
                gain = m_FadeOutCurrent / m_FadeOutMax;
                SetVolume (gain);
            }
            else
            {
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
        if (m_Stream && m_AudioManager->IsEnabled ())
        {
            m_Looping = looping;

            al_set_audio_stream_playmode (m_Stream, m_Looping ? ALLEGRO_PLAYMODE_LOOP : ALLEGRO_PLAYMODE_ONCE);
        }
    }

    //--------------------------------------------------------------------------------------------------

    void AudioStream::SetVolume (float volume)
    {
        if (m_Stream && m_AudioManager->IsEnabled ())
        {
            m_Gain = volume;

            al_set_audio_stream_gain (m_Stream, m_Gain);
        }
    }

    //--------------------------------------------------------------------------------------------------

    void AudioStream::SetSpeed (float speed)
    {
        if (m_Stream && m_AudioManager->IsEnabled ())
        {
            al_set_audio_stream_speed (m_Stream, speed);
        }
    }

    //--------------------------------------------------------------------------------------------------

    void AudioStream::SetFadeIn (float milliSeconds)
    {
        m_FadeInMax = milliSeconds;
        m_FadeInCurrent = 0.f;
        m_FadeOutMax = -1.f;
        m_FadeOutCurrent = -1.f;
        m_Gain = 0.f;
    }

    //--------------------------------------------------------------------------------------------------

    void AudioStream::SetFadeOut (float milliSeconds, bool pauseOnFinish)
    {
        m_FadeOutMax = milliSeconds;
        m_FadeOutCurrent = milliSeconds;
        m_FadeInMax = -1.f;
        m_FadeInCurrent = -1.f;
        m_Gain = 1.f;
        m_PauseOnFinish = pauseOnFinish;
    }

    //--------------------------------------------------------------------------------------------------
}
