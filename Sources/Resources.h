// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __RESOURCES_H__
#define __RESOURCES_H__

#include <string>
#include <vector>

namespace aga
{
    const std::string FONT_NAME_SMALL = "FONT_SMALL";
    const std::string FONT_NAME_MEDIUM = "FONT_MEDIUM";
    const std::string FONT_NAME_NORMAL = "FONT_NORMAL";
    const std::string FONT_NAME_SPEECH_FRAME = "FONT_SPEECH_FRAME";

    const std::string FONT_NAME_MENU_TITLE = "FONT_NAME_MENU_TITLE";
    const std::string FONT_NAME_MENU_ITEM_NORMAL = "FONT_NAME_MENU_ITEM_NORMAL";
    const std::string FONT_NAME_MENU_ITEM_SMALL = "FONT_NAME_MENU_ITEM_SMALL";

    enum ResourceID
    {
        //  Graphics
        GFX_DEFAULT_SKIN,
        GFX_MENU_COG,
        GFX_TEXT_FRAME,
        GFX_PLAYER,

        //  PACKS
        PACK_MENU_UI,
        PACK_CHARACTERS_UI,
        PACK_0_0_HOME,

        //  Fonts
        FONT_SMALL,
        FONT_MEDIUM,
        FONT_NORMAL,
        FONT_MENU_TITLE,
        FONT_MENU_ITEM_NORMAL,
        FONT_MENU_ITEM_SMALL,
        FONT_SPEECH_FRAME,
        FONT_EDITOR,

        //  Scenes
        SCENE_0_0,

        //  Scripts
        SCRIPT_0_0,

        //  Sounds
        SOUND_FOOT_STEP,
        SOUND_MENU_SELECT
    };

    struct Resource
    {
        int ID;
        std::string Dir;
        std::string Name;
    };

    extern std::vector<std::string> g_SpeechFrames;

    const std::string GetDataPath ();
    const std::string GetResourcePath (ResourceID id);
}

#endif //   __RESOURCES_H__
