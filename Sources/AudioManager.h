// Copyright 2017-2019 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __AUDIO_MANAGER_H__
#define __AUDIO_MANAGER_H__

#include "Common.h"

namespace aga
{
    class MainLoop;
    class AudioSample;
    class AudioStream;

    class AudioManager : public Lifecycle
    {
    public:
        AudioManager (MainLoop* mainLoop);
        virtual ~AudioManager ();
        bool Initialize ();
        bool Destroy ();

        AudioSample* LoadSampleFromFile (const std::string& sampleName, const std::string& path);
        AudioStream* LoadStreamFromFile (const std::string& streamName, const std::string& path);
        AudioSample* GetSample (const std::string& sampleName);
        void RemoveSample (const std::string& sampleName);

        AudioStream* GetStream (const std::string& streamName);
        void RemoveStream (const std::string& streamName);
        void ClearAudioStreams ();

        void Update (float deltaTime);

        void SetMasterVolume (float volume = 1.0f);
        float GetMasterVolume () const;
        
        void SetEnabled (bool enabled);
        bool IsEnabled ();        

        void SetSamplesEnabled (bool enabled);
        bool IsSamplesEnabled ();

        void SetStreamsEnabled (bool enabled);
        bool IsStreamsEnabled ();

        void Pause ();
        void Resume ();

        bool IsPaused () const;
        void ClearLastPlayedStreams ();

        bool IsGloballyPlaying (const std::string& name);

    private:
        MainLoop* m_MainLoop;
        float m_MasterVolume;
        bool m_StreamsEnabled;
        bool m_SamplesEnabled;
        bool m_IsPaused;
        std::map<std::string, AudioSample*> m_Samples;
        std::map<std::string, AudioStream*> m_Streams;

        std::vector<AudioStream*> m_LastPlayedStreams;
    };
}

#endif //   __AUDIO_MANAGER_H__
