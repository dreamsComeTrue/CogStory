// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __TEXTBOX_H__
#define __TEXTBOX_H__

#include "Label.h"

namespace aga
{
    class TextBox : public Label
    {
    public:
        TextBox (UIManager* uiManager, Point pos, const std::string& text);
        virtual ~TextBox ();
        bool Initialize ();
        bool Destroy ();

        void SetLength (unsigned cursorPos);
        void SetCursorPosition (unsigned cursorPos);

        virtual bool Update (double deltaTime) override;
        virtual void Render (double deltaTime) override;

        virtual void MouseDown (ALLEGRO_MOUSE_EVENT& event) override;
        virtual void KeyboardUp (ALLEGRO_KEYBOARD_EVENT& event) override;
        virtual void KeyboardDown (ALLEGRO_KEYBOARD_EVENT& event) override;
        virtual void Timer (ALLEGRO_TIMER_EVENT& event) override;

    protected:
        unsigned m_CursorPos;
        unsigned m_CalculatedCursorPos;
        bool m_DrawCursor;
        ALLEGRO_TIMER* m_CursorTimer;
    };
}

#endif //   __TEXTBOX_H__
