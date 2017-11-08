// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __SPEECH_FRAME_H__
#define __SPEECH_FRAME_H__

#include "Common.h"

namespace aga
{
    class SpeechFrameManager;

    class SpeechFrame
    {
    public:
        SpeechFrame (SpeechFrameManager* manager);
        SpeechFrame (SpeechFrameManager* manager, const std::string& text, Rect rect);
        virtual ~SpeechFrame ();

        bool Update (float deltaTime);
        void Render (float deltaTime);

        void SetDrawRect (Rect rect);
        void SetVisible (bool visible);
        bool IsVisible () const;

    private:
        SpeechFrameManager* m_Manager;
        NINE_PATCH_BITMAP* m_FrameBitmap;
        std::string m_Text;
        Rect m_DrawRect;
        bool m_Visible;
    };
}

#endif //   __SPEECH_FRAME_H__
