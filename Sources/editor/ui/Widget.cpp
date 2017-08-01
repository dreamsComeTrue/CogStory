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
        , m_Border (1)
        , m_Visible (true)
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

    void Widget::SetPosition (int x, int y) { SetPosition ({ x, y }); }

    //--------------------------------------------------------------------------------------------------

    void Widget::SetPosition (Point pos) { m_Bounds.TopLeft = pos; }

    //--------------------------------------------------------------------------------------------------

    void Widget::SetSize (int width, int height) { SetSize ({ width, height }); }

    //--------------------------------------------------------------------------------------------------

    void Widget::SetSize (Point size) { m_Bounds.BottomRight = size; }

    //--------------------------------------------------------------------------------------------------

    Point Widget::GetSize () { return m_Bounds.BottomRight; }

    //--------------------------------------------------------------------------------------------------

    Screen* Widget::GetScreen () { return m_Screen; }

    //--------------------------------------------------------------------------------------------------

    Point Widget::GetPosition () { return m_Bounds.TopLeft; }

    //--------------------------------------------------------------------------------------------------

    Rect Widget::GetBounds (bool withOutsets)
    {
        if (withOutsets)
        {
            int offset = m_Padding;

            if (m_DrawBorder)
            {
                offset += m_Border;
            }

            return Rect{ Point (m_Bounds.TopLeft.X - offset, m_Bounds.TopLeft.Y - offset),
                Point (m_Bounds.TopLeft.X + m_Bounds.BottomRight.Width + offset,
                    m_Bounds.TopLeft.Y + m_Bounds.BottomRight.Height + offset) };
        }
        else
        {
            return m_Bounds;
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Widget::SetPadding (unsigned padding) { m_Padding = padding; }

    //--------------------------------------------------------------------------------------------------

    unsigned Widget::GetPadding () const { return m_Padding; }

    //--------------------------------------------------------------------------------------------------

    bool Widget::IsVisible () const { return m_Visible; }

    //--------------------------------------------------------------------------------------------------

    void Widget::SetBackgroundColor (ALLEGRO_COLOR color) { m_BackGroundColor = color; }

    //--------------------------------------------------------------------------------------------------

    void Widget::SetBorderColor (ALLEGRO_COLOR color) { m_BorderColor = color; }

    //--------------------------------------------------------------------------------------------------

    void Widget::SetDrawBorder (bool draw) { m_DrawBorder = draw; }

    //--------------------------------------------------------------------------------------------------

    void Widget::SetBorder (unsigned border) { m_Border = border; }

    //--------------------------------------------------------------------------------------------------

    unsigned Widget::GetBorder () const { return m_Border; }

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
