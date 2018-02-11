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
            float volume = m_Gain;
            float masterVolume = m_AudioManager->GetMasterVolume ();

            if (!AreSame (masterVolume, 1.0))
            {
                volume = masterVolume;
            }

            al_play_sample (m_Sample, volume, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
        }
    }

    //--------------------------------------------------------------------------------------------------

    void AudioSample::SetVolume (float volume) { m_Gain = volume; }

    //--------------------------------------------------------------------------------------------------
}
