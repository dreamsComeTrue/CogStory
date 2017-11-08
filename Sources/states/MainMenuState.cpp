// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "MainMenuState.h"
#include "Common.h"
#include "Screen.h"
#include "StateManager.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    std::string MAIN_MENU_STATE_NAME = "MAIN_MENU_STATE";

    MainMenuState::MainMenuState (MainLoop* mainLoop)
        : State (mainLoop, MAIN_MENU_STATE_NAME)
    {
    }

    //--------------------------------------------------------------------------------------------------

    MainMenuState::~MainMenuState ()
    {
        if (!IsDestroyed ())
        {
            Destroy ();
        }
    }

    //--------------------------------------------------------------------------------------------------

    bool MainMenuState::Initialize ()
    {
        Lifecycle::Initialize ();

        m_Background = load_nine_patch_bitmap (GetResourcePath (ResourceID::GFX_TEXT_FRAME).c_str ());

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    bool MainMenuState::Destroy ()
    {
        destroy_nine_patch_bitmap (m_Background);

        return Lifecycle::Destroy ();
    }

    //--------------------------------------------------------------------------------------------------

    void MainMenuState::BeforeEnter () {}

    //--------------------------------------------------------------------------------------------------

    void MainMenuState::AfterLeave () {}

    //--------------------------------------------------------------------------------------------------

    void MainMenuState::ProcessEvent (ALLEGRO_EVENT* event, float deltaTime) {}

    //--------------------------------------------------------------------------------------------------

    void MainMenuState::Update (float deltaTime) {}

    //--------------------------------------------------------------------------------------------------

    void MainMenuState::Render (float deltaTime) { draw_nine_patch_bitmap (m_Background, 20, 20, 400, 150); }

    //--------------------------------------------------------------------------------------------------
}
