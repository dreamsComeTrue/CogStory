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

        void SetDrawTextCenter (bool center);
        bool IsDrawTextCenter () const;

        void SetDrawSpeed (float speedInMs);

    private:
        SpeechFrameManager* m_Manager;
        NINE_PATCH_BITMAP* m_FrameBitmap;
        std::string m_Text;
        Rect m_DrawRect;
        bool m_Visible;
        bool m_DrawTextCenter;

        bool m_StillUpdating;

        float m_DrawSpeed;
        float m_CurrentDrawTime;
        int m_CurrentIndex;
    };
}

#endif //   __SPEECH_FRAME_H__
