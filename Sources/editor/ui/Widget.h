// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __WIDGET_H__
#define __WIDGET_H__

#include "Common.h"

namespace aga
{
    class Screen;

    class Widget : public Lifecycle
    {
    public:
        Widget (Screen* screen, Point pos);
        virtual ~Widget ();
        bool Initialize ();
        bool Destroy ();

        virtual bool Update (double deltaTime);
        virtual void Render (double deltaTime);

        void SetPosition (int x, int y);
        void SetPosition (Point pos);
        Point GetPosition ();
        Rect GetBounds ();

        void SetBackgroundColor (ALLEGRO_COLOR color);
        void SetBorderColor (ALLEGRO_COLOR color);
        void SetDrawBorder (bool draw);

        virtual void MouseMove (ALLEGRO_MOUSE_EVENT& event);
        virtual void MouseDown (ALLEGRO_MOUSE_EVENT& event);
        virtual void MouseUp (ALLEGRO_MOUSE_EVENT& event);
        virtual void MouseEnter (ALLEGRO_MOUSE_EVENT& event);
        virtual void MouseLeave (ALLEGRO_MOUSE_EVENT& event);

        virtual void KeyboardUp (ALLEGRO_KEYBOARD_EVENT& event);
        virtual void KeyboardDown (ALLEGRO_KEYBOARD_EVENT& event);

    protected:
        Screen* m_Screen;
        Rect m_Bounds;

        bool m_DrawBorder;
        unsigned m_Padding;
        ALLEGRO_COLOR m_BackGroundColor;
        ALLEGRO_COLOR m_BorderColor;
    };
}

#endif //   __WIDGET_H__
