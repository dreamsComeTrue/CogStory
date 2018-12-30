// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "AudioSample.h"
#include "AudioManager.h"

namespace aga
{
    //---------------------------------------------------------------------------

    AudioSample::AudioSample (AudioManager* manager, const std::string& name, const std::string& path)
        : m_AudioManager (manager)
        , m_Name (name)
        , m_Sample (nullptr)
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
            m_Sample = al_load_sample (path.c_str ());
        }
    }

    //--------------------------------------------------------------------------------------------------

    AudioSample::~AudioSample ()
    {
        if (!IsDestroyed ())
        {
            Destroy ();
        }
    }

    //--------------------------------------------------------------------------------------------------

    bool AudioSample::Initialize () { return Lifecycle::Initialize (); }

    //--------------------------------------------------------------------------------------------------

    bool AudioSample::Destroy ()
    {
        for (size_t i = 0; i < m_SampleInstances.size (); ++i)
        {
            al_destroy_sample_instance (m_SampleInstances[i]);
            m_SampleInstances.erase (m_SampleInstances.begin () + i);
        }

        m_SampleInstances.clear ();

        if (m_Sample)
        {
            al_destroy_sample (m_Sample);
            m_Sample = nullptr;
        }

        return Lifecycle::Destroy ();
    }

    //--------------------------------------------------------------------------------------------------

    void AudioSample::Play ()
    {
        if (m_Sample && m_AudioManager->IsEnabled ())
        {
            CleanUpInstances ();

            float volume = 0.f;

            if (m_VolumeOverriden)
            {
                volume = m_Gain;
            }
            else
            {
                volume = m_AudioManager->GetMasterVolume ();
            }

            ALLEGRO_SAMPLE_INSTANCE* instance = al_create_sample_instance (m_Sample);
            m_SampleInstances.push_back (instance);

            al_attach_sample_instance_to_mixer (instance, al_get_default_mixer ());
            al_set_sample_instance_playmode (instance, m_Looping ? ALLEGRO_PLAYMODE_LOOP : ALLEGRO_PLAYMODE_ONCE);
            al_set_sample_instance_gain (instance, volume);
            al_play_sample_instance (instance);
        }
    }

    //--------------------------------------------------------------------------------------------------

    void AudioSample::Stop ()
    {
        CleanUpInstances ();

        for (ALLEGRO_SAMPLE_INSTANCE* instance : m_SampleInstances)
        {
            if (al_get_sample_instance_playing (instance))
            {
                al_stop_sample_instance (instance);
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    void AudioSample::Pause ()
    {
        if (m_AudioManager->IsEnabled ())
        {
            m_CurrentPos = 0;

            for (size_t i = 0; i < m_SampleInstances.size (); ++i)
            {
                unsigned pos = al_get_sample_instance_position (m_SampleInstances[i]);

                if (pos > m_CurrentPos)
                {
                    m_CurrentPos = pos;
                }

                al_set_sample_instance_playing (m_SampleInstances[i], false);
            }

            CleanUpInstances ();
        }
    }

    //--------------------------------------------------------------------------------------------------

    void AudioSample::Resume ()
    {
        if (m_AudioManager->IsEnabled ())
        {
            for (size_t i = 0; i < m_SampleInstances.size (); ++i)
            {
                al_set_sample_instance_position (m_SampleInstances[i], m_CurrentPos);
                al_set_sample_instance_playing (m_SampleInstances[i], true);
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    bool AudioSample::IsPlaying ()
    {
        for (size_t i = 0; i < m_SampleInstances.size (); ++i)
        {
            if (al_get_sample_instance_playing (m_SampleInstances[i]))
            {
                return true;
            }
        }

        return false;
    }

    //--------------------------------------------------------------------------------------------------

    void AudioSample::Update (float deltaTime)
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

    void AudioSample::SetLooping (bool looping)
    {
        if (m_AudioManager->IsEnabled ())
        {
            m_Looping = looping;

            for (size_t i = 0; i < m_SampleInstances.size (); ++i)
            {
                al_set_sample_instance_playmode (
                    m_SampleInstances[i], m_Looping ? ALLEGRO_PLAYMODE_LOOP : ALLEGRO_PLAYMODE_ONCE);
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    bool AudioSample::IsLooping () const { return m_Looping; }

    //--------------------------------------------------------------------------------------------------

    void AudioSample::SetVolume (float volume)
    {
        if (m_AudioManager->IsEnabled ())
        {
            m_Gain = volume;
            m_VolumeOverriden = true;

            for (size_t i = 0; i < m_SampleInstances.size (); ++i)
            {
                al_set_sample_instance_gain (m_SampleInstances[i], m_Gain);
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    void AudioSample::SetSpeed (float speed)
    {
        if (m_AudioManager->IsEnabled ())
        {
            for (size_t i = 0; i < m_SampleInstances.size (); ++i)
            {
                al_set_sample_instance_speed (m_SampleInstances[i], speed);
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    void AudioSample::CleanUpInstances ()
    {
        for (size_t i = 0; i < m_SampleInstances.size (); ++i)
        {
            if (!al_get_sample_instance_playing (m_SampleInstances[i]))
            {
                al_destroy_sample_instance (m_SampleInstances[i]);
                m_SampleInstances.erase (m_SampleInstances.begin () + i);
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    void AudioSample::SetFadeIn (float milliSeconds)
    {
        m_FadeInMax = milliSeconds;
        m_FadeInCurrent = 0.f;
        m_FadeOutMax = -1.f;
        m_FadeOutCurrent = -1.f;
        m_Gain = 0.f;
    }

    //--------------------------------------------------------------------------------------------------

    void AudioSample::SetFadeOut (float milliSeconds, bool pauseOnFinish)
    {
        m_FadeOutMax = milliSeconds;
        m_FadeOutCurrent = milliSeconds;
        m_FadeInMax = -1.f;
        m_FadeInCurrent = -1.f;
        m_Gain = 1.f;
        m_PauseOnFinish = pauseOnFinish;
    }

    //--------------------------------------------------------------------------------------------------

    std::string AudioSample::GetName () { return m_Name; }

    //--------------------------------------------------------------------------------------------------
}
