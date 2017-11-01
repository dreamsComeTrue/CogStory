// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "Font.h"
#include "Screen.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    Font::Font () {}

    //--------------------------------------------------------------------------------------------------

    Font::~Font ()
    {
        if (!IsDestroyed ())
        {
            Destroy ();
        }
    }

    //--------------------------------------------------------------------------------------------------

    bool Font::Initialize ()
    {
        Lifecycle::Initialize ();

        ALLEGRO_FONT* mainFont = al_load_ttf_font (GetResourcePath (FONT_MAIN).c_str (), 60, 0);

        if (!mainFont)
        {
            fprintf (stderr, "Could not load %s.\n", GetResourcePath (FONT_MAIN).c_str ());
            return false;
        }

        m_Fonts.insert (std::make_pair (FONT_NAME_MAIN, mainFont));

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    bool Font::Destroy ()
    {
        for (std::map<std::string, ALLEGRO_FONT*>::iterator it = m_Fonts.begin (); it != m_Fonts.end (); ++it)
        {
            al_destroy_font (it->second);
        }

        return Lifecycle::Destroy ();
    }

    //--------------------------------------------------------------------------------------------------

    void Font::DrawText (const std::string& fontName, ALLEGRO_COLOR color, float x, float y, const std::string& text, int flags)
    {
        ALLEGRO_FONT* font = m_Fonts[fontName];

        al_draw_text (font, color, x, y, flags, text.c_str ());
    }

    //--------------------------------------------------------------------------------------------------

    Point Font::GetTextDimensions (const std::string& fontName, const std::string& text)
    {
        int x, y, width, height;
        ALLEGRO_FONT* font = m_Fonts[fontName];

        al_get_text_dimensions (font, text.c_str (), &x, &y, &width, &height);

        return Point{ width, height };
    }

    //--------------------------------------------------------------------------------------------------

    unsigned Font::GetFontAscent (const std::string& fontName)
    {
        ALLEGRO_FONT* font = m_Fonts[fontName];

        return al_get_font_ascent (font);
    }

    //--------------------------------------------------------------------------------------------------
}
