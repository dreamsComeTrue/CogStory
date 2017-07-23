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

    bool Widget::Update (double deltaTime) { return true; }

    //--------------------------------------------------------------------------------------------------

    void Widget::Render (double deltaTime) {}

    //--------------------------------------------------------------------------------------------------

    void Widget::SetPosition (int x, int y) { SetPosition ({ x, y }); }

    //--------------------------------------------------------------------------------------------------

    void Widget::SetPosition (Point pos) { m_Bounds.TopLeft = pos; }

    //--------------------------------------------------------------------------------------------------

    void Widget::SetSize (int width, int height) { SetSize ({ width, height }); }

    //--------------------------------------------------------------------------------------------------

    void Widget::SetSize (Point size) { m_Bounds.BottomRight = { m_Bounds.TopLeft.X + size.Width, m_Bounds.TopLeft.Y + size.Height }; }

    //--------------------------------------------------------------------------------------------------

    Point Widget::GetSize () { return { m_Bounds.BottomRight.X - m_Bounds.TopLeft.X, m_Bounds.BottomRight.Y - m_Bounds.TopLeft.Y }; }

    //--------------------------------------------------------------------------------------------------

    Point Widget::GetPosition () { return m_Bounds.TopLeft; }

    //--------------------------------------------------------------------------------------------------

    Rect Widget::GetBounds () { return m_Bounds; }

    //--------------------------------------------------------------------------------------------------

    bool Widget::IsVisible () const { return m_Visible; }

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
