// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __AUDIO_MANAGER_H__
#define __AUDIO_MANAGER_H__

#include "Common.h"

namespace aga
{
    class MainLoop;
    class AudioSample;

    class AudioManager : public Lifecycle
    {
    public:
        AudioManager (MainLoop* mainLoop);
        virtual ~AudioManager ();
        bool Initialize ();
        bool Destroy ();

        AudioSample* LoadSampleFromFile (const std::string& sampleName, const std::string& path);
        AudioSample* GetSample (const std::string& sampleName);
        void RemoveSample (const std::string& sampleName);

        MainLoop* GetMainLoop () { return m_MainLoop; }

        void Update (float deltaTime);

        void SetMasterVolume (float volume = 1.0f) { m_MasterVolume = volume; }
        float GetMasterVolume () const { return m_MasterVolume; }

        void SetEnabled (bool enabled) { m_Enabled = enabled; }
        bool IsEnabled () { return m_Enabled; }

    private:
        MainLoop* m_MainLoop;
        float m_MasterVolume;
        bool m_Enabled;
        std::map<std::string, AudioSample*> m_Samples;
    };
}

#endif //   __AUDIO_MANAGER_H__
