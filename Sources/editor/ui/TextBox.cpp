// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "TextBox.h"
#include "Screen.h"
#include "UIManager.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    TextBox::TextBox (UIManager* uiManager, Point pos, const std::string& text)
      : Label (uiManager, pos, text)
      , m_CursorPos (0)
      , m_CalculatedCursorPos (0)
      , m_CursorTimer (nullptr)
      , m_DrawCursor (true)
    {
        SetBackgroundColor (COLOR_GRAY);
        SetTextColor (COLOR_WHITE);
        // SetHighlightColor ({ 0.2f, 0.2f, 0.2f, 1.0f });
        SetBorderColor (COLOR_BLACK);
        SetDrawBorder (true);

        if (text == "")
        {
            SetSize (50, 20);
        }
        else
        {
            SetLength (m_TextSize.Width * 1.5f);
        }
    }

    //--------------------------------------------------------------------------------------------------

    TextBox::~TextBox ()
    {
        if (!IsDestroyed ())
        {
            Destroy ();
        }

        Lifecycle::Destroy ();
    }

    //--------------------------------------------------------------------------------------------------

    bool TextBox::Initialize ()
    {
        Lifecycle::Initialize ();

        m_CursorTimer = al_create_timer (1.0 / 3.0);

        al_start_timer (m_CursorTimer);

        al_register_event_source (m_UIManager->GetScreen ()->GetEventQueue (), al_get_timer_event_source (m_CursorTimer));

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    bool TextBox::Destroy ()
    {
        al_destroy_timer (m_CursorTimer);

        return Lifecycle::Destroy ();
    }

    //--------------------------------------------------------------------------------------------------

    void TextBox::SetLength (unsigned pixelsWidth) { SetSize (pixelsWidth, GetSize ().Height); }

    //--------------------------------------------------------------------------------------------------

    void TextBox::SetCursorPosition (unsigned cursorPos)
    {
        m_CursorPos = cursorPos;
        m_CalculatedCursorPos =
          m_UIManager->GetScreen ()->GetFont ().GetTextDimensions (FONT_NAME_MAIN, m_Text.substr (0, m_CursorPos)).Width;
    }

    //--------------------------------------------------------------------------------------------------

    bool TextBox::Update (double deltaTime) { return true; }

    //--------------------------------------------------------------------------------------------------

    void TextBox::Render (double deltaTime)
    {
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
                                      m_BackGroundColor);

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
                                      m_BackGroundColor);
        }

        if (HasFocus () && m_DrawCursor)
        {
            float linePosX = m_Bounds.TopLeft.X + m_CalculatedCursorPos - 1;
            float linePosY = m_Bounds.TopLeft.Y - 1;
            float lineWidth = m_FontAscent + 3;

            al_draw_line (linePosX, linePosY, linePosX, linePosY + lineWidth, COLOR_WHITE, 2);
        }

        m_UIManager->GetScreen ()->GetFont ().DrawText (
          FONT_NAME_MAIN, m_TextColor, m_Bounds.TopLeft.X, m_Bounds.TopLeft.Y, m_Text, ALLEGRO_ALIGN_LEFT);
    }

    //--------------------------------------------------------------------------------------------------

    void TextBox::MouseDown (ALLEGRO_MOUSE_EVENT& event)
    {
        int clickPos = event.x - GetPosition ().X;

        int i = 1;
        int sumLength = 0;
        for (; i < m_Text.size (); ++i)
        {
            int calcLength = m_UIManager->GetScreen ()->GetFont ().GetTextDimensions (FONT_NAME_MAIN, m_Text.substr (i - 1, i)).Width;

            if (clickPos <= calcLength * 0.5 + sumLength)
            {
                break;
            }

            sumLength += calcLength;
        }

        SetCursorPosition (i - 1);
    }

    //--------------------------------------------------------------------------------------------------

    void TextBox::KeyboardUp (ALLEGRO_KEYBOARD_EVENT& event)
    {
        if (event.keycode == ALLEGRO_KEY_RIGHT)
        {
            if (m_CursorPos < m_Text.size ())
            {
                SetCursorPosition (m_CursorPos + 1);
            }
        }
        else if (event.keycode == ALLEGRO_KEY_LEFT)
        {
            if (m_CursorPos > 0)
            {
                SetCursorPosition (m_CursorPos - 1);
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    void TextBox::KeyboardDown (ALLEGRO_KEYBOARD_EVENT& event) {}

    //--------------------------------------------------------------------------------------------------

    void TextBox::Timer (ALLEGRO_TIMER_EVENT& event)
    {
        if (HasFocus () && event.source == m_CursorTimer)
        {
            m_DrawCursor = !m_DrawCursor;
        }
    }

    //--------------------------------------------------------------------------------------------------
}
