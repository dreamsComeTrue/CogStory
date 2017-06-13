// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "Resources.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    const char* GFX_UI = "gfx/ui";
    const char* GFX_CHARACTERS = "gfx/characters";
    const char* FONTS = "fonts";

    Resource g_Resources[] = {
        { GFX_MENU_COG, GFX_UI, "cog.png" },
        { GFX_TEXT_FRAME, GFX_UI, "text_frame.9.png" },
        { GFX_PLAYER, GFX_CHARACTERS, "player.png" },
        { FONT_MAIN, FONTS, "coders_crux.ttf" }
    };

    //--------------------------------------------------------------------------------------------------

    const std::string GetResourcePath (ResourceID id)
    {
        std::string path = "../Data/";
        Resource res = g_Resources[id];

        path += res.Dir + std::string ("/") + res.Name;

        return path;
    }

    //--------------------------------------------------------------------------------------------------
}
