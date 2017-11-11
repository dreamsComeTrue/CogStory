// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __AUDIO_SAMPLE_H__
#define __AUDIO_SAMPLE_H__

#include "Common.h"

namespace aga
{
    class AudioManager;

    class AudioSample : public Lifecycle
    {
    public:
        AudioSample (AudioManager* manager, const std::string& name, const std::string& path);
        virtual ~AudioSample ();

        bool Initialize ();
        bool Destroy ();

        void Play ();

    private:
        AudioManager* m_AudioManager;
        std::string m_Name;
        std::string m_FilePath;
        ALLEGRO_SAMPLE* m_Sample;
    };
}

#endif //   __SCRIPT_H__
