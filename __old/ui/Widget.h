// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __WIDGET_H__
#define __WIDGET_H__

#include "Common.h"

namespace aga
{
    class UIManager;

    class Widget : public Lifecycle
    {
    public:
        Widget (UIManager* uiManager, Point pos);
        virtual ~Widget ();
        bool Initialize ();
        bool Destroy ();

        virtual bool Update (double deltaTime) = 0;
        virtual void Render (double deltaTime) = 0;

        virtual void SetPosition (int x, int y);
        virtual void SetPosition (Point pos);
        Point GetPosition ();

        virtual void SetSize (int width, int height);
        virtual void SetSize (Point size);
        Point GetSize (bool withOutsets = true);

        UIManager* GetUIManager ();

        Rect GetBounds (bool withOutsets = true);

        void SetPadding (unsigned border);
        unsigned GetPadding () const;

        void SetVisible (bool visible);
        bool IsVisible () const;

        void SetBackgroundColor (ALLEGRO_COLOR color);
        void SetBorderColor (ALLEGRO_COLOR color);
        void SetDrawBorder (bool draw);

        void SetBorder (unsigned border);
        unsigned GetBorder () const;

        bool HasFocus ();

        virtual void MouseMove (ALLEGRO_MOUSE_EVENT& event);
        virtual void MouseDown (ALLEGRO_MOUSE_EVENT& event);
        virtual void MouseUp (ALLEGRO_MOUSE_EVENT& event);
        virtual void MouseEnter (ALLEGRO_MOUSE_EVENT& event);
        virtual void MouseLeave (ALLEGRO_MOUSE_EVENT& event);

        virtual void KeyboardUp (ALLEGRO_KEYBOARD_EVENT& event);
        virtual void KeyboardDown (ALLEGRO_KEYBOARD_EVENT& event);

        virtual void Timer (ALLEGRO_TIMER_EVENT& event);

    protected:
        UIManager* m_UIManager;
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
