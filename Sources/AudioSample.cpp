// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "AudioSample.h"
#include "AudioManager.h"

namespace aga
{
    //---------------------------------------------------------------------------

    AudioSample::AudioSample (AudioManager* manager, const std::string& name, const std::string& path)
        : m_AudioManager (manager)
        , m_Name (name)
        , m_FilePath (path)
        , m_Sample (nullptr)
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

    AudioSample::~AudioSample ()
    {
        if (!IsDestroyed ())
        {
            Destroy ();
        }
    }

    //--------------------------------------------------------------------------------------------------

    bool AudioSample::Initialize ()
    {
        Lifecycle::Initialize ();

        m_Sample = al_load_sample (m_FilePath.c_str ());

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    bool AudioSample::Destroy ()
    {
        for (int i = 0; i < m_SampleInstances.size (); ++i)
        {
            al_destroy_sample_instance (m_SampleInstances[i]);
            m_SampleInstances.erase (m_SampleInstances.begin () + i);
        }

        if (m_Sample)
        {
            al_destroy_sample (m_Sample);
        }

        return Lifecycle::Destroy ();
    }

    //--------------------------------------------------------------------------------------------------

    void AudioSample::Play ()
    {
        if (m_Sample && m_AudioManager->IsEnabled ())
        {
            CleanUpInstances ();

            float volume = m_Gain;
            float masterVolume = m_AudioManager->GetMasterVolume ();

            if (!AreSame (masterVolume, 1.0))
            {
                volume = masterVolume;
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
        if (m_Sample && m_AudioManager->IsEnabled ())
        {
            CleanUpInstances ();

            for (int i = 0; i < m_SampleInstances.size (); ++i)
            {
                al_stop_sample_instance (m_SampleInstances[i]);
            }
		}
	}

    //--------------------------------------------------------------------------------------------------

    void AudioSample::Pause ()
    {
        m_CurrentPos = 0;

        for (int i = 0; i < m_SampleInstances.size (); ++i)
        {
            unsigned pos = al_get_sample_instance_position (m_SampleInstances[i]);

            if (pos > m_CurrentPos)
            {
                m_CurrentPos = pos;
            }

            al_set_sample_instance_playing (m_SampleInstances[i], false);
        }
    }

    //--------------------------------------------------------------------------------------------------

    void AudioSample::Resume ()
    {
        for (int i = 0; i < m_SampleInstances.size (); ++i)
        {
            al_set_sample_instance_position (m_SampleInstances[i], m_CurrentPos);
            al_set_sample_instance_playing (m_SampleInstances[i], true);
        }
    }

    //--------------------------------------------------------------------------------------------------

    void AudioSample::Update (float deltaTime)
    {
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
        m_Looping = looping;

        for (int i = 0; i < m_SampleInstances.size (); ++i)
        {
            al_set_sample_instance_playmode (
                m_SampleInstances[i], m_Looping ? ALLEGRO_PLAYMODE_LOOP : ALLEGRO_PLAYMODE_ONCE);
        }
    }

    //--------------------------------------------------------------------------------------------------

    void AudioSample::SetVolume (float volume)
    {
        m_Gain = volume;

        for (int i = 0; i < m_SampleInstances.size (); ++i)
        {
            al_set_sample_instance_gain (m_SampleInstances[i], m_Gain);
        }
    }

    //--------------------------------------------------------------------------------------------------

	void AudioSample::CleanUpInstances ()
	{
        for (int i = 0; i < m_SampleInstances.size (); ++i)
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
}
