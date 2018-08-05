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

    std::string GFX_0_HOME = "gfx/scenes/0_home";
    std::string GFX_1_FACTORY = "gfx/scenes/1_factory";
    std::string GFX_2_CITY = "gfx/scenes/2_city";

    std::string FONTS = "fonts";
    std::string SCENES_HOME = "scenes/0_home";
    std::string SCRIPTS_HOME = "scripts/0_home";
    std::string SOUNDS = "sounds";

    Resource g_Resources[] = {
        //  Graphics
        {GFX_DEFAULT_SKIN, GFX, "UISkin.png"}, {GFX_TEXT_FRAME, GFX_UI, "text_frame.9.png"},

        {PACK_MENU_UI, GFX_UI, "menu_ui.pack"}, {PACK_ACTORS_UI, GFX_UI, "actors_ui.pack"},
        {PACK_PLAYER, GFX_ACTORS, "player.pack"}, {PACK_PARTICLES, GFX_EFFECTS, "particles.pack"},
        {PACK_NPC_1, GFX_NPC, "npc_1.pack"}, {PACK_0_0_HOME, GFX_0_HOME, "0_0_home.pack"},
        {PACK_1_0_FACTORY, GFX_1_FACTORY, "1_0_factory.pack"}, {PACK_2_0_CITY, GFX_2_CITY, "2_0_city.pack"},
        {PACK_2_1_CITY, GFX_2_CITY, "2_1_city.pack"},

        //  Fonts
        {FONT_SMALL, FONTS, "coders_crux.ttf"}, {FONT_MEDIUM, FONTS, "coders_crux.ttf"},
        {FONT_NORMAL, FONTS, "coders_crux.ttf"}, {FONT_MENU_TITLE, FONTS, "coders_crux.ttf"},
        {FONT_MENU_ITEM_NORMAL, FONTS, "coders_crux.ttf"}, {FONT_MENU_ITEM_SMALL, FONTS, "coders_crux.ttf"},
        {FONT_SPEECH_FRAME, FONTS, "coders_crux.ttf"}, {FONT_EDITOR, FONTS, "OpenSans.ttf"},

        //  Scenes
        {SCENE_0_0, SCENES_HOME, "0_0_home.scn"},

        //  Scripts
        {SCRIPT_0_0, SCRIPTS_HOME, "0_0_home.script"},

        //  Sounds
        {SOUND_FOOT_STEP, SOUNDS, "38874__swuing__footstep-grass.wav"},
        {SOUND_MENU_SELECT, SOUNDS, "sfx_movement_ladder2a.wav"}};

    //--------------------------------------------------------------------------------------------------

    std::vector<std::string> g_SpeechFrames = {"player_head"};

    //--------------------------------------------------------------------------------------------------
    //--------------------------------------------------------------------------------------------------

    const std::string GetDataPath () { return GetCurrentDir () + "/../../Data/"; }

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
        return {PACK_MENU_UI, PACK_ACTORS_UI, PACK_PLAYER, PACK_NPC_1, PACK_PARTICLES, PACK_0_0_HOME, PACK_1_0_FACTORY,
            PACK_2_0_CITY, PACK_2_1_CITY};
    }

    //--------------------------------------------------------------------------------------------------
}
