// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "Resources.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    std::string GFX_UI = "gfx/ui";
    std::string GFX_CHARACTERS = "gfx/characters";
    std::string FONTS = "fonts";
    std::string SCENES_HOME = "scenes/0_home";
    std::string SCRIPTS_HOME = "scripts/0_home";

    Resource g_Resources[] = {
        //  Graphics
        { GFX_MENU_COG, GFX_UI, "cog.png" },
        { GFX_TEXT_FRAME, GFX_UI, "text_frame.9.png" },
        { GFX_PLAYER, GFX_CHARACTERS, "player.png" },

        //  Fonts
        { FONT_MAIN, FONTS, "coders_crux.ttf" },

        //  Scenes
        { SCENE_0_0, SCENES_HOME, "0_0_home.scn" },

        //  Scripts
        { SCRIPT_0_0, SCRIPTS_HOME, "0_0_home.script" }
    };

    //--------------------------------------------------------------------------------------------------

    const std::string GetDataPath ()
    {
        std::string path = "../Data/";

        return path;
    }

    //--------------------------------------------------------------------------------------------------

    const std::string GetResourcePath (ResourceID id)
    {
        std::string path = GetDataPath ();
        Resource res = g_Resources[id];

        path += res.Dir + std::string ("/") + res.Name;

        return path;
    }

    //--------------------------------------------------------------------------------------------------
}
