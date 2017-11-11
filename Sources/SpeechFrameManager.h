// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __SPEECH_FRAME_MANAGER_H__
#define __SPEECH_FRAME_MANAGER_H__

#include "Common.h"

namespace aga
{
    class SpeechFrame;
    class SceneManager;

    class SpeechFrameManager : public Lifecycle
    {
    public:
        SpeechFrameManager (SceneManager* sceneManager);
        virtual ~SpeechFrameManager ();

        bool Initialize ();
        bool Destroy ();

        void ProcessEvent (ALLEGRO_EVENT* event, float deltaTime);

        bool Update (float deltaTime);
        void Render (float deltaTime);
        void Clear ();

        SpeechFrame* AddSpeechFrame (const std::string& id, const std::string& text, Rect rect);
        SpeechFrame* AddMultiLineSpeechFrame (const std::string& id, const std::string& text, Rect rect);

        SceneManager* GetSceneManager ();

    private:
        SceneManager* m_SceneManager;
        std::map<std::string, SpeechFrame*> m_Frames;
    };
}

#endif //   __SPEECH_FRAME_MANAGER_H__
