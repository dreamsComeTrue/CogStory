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
        void SetVolume (float volume = 1.0f);

        void Update (float deltaTime);

        void SetLooping (bool looping) { m_Looping = looping; }
        bool IsLooping () const { return m_Looping; }

        void SetFadeIn (float milliSeconds);
        void SetFadeOut (float milliSeconds);

    private:
        void CleanUpInstances ();

    private:
        AudioManager* m_AudioManager;
        std::string m_Name;
        std::string m_FilePath;
        ALLEGRO_SAMPLE* m_Sample;
        std::vector<ALLEGRO_SAMPLE_INSTANCE*> m_SampleInstances;

        float m_Gain;
        bool m_Looping;

        float m_FadeInCurrent, m_FadeInMax;
        float m_FadeOutCurrent, m_FadeOutMax;
    };
}

#endif //   __SCRIPT_H__
