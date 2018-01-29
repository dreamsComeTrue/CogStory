// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __SPEECH_FRAME_MANAGER_H__
#define __SPEECH_FRAME_MANAGER_H__

#include "Common.h"

namespace aga
{
    class SpeechFrame;
    class SceneManager;
    class AudioSample;
    class SpeechData;

    enum SpeechFramePosition
    {
        TopLeft,
        TopCenter,
        TopRight,
        BottomLeft,
        BottomCenter,
        BottomRight,
        Center
    };

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

        SpeechFrame* AddSpeechFrame (SpeechData* speechData, Point pos, int maxLineCharsCount, int linesCount,
                                     bool shouldBeHandled = true, const std::string& regionName = "");

        SpeechFrame* AddSpeechFrame (const std::string& speechID, Point pos, int maxLineCharsCount, int linesCount,
                                     bool shouldBeHandled = true);

        SpeechFrame* AddSpeechFrame (const std::string& speechID, SpeechFramePosition position, int maxLineCharsCount,
                                     int linesCount, bool shouldBeHandled = true);

        SceneManager* GetSceneManager ();

    private:
        Point GetTextRectSize (int maxLineCharsCount, int linesCount);

    private:
        SceneManager* m_SceneManager;
        std::map<std::string, SpeechFrame*> m_Frames;
        AudioSample* m_SelectSample;
    };
}

#endif //   __SPEECH_FRAME_MANAGER_H__
