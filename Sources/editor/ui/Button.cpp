// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "Button.h"
#include "Screen.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    Button::Button (Screen* screen, Point pos, const std::string& text)
      : Label (screen, pos, text)
      , m_IsPressed (false)
      , m_IsHighlight (false)
      , m_PressedColor ({ 0.5, 0.5, 0.5 })
      , m_HighlightColor ({ 0.7, 0.7, 0.7 })
    {
    }

    //--------------------------------------------------------------------------------------------------

    Button::~Button ()
    {
        if (!IsDestroyed ())
        {
            Destroy ();
        }

        Lifecycle::Destroy ();
    }

    //--------------------------------------------------------------------------------------------------

    bool Button::Initialize () { Lifecycle::Initialize (); }

    //--------------------------------------------------------------------------------------------------

    bool Button::Destroy () { Lifecycle::Destroy (); }

    //--------------------------------------------------------------------------------------------------

    bool Button::Update (double deltaTime)
    {
        Label::Update (deltaTime);

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    void Button::Render (double deltaTime)
    {
        int thickness = 1;

        ALLEGRO_COLOR color =
          m_IsPressed ? m_PressedColor : m_IsHighlight ? m_HighlightColor : m_BackGroundColor;

        al_draw_filled_rectangle (m_Bounds.TopLeft.X - m_Padding,
                                  m_Bounds.TopLeft.Y - m_Padding,
                                  m_Bounds.BottomRight.X + m_Padding - thickness,
                                  m_Bounds.BottomRight.Y + m_Padding + thickness,
                                  color);

        if (m_DrawBorder)
        {
            al_draw_rectangle (m_Bounds.TopLeft.X - m_Padding,
                               m_Bounds.TopLeft.Y - m_Padding,
                               m_Bounds.BottomRight.X + m_Padding - thickness,
                               m_Bounds.BottomRight.Y + m_Padding + thickness,
                               m_BorderColor,
                               thickness);
        }

        m_Screen->GetFont ().DrawText (
          FONT_NAME_MAIN, m_TextColor, m_Bounds.TopLeft.X, m_Bounds.TopLeft.Y, m_Text, ALLEGRO_ALIGN_LEFT);
    }

    //--------------------------------------------------------------------------------------------------

    void Button::SetHighlightColor (ALLEGRO_COLOR color) { m_HighlightColor = color; }

    //--------------------------------------------------------------------------------------------------

    void Button::SetPressedColor (ALLEGRO_COLOR color) { m_PressedColor = color; }

    //--------------------------------------------------------------------------------------------------

    void Button::MouseMove (ALLEGRO_MOUSE_EVENT& event) {}

    //--------------------------------------------------------------------------------------------------

    void Button::MouseDown (ALLEGRO_MOUSE_EVENT& event) { m_IsPressed = true; }

    //--------------------------------------------------------------------------------------------------

    void Button::MouseUp (ALLEGRO_MOUSE_EVENT& event) { m_IsPressed = false; }

    //--------------------------------------------------------------------------------------------------

    void Button::MouseEnter (ALLEGRO_MOUSE_EVENT& event) { m_IsHighlight = true; }

    //--------------------------------------------------------------------------------------------------

    void Button::MouseLeave (ALLEGRO_MOUSE_EVENT& event) { m_IsHighlight = false; }

    //--------------------------------------------------------------------------------------------------

    void Button::KeyboardUp (ALLEGRO_KEYBOARD_EVENT& event) {}

    //--------------------------------------------------------------------------------------------------

    void Button::KeyboardDown (ALLEGRO_KEYBOARD_EVENT& event) {}

    //--------------------------------------------------------------------------------------------------
}
