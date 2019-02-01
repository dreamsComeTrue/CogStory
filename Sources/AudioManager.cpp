// Copyright 2017-2019 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "AudioManager.h"
#include "AudioSample.h"
#include "AudioStream.h"

namespace aga
{
    //---------------------------------------------------------------------------

    AudioManager::AudioManager (MainLoop* mainLoop)
        : m_MainLoop (mainLoop)
        , m_MasterVolume (1.0f)
        , m_LastMasterVolume (1.0f)
        , m_Enabled (true)
        , m_IsPaused (false)
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

    bool AudioManager::Initialize () { return Lifecycle::Initialize (); }

    //--------------------------------------------------------------------------------------------------

    bool AudioManager::Destroy ()
    {
        for (std::map<std::string, AudioSample*>::iterator it = m_Samples.begin (); it != m_Samples.end ();)
        {
            SAFE_DELETE (it->second);
            m_Samples.erase (it++);
        }

        for (std::map<std::string, AudioStream*>::iterator it = m_Streams.begin (); it != m_Streams.end ();)
        {
            SAFE_DELETE (it->second);
            m_Streams.erase (it++);
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

    void AudioManager::ClearAudioStreams ()
    {
        for (std::map<std::string, AudioStream*>::iterator it = m_Streams.begin (); it != m_Streams.end ();)
        {
            SAFE_DELETE (it->second);
            m_Streams.erase (it++);
        }
    }

    //--------------------------------------------------------------------------------------------------

    void AudioManager::Pause ()
    {
        m_LastMasterVolume = m_MasterVolume;
        m_MasterVolume = 0.0f;

        for (std::map<std::string, AudioSample*>::iterator it = m_Samples.begin (); it != m_Samples.end (); ++it)
        {
            it->second->Pause ();
        }

        for (std::map<std::string, AudioStream*>::iterator it = m_Streams.begin (); it != m_Streams.end (); ++it)
        {
            it->second->Pause ();
        }

        m_IsPaused = true;
    }

    //--------------------------------------------------------------------------------------------------

    void AudioManager::Resume ()
    {
        m_MasterVolume = m_LastMasterVolume;

        for (std::map<std::string, AudioSample*>::iterator it = m_Samples.begin (); it != m_Samples.end (); ++it)
        {
            it->second->Resume ();
        }

        for (std::map<std::string, AudioStream*>::iterator it = m_Streams.begin (); it != m_Streams.end (); ++it)
        {
            it->second->Resume ();
        }

        m_IsPaused = false;
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

    void AudioManager::SetMasterVolume (float volume) { m_MasterVolume = volume; }

    //--------------------------------------------------------------------------------------------------

    float AudioManager::GetMasterVolume () const { return m_MasterVolume; }

    //--------------------------------------------------------------------------------------------------

    void AudioManager::SetEnabled (bool enabled)
    {
        m_Enabled = enabled;

        if (m_Enabled)
        {
            m_MasterVolume = 1.0f;
        }
        else
        {
            m_LastMasterVolume = m_MasterVolume;
            m_MasterVolume = 0.0f;
        }

        if (enabled)
        {
            for (AudioStream* lastPlayed : m_LastPlayedStreams)
            {
                lastPlayed->Play ();
            }

            ClearLastPlayedStreams ();
        }
        else
        {
            for (std::map<std::string, AudioStream*>::iterator it = m_Streams.begin (); it != m_Streams.end (); ++it)
            {
                if (it->second->IsPlaying ())
                {
                    m_LastPlayedStreams.push_back (it->second);
                    it->second->Stop ();
                }
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    bool AudioManager::IsEnabled () { return m_Enabled; }

    //--------------------------------------------------------------------------------------------------

    bool AudioManager::IsPaused () const { return m_IsPaused; }

    //--------------------------------------------------------------------------------------------------

    void AudioManager::ClearLastPlayedStreams () { m_LastPlayedStreams.clear (); }

    //--------------------------------------------------------------------------------------------------

    bool AudioManager::IsGloballyPlaying (const std::string& name)
    {
        if (m_Samples.find (name) != m_Samples.end ())
        {
            return m_Samples[name]->IsPlaying ();
        }

        return false;
    }

    //--------------------------------------------------------------------------------------------------
}
