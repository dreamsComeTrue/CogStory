// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __BUTTON_H__
#define __BUTTON_H__

#include "Label.h"

namespace aga
{
    class Button : public Label
    {
    public:
        Button (Screen* screen, Point pos, const std::string& text = "");
        virtual ~Button ();
        bool Initialize ();
        bool Destroy ();

        virtual bool Update (double deltaTime);
        virtual void Render (double deltaTime);

        void SetHighlightColor (ALLEGRO_COLOR color);
        void SetPressedColor (ALLEGRO_COLOR color);

        void MouseMove (ALLEGRO_MOUSE_EVENT& event);
        void MouseDown (ALLEGRO_MOUSE_EVENT& event);
        void MouseUp (ALLEGRO_MOUSE_EVENT& event);
        void MouseEnter (ALLEGRO_MOUSE_EVENT& event);
        void MouseLeave (ALLEGRO_MOUSE_EVENT& event);

        void KeyboardUp (ALLEGRO_KEYBOARD_EVENT& event);
        void KeyboardDown (ALLEGRO_KEYBOARD_EVENT& event);

    protected:
        bool m_IsHighlight;
        ALLEGRO_COLOR m_HighlightColor;

        bool m_IsPressed;
        ALLEGRO_COLOR m_PressedColor;
    };
}

#endif //   __BUTTON_H__
