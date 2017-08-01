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

        virtual bool Update (double deltaTime) = 0;
        virtual void Render (double deltaTime) = 0;

        void SetPosition (int x, int y);
        void SetPosition (Point pos);
        Point GetPosition ();

        virtual void SetSize (int width, int height);
        virtual void SetSize (Point size);
        Point GetSize ();

        Screen* GetScreen ();

        Rect GetBounds (bool withOutsets = true);

        void SetPadding(unsigned border);
        unsigned GetPadding() const;

        bool IsVisible () const;

        void SetBackgroundColor (ALLEGRO_COLOR color);
        void SetBorderColor (ALLEGRO_COLOR color);
        void SetDrawBorder (bool draw);

        void SetBorder(unsigned border);
        unsigned GetBorder() const;

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

        unsigned m_Border;
        bool m_DrawBorder;
        unsigned m_Padding;
        ALLEGRO_COLOR m_BackGroundColor;
        ALLEGRO_COLOR m_BorderColor;

        bool m_Visible;
    };
}

#endif //   __WIDGET_H__
