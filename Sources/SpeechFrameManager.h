// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __SPEECH_FRAME_MANAGER_H__
#define __SPEECH_FRAME_MANAGER_H__

#include "Common.h"

namespace aga
{
    class SpeechFrame;
    class SceneManager;
    class AudioSample;
    struct SpeechData;

    class SpeechFrameManager : public Lifecycle
    {
    public:
        SpeechFrameManager (SceneManager* sceneManager);
        virtual ~SpeechFrameManager ();

        bool Initialize ();
        bool Destroy ();

        bool ProcessEvent (ALLEGRO_EVENT* event, float deltaTime);

        bool Update (float deltaTime);
        void Render (float deltaTime);
        void Clear ();

        SpeechFrame* AddSpeechFrame (const std::string& id, const std::string& text, Rect rect,
            bool shouldBeHandled = true, const std::string& actionName = "", const std::string& regionName = "");
        SpeechFrame* AddSpeechFrame (const std::string& id, const std::string& text, Point pos, int maxLineCharsCount,
            int linesCount, bool shouldBeHandled = true, const std::string& actionName = "",
            const std::string& regionName = "");

        SpeechFrame* AddSpeechFrame (SpeechData* speechData, bool shouldBeHandled = true);

        SpeechFrame* AddSpeechFrame (const std::string& speechID, Point pos, bool shouldBeHandled = true);
        SpeechFrame* AddSpeechFrame (const std::string& speechID, bool shouldBeHandled = true);
        void RegisterSpeechFinishedHandler (asIScriptFunction* func) { m_SpeechesFinished = func; }

        void RemoveSpeechFrame (const std::string& speechID);

        SceneManager* GetSceneManager ();

        std::map<std::string, SpeechFrame*>& GetSpeechFrames () { return m_Speeches; }

    private:
        Point GetFramePos (
            ScreenRelativePosition position, Point absolutePos, int maxCharsInLine, int maxLines, bool showActor);
        Point GetTextRectSize (int maxLineCharsCount, int linesCount);

    private:
        SceneManager* m_SceneManager;
        asIScriptFunction* m_SpeechesFinished;
        std::map<std::string, SpeechFrame*> m_Speeches;
        AudioSample* m_SelectSample;
    };
}

#endif //   __SPEECH_FRAME_MANAGER_H__
