// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "Label.h"
#include "Screen.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    Label::Label (Screen* screen, const std::string& text)
        : Widget (screen)
        , m_DrawBorder (false)
        , m_TextColor (COLOR_WHITE)
        , m_BorderColor (COLOR_BLACK)
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

    void Label::ProcessEvent (ALLEGRO_EVENT* event, double deltaTime) {}

    //--------------------------------------------------------------------------------------------------

    void Label::SetText (const std::string& text)
    {
        m_Text = text;

        m_TextSize = m_Screen->GetFont ().GetTextDimensions (FONT_NAME_MAIN, m_Text);
        m_FontAscent = m_Screen->GetFont ().GetFontAscent (FONT_NAME_MAIN);
    }

    //--------------------------------------------------------------------------------------------------

    void Label::SetTextColor (ALLEGRO_COLOR color) { m_TextColor = color; }

    //--------------------------------------------------------------------------------------------------

    void Label::SetBorderColor (ALLEGRO_COLOR color) { m_BorderColor = color; }

    //--------------------------------------------------------------------------------------------------

    void Label::SetDrawBorder (bool draw) { m_DrawBorder = draw; }

    //--------------------------------------------------------------------------------------------------

    void Label::Render (double deltaTime)
    {
        if (m_DrawBorder)
        {
            int padding = 2;
            int thickness = 1;

            al_draw_rectangle (m_Position.X - padding, m_Position.Y - padding,
                m_Position.X + m_TextSize.Width + padding - thickness,
                m_Position.Y + m_FontAscent + padding + thickness, m_BorderColor, thickness);
        }

        m_Screen->GetFont ().DrawText (
            FONT_NAME_MAIN, m_TextColor, m_Position.X, m_Position.Y, m_Text, ALLEGRO_ALIGN_LEFT);
    }

    //--------------------------------------------------------------------------------------------------
}
