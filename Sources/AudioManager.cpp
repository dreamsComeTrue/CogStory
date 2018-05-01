// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "AudioManager.h"
#include "AudioSample.h"
#include "MainLoop.h"

namespace aga
{
    //---------------------------------------------------------------------------

    AudioManager::AudioManager (MainLoop* mainLoop)
        : m_MainLoop (mainLoop)
        , m_MasterVolume (1.0f)
        , m_Enabled (true)
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

    AudioSample* AudioManager::GetSample (const std::string& sampleName)
    {
        if (m_Samples.find (sampleName) != m_Samples.end ())
        {
            return m_Samples[sampleName];
        }

        return nullptr;
    }

    //--------------------------------------------------------------------------------------------------

    MainLoop* AudioManager::GetMainLoop () { return m_MainLoop; }

    //--------------------------------------------------------------------------------------------------

    void AudioManager::SetMasterVolume (float volume) { m_MasterVolume = volume; }

    //--------------------------------------------------------------------------------------------------

    float AudioManager::GetMasterVolume () const { return m_MasterVolume; }

    //--------------------------------------------------------------------------------------------------

    void AudioManager::SetEnabled (bool enabled) { m_Enabled = enabled; }

    //--------------------------------------------------------------------------------------------------

    bool AudioManager::IsEnabled () { return m_Enabled; }

    //--------------------------------------------------------------------------------------------------
}
