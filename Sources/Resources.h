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
        GFX_TEXT_FRAME,

        //  PACKS
        PACK_MENU_UI,
        PACK_ACTORS_UI,
        PACK_PLAYER,
        PACK_PARTICLES,
        PACK_NPC_1,
        PACK_0_0_HOME,
        PACK_1_0_FACTORY,
        PACK_2_0_CITY,
        PACK_2_1_CITY,

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
    Resource& GetResource (ResourceID id);

    std::vector<ResourceID> GetGfxPacks ();
}

#endif //   __RESOURCES_H__
