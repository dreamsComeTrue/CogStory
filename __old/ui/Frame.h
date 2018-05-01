// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __FRAME_H__
#define __FRAME_H__

#include "Widget.h"

namespace aga
{
    class Frame : public Widget
    {
    public:
        Frame (UIManager* uiManager, Rect rect, bool filled = false, float thickness = 1.0);
        virtual ~Frame ();
        bool Initialize ();
        bool Destroy ();

        virtual bool Update (double deltaTime);
        virtual void Render (double deltaTime);

    protected:
        Rect m_Rect;
        bool m_Filled;
        float m_Thickness;
    };
}

#endif //   __FRAME_H__
