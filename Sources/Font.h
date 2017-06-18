// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __FONT_H__
#define __FONT_H__

#include "Common.h"

namespace aga
{
    const std::string FONT_NAME_MAIN = "FONT_MAIN";

    class Font : public Lifecycle
    {
    public:
        Font ();
        virtual ~Font ();
        bool Initialize ();
        bool Destroy ();

        void DrawText (const std::string& fontName, ALLEGRO_COLOR color, float x, float y, const std::string& text, int flags = ALLEGRO_ALIGN_CENTRE);
        Point GetTextDimensions (const std::string& fontName, const std::string& text);
        unsigned GetFontAscent (const std::string& fontName);

    private:
        std::map<std::string, ALLEGRO_FONT*> m_Fonts;
    };
}

#endif //   __FONT_H__
