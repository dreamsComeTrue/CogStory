// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "Button.h"
#include "Screen.h"
#include "UIManager.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    Button::Button (UIManager* uiManager, Point pos, const std::string& text)
      : Label (uiManager, pos, text)
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

    bool Button::Initialize () { return Lifecycle::Initialize (); }

    //--------------------------------------------------------------------------------------------------

    bool Button::Destroy () { return Lifecycle::Destroy (); }

    //--------------------------------------------------------------------------------------------------

    bool Button::Update (double deltaTime) { return Label::Update (deltaTime); }

    //--------------------------------------------------------------------------------------------------

    void Button::Render (double deltaTime)
    {
        ALLEGRO_COLOR color = m_IsPressed ? m_PressedColor : m_IsHighlight ? m_HighlightColor : m_BackGroundColor;
        int offset = m_Padding;

        if (m_DrawBorder)
        {
            offset += m_Border / 2;

            int delta = 0;
            if (offset % 2)
            {
                delta = 1;
            }

            al_draw_filled_rectangle (m_Bounds.TopLeft.X - offset,
                                      m_Bounds.TopLeft.Y - offset,
                                      m_Bounds.TopLeft.X + m_Bounds.BottomRight.Width + offset - delta,
                                      m_Bounds.TopLeft.Y + m_Bounds.BottomRight.Height + offset - delta,
                                      color);

            al_draw_rectangle (m_Bounds.TopLeft.X - offset,
                               m_Bounds.TopLeft.Y - offset,
                               m_Bounds.TopLeft.X + m_Bounds.BottomRight.Width + offset - delta,
                               m_Bounds.TopLeft.Y + m_Bounds.BottomRight.Height + offset - delta,
                               m_BorderColor,
                               m_Border);
        }
        else
        {
            al_draw_filled_rectangle (m_Bounds.TopLeft.X - offset,
                                      m_Bounds.TopLeft.Y - offset,
                                      m_Bounds.TopLeft.X + m_Bounds.BottomRight.Width + offset,
                                      m_Bounds.TopLeft.Y + m_Bounds.BottomRight.Height + offset,
                                      color);
        }

        m_UIManager->GetScreen ()->GetFont ().DrawText (
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

    void Button::MouseUp (ALLEGRO_MOUSE_EVENT& event)
    {
        m_IsPressed = false;

        if (OnClick != nullptr)
        {
            OnClick ();
        }
    }

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
