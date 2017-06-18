// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "Widget.h"
#include "Screen.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    Widget::Widget (Screen* screen, Point pos)
        : m_Screen (screen)
        , m_DrawBorder (false)
        , m_BorderColor (COLOR_BLACK)
        , m_BackGroundColor (COLOR_BLACK)
        , m_Padding (2)
    {
        SetPosition (pos);
    }

    //--------------------------------------------------------------------------------------------------

    Widget::~Widget ()
    {
        if (!IsDestroyed ())
        {
            Destroy ();
        }

        Lifecycle::Destroy ();
    }

    //--------------------------------------------------------------------------------------------------

    bool Widget::Initialize () { Lifecycle::Initialize (); }

    //--------------------------------------------------------------------------------------------------

    bool Widget::Destroy () { Lifecycle::Destroy (); }

    //--------------------------------------------------------------------------------------------------

    bool Widget::Update (double deltaTime) { return true; }

    //--------------------------------------------------------------------------------------------------

    void Widget::Render (double deltaTime)
    {
        int thickness = 1;

        al_draw_filled_rectangle (m_Bounds.TopLeft.X - m_Padding, m_Bounds.TopLeft.Y - m_Padding,
            m_Bounds.BottomRight.X + m_Padding - thickness,
            m_Bounds.BottomRight.Y + m_Padding + thickness, m_BackGroundColor);

        if (m_DrawBorder)
        {
            al_draw_rectangle (m_Bounds.TopLeft.X - m_Padding, m_Bounds.TopLeft.Y - m_Padding,
                m_Bounds.BottomRight.X + m_Padding - thickness,
                m_Bounds.BottomRight.Y + m_Padding + thickness, m_BorderColor, thickness);
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Widget::SetPosition (int x, int y) { m_Bounds.TopLeft = { x, y }; }

    //--------------------------------------------------------------------------------------------------

    void Widget::SetPosition (Point pos) { m_Bounds.TopLeft = pos; }

    //--------------------------------------------------------------------------------------------------

    Point Widget::GetPosition () { return m_Bounds.TopLeft; }

    //--------------------------------------------------------------------------------------------------

    Rect Widget::GetBounds () { return m_Bounds; }

    //--------------------------------------------------------------------------------------------------

    void Widget::SetBackgroundColor (ALLEGRO_COLOR color) { m_BackGroundColor = color; }

    //--------------------------------------------------------------------------------------------------

    void Widget::SetBorderColor (ALLEGRO_COLOR color) { m_BorderColor = color; }

    //--------------------------------------------------------------------------------------------------

    void Widget::SetDrawBorder (bool draw) { m_DrawBorder = draw; }

    //--------------------------------------------------------------------------------------------------

    void Widget::MouseMove (ALLEGRO_MOUSE_EVENT& event) {}

    //--------------------------------------------------------------------------------------------------

    void Widget::MouseDown (ALLEGRO_MOUSE_EVENT& event) {}

    //--------------------------------------------------------------------------------------------------

    void Widget::MouseUp (ALLEGRO_MOUSE_EVENT& event) {}

    //--------------------------------------------------------------------------------------------------

    void Widget::MouseEnter (ALLEGRO_MOUSE_EVENT& event) {}

    //--------------------------------------------------------------------------------------------------

    void Widget::MouseLeave (ALLEGRO_MOUSE_EVENT& event) {}

    //--------------------------------------------------------------------------------------------------

    void Widget::KeyboardUp (ALLEGRO_KEYBOARD_EVENT& event) {}

    //--------------------------------------------------------------------------------------------------

    void Widget::KeyboardDown (ALLEGRO_KEYBOARD_EVENT& event) {}

    //--------------------------------------------------------------------------------------------------
}
