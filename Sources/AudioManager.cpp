// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "AudioManager.h"
#include "AudioSample.h"
#include "AudioStream.h"
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

        for (std::map<std::string, AudioStream*>::iterator it = m_Streams.begin (); it != m_Streams.end (); ++it)
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
            std::string finalPath = GetDataPath () + path;
            AudioSample* sample = new AudioSample (this, sampleName, finalPath);
            sample->Initialize ();

            m_Samples.insert (std::make_pair (sampleName, sample));
        }

        return m_Samples[sampleName];
    }

    //--------------------------------------------------------------------------------------------------

    AudioStream* AudioManager::LoadStreamFromFile (const std::string& streamName, const std::string& path)
    {
        if (m_Streams.find (streamName) == m_Streams.end ())
        {
            std::string finalPath = GetDataPath () + path;
            AudioStream* stream = new AudioStream (this, streamName, finalPath);
            stream->Initialize ();

            m_Streams.insert (std::make_pair (streamName, stream));
        }

        return m_Streams[streamName];
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

    void AudioManager::RemoveSample (const std::string& sampleName)
    {
        std::map<std::string, AudioSample*>::iterator samplePos = m_Samples.find (sampleName);

        if (samplePos != m_Samples.end ())
        {
            SAFE_DELETE (samplePos->second);
            m_Samples.erase (sampleName);
        }
    }

    //--------------------------------------------------------------------------------------------------

    AudioStream* AudioManager::GetStream (const std::string& streamName)
    {
        if (m_Streams.find (streamName) != m_Streams.end ())
        {
            return m_Streams[streamName];
        }

        return nullptr;
    }

    //--------------------------------------------------------------------------------------------------

    void AudioManager::RemoveStream (const std::string& streamName)
    {
        std::map<std::string, AudioStream*>::iterator streamPos = m_Streams.find (streamName);

        if (streamPos != m_Streams.end ())
        {
            SAFE_DELETE (streamPos->second);
            m_Streams.erase (streamName);
        }
    }

    //--------------------------------------------------------------------------------------------------

    void AudioManager::Update (float deltaTime)
    {
        if (m_Enabled)
        {
            for (std::map<std::string, AudioSample*>::iterator it = m_Samples.begin (); it != m_Samples.end (); ++it)
            {
                it->second->Update (deltaTime);
            }

            for (std::map<std::string, AudioStream*>::iterator it = m_Streams.begin (); it != m_Streams.end (); ++it)
            {
                it->second->Update (deltaTime);
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    void AudioManager::SetEnabled (bool enabled)
    {
        m_Enabled = enabled;

        if (!enabled)
        {
            for (std::map<std::string, AudioSample*>::iterator it = m_Samples.begin (); it != m_Samples.end (); ++it)
            {
                it->second->Stop ();
            }

            for (std::map<std::string, AudioStream*>::iterator it = m_Streams.begin (); it != m_Streams.end (); ++it)
            {
                it->second->Stop ();
            }
        }
    }

    //--------------------------------------------------------------------------------------------------
}
