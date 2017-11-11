// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "AudioSample.h"

namespace aga
{
    //---------------------------------------------------------------------------

    AudioSample::AudioSample (AudioManager* manager, const std::string& name, const std::string& path)
      : m_AudioManager (manager)
      , m_Name (name)
      , m_FilePath (path)
      , m_Sample (nullptr)
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
        if (m_Sample)
        {
            al_play_sample (m_Sample, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
        }
    }

    //--------------------------------------------------------------------------------------------------
}
