// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "Resources.h"
#include "Common.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    std::string GFX = "gfx";
    std::string GFX_UI = "gfx/ui";
    std::string GFX_ACTORS = "gfx/actors";
    std::string GFX_NPC = "gfx/actors/npc";
    std::string GFX_EFFECTS = "gfx/effects";
    std::string GFX_ENV = "gfx/env";

    std::string GFX_0_HOME = "gfx/scenes/0_home";
    std::string GFX_1_FACTORY = "gfx/scenes/1_factory";
    std::string GFX_2_CITY = "gfx/scenes/2_city";

    std::string FONTS = "fonts/";
    std::string SCENES_HOME = "scenes/0_home/";
    std::string SCRIPTS_HOME = "scripts/0_home/";
    std::string SOUNDS = "sounds/";

    Resource g_Resources[] = {
        //  Graphics
        {GFX_DEFAULT_SKIN, GFX, "UISkin.png"}, {GFX_TEXT_FRAME, GFX_UI, "text_frame.9.png"},

        {__PACKS__MARKER__}, {PACK_MENU_UI, GFX_UI, "menu_ui.pack"}, {PACK_ACTORS_UI, GFX_UI, "actors_ui.pack"},
        {PACK_PLAYER, GFX_ACTORS, "player.pack"}, {PACK_PARTICLES, GFX_EFFECTS, "particles.pack"},
        {PACK_NPC_1, GFX_NPC, "npc_1.pack"}, {PACK_NPC_2, GFX_NPC, "npc_2.pack"}, {PACK_NPC_3, GFX_NPC, "npc_3.pack"},
        {PACK_NPC_4, GFX_NPC, "npc_4.pack"},

        //  Home
        {PACK_0_0_HOME, GFX_0_HOME, "0_0_home.pack"},

        //  Factory
        {PACK_1_0_FACTORY, GFX_1_FACTORY, "1_0_factory.pack"}, {PACK_1_1_FACTORY, GFX_1_FACTORY, "1_1_factory.pack"},

        //  City
        {PACK_2_0_CITY, GFX_2_CITY, "2_0_city.pack"}, {PACK_2_1_CITY, GFX_2_CITY, "2_1_city.pack"},
        {PACK_2_2_CITY, GFX_2_CITY, "2_2_city.pack"}, {PACK_2_3_CITY, GFX_2_CITY, "2_3_city.pack"},

        //  Env
        {PACK_ENV_0, GFX_ENV, "env_0.pack"}, {PACK_ENV_1, GFX_ENV, "env_1.pack"}, {PACK_ENV_5, GFX_ENV, "env_5.pack"},

        //  Fonts
        {__FONTS__MARKER__}, {FONT_SMALL, FONTS, "coders_crux.ttf"}, {FONT_MEDIUM, FONTS, "coders_crux.ttf"},
        {FONT_NORMAL, FONTS, "coders_crux.ttf"}, {FONT_MENU_TITLE, FONTS, "coders_crux.ttf"},
        {FONT_MENU_ITEM_NORMAL, FONTS, "coders_crux.ttf"}, {FONT_MENU_ITEM_SMALL, FONTS, "coders_crux.ttf"},
        {FONT_SPEECH_FRAME, FONTS, "coders_crux.ttf"}, {FONT_EDITOR, FONTS, "OpenSans.ttf"},

        //  Sounds
        {SOUND_FOOT_STEP, SOUNDS, "38874__swuing__footstep-grass.wav"},

        {SOUND_SPEECH_SELECT, SOUNDS, "select.wav"}, {SOUND_SPEECH_TYPE, SOUNDS, "type.wav"}};

    //--------------------------------------------------------------------------------------------------
    //--------------------------------------------------------------------------------------------------

    const std::string GetResourcePath (ResourceID id)
    {
        std::string path = GetDataPath ();
        Resource res = g_Resources[id];

        path += res.Dir + std::string ("/") + res.Name;

        return path;
    }

    //--------------------------------------------------------------------------------------------------

    Resource& GetResource (ResourceID id) { return g_Resources[id]; }

    //--------------------------------------------------------------------------------------------------

    std::vector<ResourceID> GetGfxPacks ()
    {
        std::vector<ResourceID> packs;

        for (int i = ResourceID::__PACKS__MARKER__ + 1; i < ResourceID::__FONTS__MARKER__; ++i)
        {
            packs.push_back (static_cast<ResourceID> (i));
        }

        return packs;
    }

    //--------------------------------------------------------------------------------------------------
}
