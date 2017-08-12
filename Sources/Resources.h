// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __RESOURCES_H__
#define __RESOURCES_H__

#include <string>

namespace aga
{
    enum ResourceID
    {
        //  Graphics
        GFX_DEFAULT_SKIN,
        GFX_MENU_COG,
        GFX_TEXT_FRAME,
        GFX_PLAYER,

        //  Fonts
        FONT_MAIN,
        FONT_EDITOR,

        //  Scenes
        SCENE_0_0,

        //  Scripts
        SCRIPT_0_0
    };

    struct Resource
    {
        int ID;
        std::string Dir;
        std::string Name;
    };

    const std::string GetDataPath ();
    const std::string GetResourcePath (ResourceID id);
}

#endif //   __RESOURCES_H__
