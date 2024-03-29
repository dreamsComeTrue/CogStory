// Copyright 2017-2019 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __FONT_H__
#define __FONT_H__

#include "Common.h"

namespace aga
{
    const std::string FONT_NAME_SMALL = "FONT_SMALL";
    const std::string FONT_NAME_MEDIUM = "FONT_MEDIUM";
    const std::string FONT_NAME_NORMAL = "FONT_NORMAL";
    const std::string FONT_NAME_SPEECH_FRAME = "FONT_SPEECH_FRAME";

    const std::string FONT_NAME_MENU_TITLE = "FONT_NAME_MENU_TITLE";
    const std::string FONT_NAME_MENU_ITEM_NORMAL = "FONT_NAME_MENU_ITEM_NORMAL";
    const std::string FONT_NAME_MENU_ITEM_SMALL = "FONT_NAME_MENU_ITEM_SMALL";

    class Font : public Lifecycle
    {
    public:
        Font ();
        virtual ~Font ();
        bool Initialize ();
        bool Destroy ();

        static void DrawText (const std::string& fontName, const std::string& text, ALLEGRO_COLOR color, float x,
            float y, float scale, int flags = ALLEGRO_ALIGN_CENTRE);
        static void DrawMultilineText (const std::string& fontName, ALLEGRO_COLOR color, float x, float y,
            float max_width, float line_height, int flags, const char* text, ...);

        static Point GetTextDimensions (const std::string& fontName, const std::string& text);
        static int GetFontAscent (const std::string& fontName);
        static int GetFontDescent (const std::string& fontName);

    private:
        void GenerateFont (const std::string& name, const std::string& path, int size);

    private:
        static ALLEGRO_TRANSFORM* m_BackupTransform;
        static std::map<std::string, ALLEGRO_FONT*> m_Fonts;
    };
}

#endif //   __FONT_H__
