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

    enum SpeechFramePosition
    {
        Absoulte,
        TopLeft,
        TopCenter,
        TopRight,
        BottomLeft,
        BottomCenter,
        BottomRight,
        Center
    };

    extern std::map<SpeechFramePosition, std::string> g_SpeechFramePosition;

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

        SpeechFrame* AddSpeechFrame (const std::string& id, const std::string& text, Rect rect,
                                     bool shouldBeHandled = true, const std::string& regionName = "");
        SpeechFrame* AddSpeechFrame (const std::string& id, const std::string& text, Point pos, int maxLineCharsCount,
                                     int linesCount, bool shouldBeHandled = true, const std::string& regionName = "");

        SpeechFrame* AddSpeechFrame (SpeechData* speechData, bool shouldBeHandled = true);

        SpeechFrame* AddSpeechFrame (const std::string& speechID, Point pos, bool shouldBeHandled = true);
        SpeechFrame* AddSpeechFrame (const std::string& speechID, bool shouldBeHandled = true);

        void RemoveSpeechFrame (const std::string& speechID);

        SceneManager* GetSceneManager ();

    private:
        Point GetFramePos (SpeechFramePosition position, Point absolutePos, int maxCharsInLine, int maxLines,
                           bool showActor);
        Point GetTextRectSize (int maxLineCharsCount, int linesCount);

    private:
        SceneManager* m_SceneManager;
        std::map<std::string, SpeechFrame*> m_Speeches;
        AudioSample* m_SelectSample;
    };
}

#endif //   __SPEECH_FRAME_MANAGER_H__
