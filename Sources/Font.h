// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __FONT_H__
#define __FONT_H__

#include "Common.h"

namespace aga
{
    class Font : public Lifecycle
    {
    public:
        Font ();
        virtual ~Font ();
        bool Initialize ();
        bool Destroy ();

        static void DrawText (const std::string& fontName, ALLEGRO_COLOR color, float x, float y,
                              const std::string& text, int flags = ALLEGRO_ALIGN_CENTRE);
        static void DrawMultilineText (const std::string& fontName, ALLEGRO_COLOR color, float x, float y,
                                       float max_width, float line_height, int flags, const char* text, ...);

        static Point GetTextDimensions (const std::string& fontName, const std::string& text);
        static unsigned GetFontAscent (const std::string& fontName);
        static unsigned GetFontDescent (const std::string& fontName);

    private:
        void GenerateFont (const std::string& name, const std::string& path, float size);

    private:
        static std::map<std::string, ALLEGRO_FONT*> m_Fonts;
    };
}

#endif //   __FONT_H__
