// Copyright 2017-2019 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "Font.h"
#include "Resources.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    std::map<std::string, ALLEGRO_FONT*> Font::m_Fonts;
    ALLEGRO_TRANSFORM* Font::m_BackupTransform;

    //--------------------------------------------------------------------------------------------------
    //--------------------------------------------------------------------------------------------------

    Font::Font () { m_BackupTransform = nullptr; }

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
        GenerateFont (FONT_NAME_SPEECH_FRAME, GetResourcePath (FONT_SPEECH_FRAME), 65);

        GenerateFont (FONT_NAME_MENU_TITLE, GetResourcePath (FONT_MENU_TITLE), 120);
        GenerateFont (FONT_NAME_MENU_ITEM_NORMAL, GetResourcePath (FONT_MENU_ITEM_NORMAL), 70);
        GenerateFont (FONT_NAME_MENU_ITEM_SMALL, GetResourcePath (FONT_MENU_ITEM_SMALL), 50);

        m_BackupTransform = new ALLEGRO_TRANSFORM;

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    void Font::GenerateFont (const std::string& name, const std::string& path, int size)
    {
        ALLEGRO_FONT* font = al_load_ttf_font (path.c_str (), size, 0);

        if (!font)
        {
            Log ("Could not load %s.\n", path.c_str ());
            return;
        }

        m_Fonts.insert (std::make_pair (name, font));
    }

    //--------------------------------------------------------------------------------------------------

    bool Font::Destroy ()
    {
        for (std::map<std::string, ALLEGRO_FONT*>::iterator it = m_Fonts.begin (); it != m_Fonts.end ();)
        {
            al_destroy_font (it->second);
            m_Fonts.erase (it++);
        }

        SAFE_DELETE (m_BackupTransform);

        return Lifecycle::Destroy ();
    }

    //--------------------------------------------------------------------------------------------------

    void Font::DrawText (const std::string& fontName, const std::string& text, ALLEGRO_COLOR color, float x, float y,
        float scale, int flags)
    {
        ALLEGRO_FONT* font = m_Fonts[fontName];

        ALLEGRO_TRANSFORM* currentTransform = const_cast<ALLEGRO_TRANSFORM*> (al_get_current_transform ());
        al_copy_transform (m_BackupTransform, currentTransform);

        al_identity_transform (currentTransform);

        al_scale_transform (currentTransform, scale, scale);

        al_draw_text (font, color, x / scale, y / scale, flags, text.c_str ());

        al_use_transform (m_BackupTransform);
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

        return Point {width, height};
    }

    //--------------------------------------------------------------------------------------------------

    int Font::GetFontAscent (const std::string& fontName)
    {
        ALLEGRO_FONT* font = m_Fonts[fontName];

        return al_get_font_ascent (font);
    }

    //--------------------------------------------------------------------------------------------------

    int Font::GetFontDescent (const std::string& fontName)
    {
        ALLEGRO_FONT* font = m_Fonts[fontName];

        return al_get_font_descent (font);
    }

    //--------------------------------------------------------------------------------------------------
}
