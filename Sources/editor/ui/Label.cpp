// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "Label.h"
#include "Screen.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    Label::Label (Screen* screen, Point pos, const std::string& text)
        : Widget (screen, pos)
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

        m_TextSize = m_Screen->GetFont ().GetTextDimensions (FONT_NAME_MAIN, m_Text);
        m_FontAscent = m_Screen->GetFont ().GetFontAscent (FONT_NAME_MAIN);

        SetSize (m_TextSize.Width - 1, m_FontAscent + 1);
    }

    //--------------------------------------------------------------------------------------------------

    std::string Label::GetText () const { return m_Text; }

    //--------------------------------------------------------------------------------------------------

    void Label::SetTextColor (ALLEGRO_COLOR color) { m_TextColor = color; }

    //--------------------------------------------------------------------------------------------------

    void Label::Render (double deltaTime)
    {
        if (m_DrawBorder)
        {
            int halfBorder = m_Border / 2;
            al_draw_filled_rectangle (m_Bounds.TopLeft.X - m_Padding - halfBorder,
                m_Bounds.TopLeft.Y - m_Padding - halfBorder, m_Bounds.BottomRight.Width + halfBorder,
                m_Bounds.BottomRight.Height + halfBorder, m_BackGroundColor);

            al_draw_rectangle (m_Bounds.TopLeft.X - halfBorder - m_Padding, m_Bounds.TopLeft.Y - halfBorder - m_Padding,
                m_Bounds.BottomRight.Width + halfBorder - 1 + m_Padding,
                m_Bounds.BottomRight.Height + halfBorder - 1 + m_Padding, m_BorderColor, m_Border);
        }
        else
        {
            al_draw_filled_rectangle (m_Bounds.TopLeft.X - m_Padding, m_Bounds.TopLeft.Y - m_Padding,
                m_Bounds.BottomRight.Width, m_Bounds.BottomRight.Height + m_Padding, m_BackGroundColor);
        }

        m_Screen->GetFont ().DrawText (
            FONT_NAME_MAIN, m_TextColor, m_Bounds.TopLeft.X, m_Bounds.TopLeft.Y, m_Text, ALLEGRO_ALIGN_LEFT);
    }

    //--------------------------------------------------------------------------------------------------
}
