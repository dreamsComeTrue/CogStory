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

        ALLEGRO_FONT* smallFont = al_load_ttf_font (GetResourcePath (FONT_MAIN_SMALL).c_str (), 20, 0);

        if (!smallFont)
        {
            fprintf (stderr, "Could not load %s.\n", GetResourcePath (FONT_MAIN_SMALL).c_str ());
            return false;
        }

        m_Fonts.insert (std::make_pair (FONT_NAME_MAIN_SMALL, smallFont));

        ALLEGRO_FONT* mediumFont = al_load_ttf_font (GetResourcePath (FONT_MAIN_MEDIUM).c_str (), 30, 0);

        if (!mediumFont)
        {
            fprintf (stderr, "Could not load %s.\n", GetResourcePath (FONT_MAIN_MEDIUM).c_str ());
            return false;
        }

        m_Fonts.insert (std::make_pair (FONT_NAME_MAIN_MEDIUM, mediumFont));

        ALLEGRO_FONT* normalFont = al_load_ttf_font (GetResourcePath (FONT_MAIN_NORMAL).c_str (), 40, 0);

        if (!normalFont)
        {
            fprintf (stderr, "Could not load %s.\n", GetResourcePath (FONT_MAIN_NORMAL).c_str ());
            return false;
        }

        m_Fonts.insert (std::make_pair (FONT_NAME_MAIN_NORMAL, normalFont));

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

    void Font::DrawText (const std::string& fontName,
                         ALLEGRO_COLOR color,
                         float x,
                         float y,
                         const std::string& text,
                         int flags)
    {
        ALLEGRO_FONT* font = m_Fonts[fontName];

        al_draw_text (font, color, x, y, flags, text.c_str ());
    }

    //--------------------------------------------------------------------------------------------------

    void Font::DrawMultilineText (const std::string& fontName,
                                  ALLEGRO_COLOR color,
                                  float x,
                                  float y,
                                  float max_width,
                                  float line_height,
                                  int flags,
                                  const char* text,
                                  ...)
    {
        ALLEGRO_FONT* font = m_Fonts[fontName];

        va_list args;
        va_start (args, text);

        al_draw_multiline_textf (font, color, x, y, max_width, line_height, flags, text, args);

        va_end (args);
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

    unsigned Font::GetFontDescent (const std::string& fontName)
    {
        ALLEGRO_FONT* font = m_Fonts[fontName];

        return al_get_font_descent (font);
    }

    //--------------------------------------------------------------------------------------------------
}
