// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __RESOURCES_H__
#define __RESOURCES_H__

#include <string>
#include <vector>

namespace aga
{
    enum ResourceID
    {
        //  Graphics
        GFX_DEFAULT_SKIN,
        GFX_TEXT_FRAME,

        __PACKS__MARKER__,

        //  PACKS
        PACK_MENU_UI,
        PACK_ACTORS_UI,
        PACK_PLAYER,
        PACK_PARTICLES,
        PACK_NPC_1,
        PACK_NPC_2,
        PACK_NPC_3,
        PACK_NPC_4,
        PACK_0_0_HOME,
        PACK_1_0_FACTORY,
        PACK_1_1_FACTORY,
        PACK_2_0_CITY,
        PACK_2_1_CITY,
        PACK_2_2_CITY,
        PACK_2_3_CITY,
        PACK_ENV_0,
        PACK_ENV_1,
        PACK_ENV_5,

        __FONTS__MARKER__,

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
        SOUND_SPEECH_SELECT,
        SOUND_SPEECH_TYPE
    };

    struct Resource
    {
        int ID;
        std::string Dir;
        std::string Name;
    };

    const std::string GetResourcePath (ResourceID id);
    Resource& GetResource (ResourceID id);

    std::vector<ResourceID> GetGfxPacks ();
}

#endif //   __RESOURCES_H__
