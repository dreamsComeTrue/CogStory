// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "MainMenuState.h"
#include "AudioSample.h"
#include "Common.h"
#include "GamePlayState.h"
#include "MainLoop.h"
#include "Screen.h"
#include "StateManager.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    float MENU_SELECTION_ROTATION_SPEED = 1;

    MainMenuState::MainMenuState (MainLoop* mainLoop)
      : State (mainLoop, MAIN_MENU_STATE_NAME)
      , m_Selection (0)
      , m_SelectionAngle (0.0f)
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

        m_SelectSample = m_MainLoop->GetSceneManager ().GetMainLoop ()->GetAudioManager ().LoadSampleFromFile (
          "SELECT_MENU", GetResourcePath (SOUND_MENU_SELECT));
        m_SelectImage = al_load_bitmap (GetResourcePath (ResourceID::GFX_MENU_COG).c_str ());

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    bool MainMenuState::Destroy ()
    {
        al_destroy_bitmap (m_SelectImage);

        return Lifecycle::Destroy ();
    }

    //--------------------------------------------------------------------------------------------------

    void MainMenuState::BeforeEnter () { m_MainLoop->GetScreen ()->SetBackgroundColor (al_map_rgb (60, 60, 70)); }

    //--------------------------------------------------------------------------------------------------

    void MainMenuState::AfterLeave () {}

    //--------------------------------------------------------------------------------------------------

    void MainMenuState::ProcessEvent (ALLEGRO_EVENT* event, float deltaTime)
    {
        if (event->type == ALLEGRO_EVENT_KEY_DOWN)
        {
            switch (event->keyboard.keycode)
            {
                case ALLEGRO_KEY_UP:
                {
                    m_SelectSample->Play ();

                    --m_Selection;

                    if (m_Selection < MENU_ITEM_NEW_STORY)
                    {
                        m_Selection = MENU_ITEM_EXIT;
                    }

                    break;
                }

                case ALLEGRO_KEY_DOWN:
                {
                    m_SelectSample->Play ();

                    ++m_Selection;

                    if (m_Selection > MENU_ITEM_EXIT)
                    {
                        m_Selection = MENU_ITEM_NEW_STORY;
                    }

                    break;
                }

                case ALLEGRO_KEY_ENTER:
                {
                    m_SelectSample->Play ();
                    HandleSelection ();

                    break;
                }
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    void MainMenuState::HandleSelection ()
    {
        switch (m_Selection)
        {
            case MENU_ITEM_NEW_STORY:
            {
                m_MainLoop->GetStateManager ().StateFadeInOut (GAMEPLAY_STATE_NAME);
                break;
            }

            case MENU_ITEM_CONTINUE:
            {
                m_MainLoop->GetStateManager ().StateFadeInOut (GAMEPLAY_STATE_NAME);
                break;
            }

            case MENU_ITEM_EXIT:
            {
                m_MainLoop->Exit ();
                break;
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    void MainMenuState::Update (float deltaTime) {}

    //--------------------------------------------------------------------------------------------------

    void MainMenuState::Render (float deltaTime)
    {
        const Point winSize = m_MainLoop->GetScreen ()->GetWindowSize ();

        Font& font = m_MainLoop->GetScreen ()->GetFont ();

        int titlePosY = 100;
        int menuItemSpacing = 30;
        int menuItemStartY = winSize.Height * 0.5f - menuItemSpacing;

        ALLEGRO_COLOR menuItemColor = al_map_rgb (140, 140, 160);

        font.DrawText (
          FONT_NAME_MAIN_NORMAL, menuItemColor, winSize.Width * 0.5f, titlePosY, "ROBOT TALES", ALLEGRO_ALIGN_CENTER);

        font.DrawText (m_Selection == 0 ? FONT_NAME_MAIN_NORMAL : FONT_NAME_MAIN_MEDIUM,
                       menuItemColor,
                       winSize.Width * 0.5f,
                       menuItemStartY,
                       "NEW STORY",
                       ALLEGRO_ALIGN_CENTER);
        font.DrawText (m_Selection == 1 ? FONT_NAME_MAIN_NORMAL : FONT_NAME_MAIN_MEDIUM,
                       menuItemColor,
                       winSize.Width * 0.5f,
                       menuItemStartY + menuItemSpacing,
                       "CONTINUE",
                       ALLEGRO_ALIGN_CENTER);
        font.DrawText (m_Selection == 2 ? FONT_NAME_MAIN_NORMAL : FONT_NAME_MAIN_MEDIUM,
                       menuItemColor,
                       winSize.Width * 0.5f,
                       menuItemStartY + 2 * menuItemSpacing,
                       "EXIT",
                       ALLEGRO_ALIGN_CENTER);

        int sourceWidth = al_get_bitmap_width (m_SelectImage);
        int sourceHeight = al_get_bitmap_height (m_SelectImage);

        int yPos = menuItemStartY + 10 + m_Selection * 27;

        m_SelectionAngle += MENU_SELECTION_ROTATION_SPEED * deltaTime;

        al_draw_tinted_scaled_rotated_bitmap (m_SelectImage,
                                              COLOR_WHITE,
                                              sourceWidth * 0.5,
                                              sourceHeight * 0.5,
                                              winSize.Width * 0.5f - 100,
                                              yPos,
                                              0.6f,
                                              0.6f,
                                              m_SelectionAngle,
                                              0);

        font.DrawText (FONT_NAME_MAIN_SMALL,
                       al_map_rgb (130, 130, 130),
                       winSize.Width - 5,
                       winSize.Height - 20,
                       "v 0.1",
                       ALLEGRO_ALIGN_RIGHT);
    }

    //--------------------------------------------------------------------------------------------------
}
