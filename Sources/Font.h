// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __FONT_H__
#define __FONT_H__

#include "Common.h"

namespace aga
{
    const std::string FONT_NAME_MAIN_SMALL = "FONT_MAIN_SMALL";
    const std::string FONT_NAME_MAIN_MEDIUM = "FONT_MAIN_MEDIUM";
    const std::string FONT_NAME_MAIN_NORMAL = "FONT_MAIN_NORMAL";

    class Font : public Lifecycle
    {
    public:
        Font ();
        virtual ~Font ();
        bool Initialize ();
        bool Destroy ();

        void DrawText (const std::string& fontName,
                       ALLEGRO_COLOR color,
                       float x,
                       float y,
                       const std::string& text,
                       int flags = ALLEGRO_ALIGN_CENTRE);
        void DrawMultilineText (const std::string& fontName,
                                ALLEGRO_COLOR color,
                                float x,
                                float y,
                                float max_width,
                                float line_height,
                                int flags,
                                const char* text,
                                ...);

        Point GetTextDimensions (const std::string& fontName, const std::string& text);
        unsigned GetFontAscent (const std::string& fontName);
        unsigned GetFontDescent (const std::string& fontName);

    private:
        std::map<std::string, ALLEGRO_FONT*> m_Fonts;
    };
}

#endif //   __FONT_H__
