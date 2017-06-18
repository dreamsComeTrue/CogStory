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

    bool Label::Update (double deltaTime)
    {
        Widget::Update (deltaTime);
        return true;
    }

    //--------------------------------------------------------------------------------------------------

    void Label::SetText (const std::string& text)
    {
        m_Text = text;

        m_TextSize = m_Screen->GetFont ().GetTextDimensions (FONT_NAME_MAIN, m_Text);
        m_FontAscent = m_Screen->GetFont ().GetFontAscent (FONT_NAME_MAIN);

        m_Bounds.BottomRight
            = { m_Bounds.TopLeft.X + m_TextSize.Width, m_Bounds.TopLeft.Y + m_FontAscent };
    }

    //--------------------------------------------------------------------------------------------------

    void Label::SetTextColor (ALLEGRO_COLOR color) { m_TextColor = color; }

    //--------------------------------------------------------------------------------------------------

    void Label::Render (double deltaTime)
    {
        Widget::Render (deltaTime);

        m_Screen->GetFont ().DrawText (FONT_NAME_MAIN, m_TextColor, m_Bounds.TopLeft.X,
            m_Bounds.TopLeft.Y, m_Text, ALLEGRO_ALIGN_LEFT);
    }

    //--------------------------------------------------------------------------------------------------
}
