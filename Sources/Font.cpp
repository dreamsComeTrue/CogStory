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

        GenerateFont (FONT_NAME_SMALL, GetResourcePath (FONT_SMALL), 20);
        GenerateFont (FONT_NAME_MEDIUM, GetResourcePath (FONT_MEDIUM), 30);
        GenerateFont (FONT_NAME_NORMAL, GetResourcePath (FONT_NORMAL), 40);
        GenerateFont (FONT_NAME_SPEECH_FRAME, GetResourcePath (FONT_SPEECH_FRAME), 50);

        GenerateFont (FONT_NAME_MENU_TITLE, GetResourcePath (FONT_MENU_TITLE), 120);
        GenerateFont (FONT_NAME_MENU_ITEM_NORMAL, GetResourcePath (FONT_MENU_ITEM_NORMAL), 70);
        GenerateFont (FONT_NAME_MENU_ITEM_SMALL, GetResourcePath (FONT_MENU_ITEM_SMALL), 50);

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    void Font::GenerateFont (const std::string& name, const std::string& path, float size)
    {
        ALLEGRO_FONT* font = al_load_ttf_font (path.c_str (), size, 0);

        if (!font)
        {
            fprintf (stderr, "Could not load %s.\n", path.c_str ());
            return;
        }

        m_Fonts.insert (std::make_pair (name, font));
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

    void Font::DrawText (const std::string& fontName, ALLEGRO_COLOR color, float x, float y, const std::string& text,
                         int flags)
    {
        ALLEGRO_FONT* font = m_Fonts[fontName];

        al_draw_text (font, color, x, y, flags, text.c_str ());
    }

    //--------------------------------------------------------------------------------------------------

    void Font::DrawMultilineText (const std::string& fontName, ALLEGRO_COLOR color, float x, float y, float max_width,
                                  float line_height, int flags, const char* text, ...)
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
