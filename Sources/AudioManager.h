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

        MainLoop* GetMainLoop ();

    private:
        MainLoop* m_MainLoop;
        std::map<std::string, AudioSample*> m_Samples;
    };
}

#endif //   __AUDIO_MANAGER_H__
