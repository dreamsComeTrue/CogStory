// Copyright 2017-2019 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __AUDIO_STREAM_H__
#define __AUDIO_STREAM_H__

#include "Common.h"

namespace aga
{
    class AudioManager;

    class AudioStream : public Lifecycle
    {
    public:
        AudioStream (AudioManager* manager, const std::string& name, const std::string& path);
        virtual ~AudioStream ();

        bool Initialize ();
        bool Destroy ();

        void Play ();
        void Stop ();

        void Pause ();
        void Resume ();

        void SetVolume (float volume = 1.0f);
        void SetSpeed (float speed = 1.0f);

        void Update (float deltaTime);

        void SetLooping (bool looping);
        bool IsLooping () const;

        void SetFadeIn (float milliSeconds);
        void SetFadeOut (float milliSeconds, bool pauseOnFinish = true);

        bool IsPlaying ();

    private:
        AudioManager* m_AudioManager;
        std::string m_Name;
        ALLEGRO_AUDIO_STREAM* m_Stream;

        float m_Gain;
        bool m_VolumeOverriden;
        bool m_Looping;

        float m_FadeInCurrent, m_FadeInMax;
        float m_FadeOutCurrent, m_FadeOutMax;

        double m_CurrentPos;
        bool m_PauseOnFinish;
    };
}

#endif //   __AUDIO_STREAM_H__
