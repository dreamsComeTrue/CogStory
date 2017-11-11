// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "AudioManager.h"
#include "AudioSample.h"
#include "MainLoop.h"

namespace aga
{
    //---------------------------------------------------------------------------

    AudioManager::AudioManager (MainLoop* mainLoop)
      : m_MainLoop (mainLoop)
    {
    }

    //--------------------------------------------------------------------------------------------------

    AudioManager::~AudioManager ()
    {
        if (!IsDestroyed ())
        {
            Destroy ();
        }
    }

    //--------------------------------------------------------------------------------------------------

    bool AudioManager::Initialize ()
    {
        Lifecycle::Initialize ();

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    bool AudioManager::Destroy ()
    {
        for (std::map<std::string, AudioSample*>::iterator it = m_Samples.begin (); it != m_Samples.end (); ++it)
        {
            SAFE_DELETE (it->second);
        }

        return Lifecycle::Destroy ();
    }

    //--------------------------------------------------------------------------------------------------

    AudioSample* AudioManager::LoadSampleFromFile (const std::string& sampleName, const std::string& path)
    {
        if (m_Samples.find (sampleName) == m_Samples.end ())
        {
            AudioSample* sample = new AudioSample (this, sampleName, path);
            sample->Initialize ();

            m_Samples.insert (std::make_pair (sampleName, sample));
        }

        return m_Samples[sampleName];
    }

    //--------------------------------------------------------------------------------------------------

    MainLoop* AudioManager::GetMainLoop () { return m_MainLoop; }

    //--------------------------------------------------------------------------------------------------
}
