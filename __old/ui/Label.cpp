// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "Label.h"
#include "Screen.h"
#include "UIManager.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    Label::Label (UIManager* uiManager, Point pos, const std::string& text)
      : Widget (uiManager, pos)
      , m_TextColor (COLOR_WHITE)
    {
        SetText (text);
    }

    //--------------------------------------------------------------------------------------------------

    Label::~Label ()
    {
        if (!IsDestroyed ())
        {
            Destroy ();
        }

        Lifecycle::Destroy ();
    }

    //--------------------------------------------------------------------------------------------------

    bool Label::Initialize () { Lifecycle::Initialize (); }

    //--------------------------------------------------------------------------------------------------

    bool Label::Destroy () { Lifecycle::Destroy (); }

    //--------------------------------------------------------------------------------------------------

    bool Label::Update (double deltaTime) { return true; }

    //--------------------------------------------------------------------------------------------------

    void Label::SetText (const std::string& text)
    {
        m_Text = text;

        m_TextSize = m_UIManager->GetScreen ()->GetFont ().GetTextDimensions (FONT_NAME_MAIN, m_Text);
        m_FontAscent = m_UIManager->GetScreen ()->GetFont ().GetFontAscent (FONT_NAME_MAIN);

        SetSize (m_TextSize.Width - 1, m_FontAscent + 1);
    }

    //--------------------------------------------------------------------------------------------------

    std::string Label::GetText () const { return m_Text; }

    //--------------------------------------------------------------------------------------------------

    void Label::SetTextColor (ALLEGRO_COLOR color) { m_TextColor = color; }

    //--------------------------------------------------------------------------------------------------

    void Label::Render (double deltaTime)
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

        m_UIManager->GetScreen ()->GetFont ().DrawText (
          FONT_NAME_MAIN, m_TextColor, m_Bounds.TopLeft.X, m_Bounds.TopLeft.Y, m_Text, ALLEGRO_ALIGN_LEFT);
    }

    //--------------------------------------------------------------------------------------------------
}
