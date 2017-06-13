// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "Font.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    Font::Font ()
    {
    }

    //--------------------------------------------------------------------------------------------------

    Font::~Font ()
    {
        if (!IsDestroyed ())
        {
            Destroy ();
        }

        Lifecycle::Destroy ();
    }

    //--------------------------------------------------------------------------------------------------

    bool Font::Initialize ()
    {
        ALLEGRO_FONT* mainFont = al_load_ttf_font (GetResourcePath (FONT_MAIN).c_str (), 32, 0);

        if (!mainFont)
        {
            fprintf (stderr, "Could not load %s.\n", GetResourcePath (FONT_MAIN).c_str ());
            return false;
        }

        m_Fonts.insert (std::make_pair (FONT_NAME_MAIN, mainFont));

        Lifecycle::Initialize ();
    }

    //--------------------------------------------------------------------------------------------------

    bool Font::Destroy ()
    {
        Lifecycle::Destroy ();
    }

    //--------------------------------------------------------------------------------------------------

    void Font::DrawText (const std::string& fontName, ALLEGRO_COLOR color, float x, float y, const std::string& text)
    {
        ALLEGRO_FONT* font = m_Fonts[fontName];

        al_draw_text (font, color, x, y, ALLEGRO_ALIGN_CENTRE, text.c_str ());
    }

    //--------------------------------------------------------------------------------------------------
}
