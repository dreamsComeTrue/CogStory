// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "MainMenuState.h"
#include "Atlas.h"
#include "AudioSample.h"
#include "GamePlayState.h"
#include "MainLoop.h"
#include "Resources.h"
#include "Screen.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    const float MENU_SELECTION_ROTATION_SPEED = 0.5;
    const float MENU_ANIMATION_TIME = 500.f;

    //--------------------------------------------------------------------------------------------------
    //--------------------------------------------------------------------------------------------------

    MainMenuState::MainMenuState (MainLoop* mainLoop)
        : State (mainLoop, MAIN_MENU_STATE_NAME)
        , m_Image (nullptr)
        , m_SelectionAngle (0.0f)
        , m_Selection (0)
        , m_AnimationStage (0)
        , m_AnimationTimer (0.f)
        , m_SelectionTimer (0.f)
        , m_AnimationUp (true)
        , m_ExitSelected (false)
        , m_Closing (false)
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
            "SELECT_MENU", GetResource (SOUND_SPEECH_SELECT).Dir + GetResource (SOUND_SPEECH_SELECT).Name);

        m_SelectItemAtlas = m_MainLoop->GetAtlasManager ().GetAtlas ("menu_ui");

        m_Image = al_load_bitmap ((GetDataPath () + "/gfx/ui/main_menu.png").c_str ());

        m_AnimationStage = 0;
        m_AnimationTimer = 0.f;

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    bool MainMenuState::Destroy ()
    {
        al_destroy_bitmap (m_Image);

        return Lifecycle::Destroy ();
    }

    //--------------------------------------------------------------------------------------------------

    void MainMenuState::BeforeEnter ()
    {
        al_hide_mouse_cursor (m_MainLoop->GetScreen ()->GetDisplay ());
        m_MainLoop->GetScreen ()->SetBackgroundColor (al_map_rgb (60, 60, 70));

        m_MainLoop->GetAudioManager ().SetEnabled (true);

        //  TODO: remove
        m_MainLoop->GetAudioManager ().SetEnabled (false);

        m_ExitSelected = false;
        m_Closing = false;

        m_AnimationStage = 0;
        m_AnimationTimer = 0.f;
        m_SelectionTimer = 0.f;
    }

    //--------------------------------------------------------------------------------------------------

    void MainMenuState::AfterLeave () {}

    //--------------------------------------------------------------------------------------------------

    bool MainMenuState::ProcessEvent (ALLEGRO_EVENT* event, float)
    {
        if (event->type == ALLEGRO_EVENT_JOYSTICK_BUTTON_UP)
        {
            if (event->joystick.button == 1)
            {
                m_AnimationTimer = 0.f;
                m_Closing = true;
            }
            else if (event->joystick.button == 2)
            {
                HandleSelection ();
            }
        }

        if (event->type == ALLEGRO_EVENT_JOYSTICK_AXIS)
        {
            if (event->joystick.pos < -0.7f)
            {
                HandleMoveUp ();
            }
            else if (event->joystick.pos > 0.7f)
            {
                HandleMoveDown ();
            }
        }

        if (event->type == ALLEGRO_EVENT_KEY_CHAR)
        {
            switch (event->keyboard.keycode)
            {
            case ALLEGRO_KEY_ESCAPE:
                m_AnimationTimer = 0.f;
                m_Closing = true;
                break;
            }
        }

        if (event->type == ALLEGRO_EVENT_KEY_DOWN)
        {
            switch (event->keyboard.keycode)
            {
            case ALLEGRO_KEY_UP:
            case ALLEGRO_KEY_W:
            {
                HandleMoveUp ();

                break;
            }

            case ALLEGRO_KEY_DOWN:
            case ALLEGRO_KEY_S:
            {
                HandleMoveDown ();

                break;
            }

            case ALLEGRO_KEY_ENTER:
            case ALLEGRO_KEY_X:
            {
                HandleSelection ();

                break;
            }
            }
        }

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    void MainMenuState::HandleMoveUp ()
    {
        m_SelectSample->Play ();

        --m_Selection;

        if (m_ExitSelected)
        {
            if (m_Selection < MENU_ITEM_EXIT_YES)
            {
                m_Selection = MENU_ITEM_EXIT_NO;
            }
        }
        else
        {
            if (m_Selection < MENU_ITEM_NEW_JOURNEY)
            {
                m_Selection = MENU_ITEM_EXIT;
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    void MainMenuState::HandleMoveDown ()
    {
        m_SelectSample->Play ();

        ++m_Selection;

        if (m_ExitSelected)
        {
            if (m_Selection > MENU_ITEM_EXIT_NO)
            {
                m_Selection = MENU_ITEM_EXIT_YES;
            }
        }
        else
        {
            if (m_Selection > MENU_ITEM_EXIT)
            {
                m_Selection = MENU_ITEM_NEW_JOURNEY;
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    void MainMenuState::HandleSelection ()
    {
        m_SelectSample->Play ();

        switch (m_Selection)
        {
        case MENU_ITEM_NEW_JOURNEY:
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
            m_ExitSelected = true;
            m_Selection = MENU_ITEM_EXIT_YES;
            break;
        }

        case MENU_ITEM_EXIT_YES:
        {
            m_MainLoop->Exit ();
            break;
        }

        case MENU_ITEM_EXIT_NO:
        {
            m_ExitSelected = false;
            m_Selection = MENU_ITEM_EXIT;
            break;
        }
        }
    }

    //--------------------------------------------------------------------------------------------------

    void MainMenuState::Update (float) {}

    //--------------------------------------------------------------------------------------------------

    struct VertexInfo
    {
        float x;
        float y;
    };

    void DrawPolyLine (float x1, float y1, float x2, float y2, ALLEGRO_COLOR c)
    {
        float lineThickness = 15;

        float verts[] = {x1, y1, x2, y2};

        al_draw_polyline (
            verts, 2 * sizeof (float), 2, ALLEGRO_LINE_JOIN_ROUND, ALLEGRO_LINE_CAP_ROUND, c, lineThickness, 1.0);
    }

    //--------------------------------------------------------------------------------------------------

    void MainMenuState::Render (float deltaTime)
    {
        const Point winSize = m_MainLoop->GetScreen ()->GetWindowSize ();
        Font& font = m_MainLoop->GetScreen ()->GetFont ();

        if (!m_Closing)
        {
            if (m_AnimationStage < 2)
            {
                m_AnimationTimer += deltaTime * 1000.f;

                if (m_AnimationTimer > MENU_ANIMATION_TIME)
                {
                    ++m_AnimationStage;
                    m_AnimationTimer = 0.f;
                }
            }
        }
        else
        {
            m_AnimationTimer += deltaTime * 1000.f;

            if (m_AnimationTimer > MENU_ANIMATION_TIME)
            {
                ++m_AnimationStage;
                m_AnimationTimer = 0.f;
            }
        }

        float percent = m_AnimationTimer / MENU_ANIMATION_TIME;
        float currentPercent;

        if (!m_Closing)
        {
            currentPercent = m_AnimationStage > 0 ? 1.0f : clamp (percent, 0.f, 1.f);
        }
        else
        {
            currentPercent = 1.0f - clamp (percent, 0.f, 1.f);

            if (m_AnimationStage >= 1)
            {
                m_MainLoop->GetStateManager ().StateFadeInOut (GAMEPLAY_STATE_NAME);
            }
        }

        float imgHeight = al_get_bitmap_height (m_Image);
        float targetYPos = winSize.Height * 0.5f - imgHeight * 0.5f;
        float yPos = -imgHeight * 0.5f + std::abs (-imgHeight * 0.5f - targetYPos) * currentPercent;

        al_draw_bitmap (m_Image, winSize.Width * 0.5f - al_get_bitmap_width (m_Image) * 0.5f, yPos, 0);

        if (m_ExitSelected)
        {
            RenderExitItems ();
        }
        else
        {
            RenderMenuItems ();
        }

        //  Draw center cog
        float scale = 1.3f * currentPercent;

        m_SelectionAngle += MENU_SELECTION_ROTATION_SPEED * deltaTime;
        m_SelectItemAtlas->DrawRegion ("cog", winSize.Width * 0.5f, winSize.Height * 0.5f - 70, scale, scale,
            m_SelectionAngle, false, COLOR_WHITE);

        if (m_AnimationUp)
        {
            m_SelectionTimer += deltaTime * 1000.f;

            if (m_SelectionTimer > MENU_ANIMATION_TIME)
            {
                m_AnimationUp = false;
            }
        }
        else
        {
            m_SelectionTimer -= deltaTime * 1000.f;

            if (m_SelectionTimer <= 0.f)
            {
                m_SelectionTimer = 0.f;
                m_AnimationUp = true;
            }
        }

        font.DrawText (FONT_NAME_SMALL, "v 0.1 (C) Agado Studio", al_map_rgb (130, 130, 130), 5, winSize.Height - 20,
            1.0f, ALLEGRO_ALIGN_LEFT);
    }

    //--------------------------------------------------------------------------------------------------

    void MainMenuState::RenderMenuItems ()
    {
        const Point winSize = m_MainLoop->GetScreen ()->GetWindowSize ();
        std::string menuItems[3] = {"NEW JOURNEY", "CONTINUE", "EXIT"};

        Font& font = m_MainLoop->GetScreen ()->GetFont ();
        Point textSize = font.GetTextDimensions (FONT_NAME_MENU_ITEM_NORMAL, menuItems[m_Selection]);

        int offset = 10;
        int menuItemSpacing = static_cast<int> (textSize.Height + offset);
        int menuItemStartY = static_cast<int> (winSize.Height * 0.6f + menuItemSpacing);

        ALLEGRO_COLOR menuItemColor = al_map_rgb (140, 140, 160);

        float percent = m_AnimationTimer / MENU_ANIMATION_TIME;
        float currentPercent;

        if (!m_Closing)
        {
            currentPercent = m_AnimationStage > 0 ? 1.0f : clamp (percent, 0.f, 1.f);
        }
        else
        {
            currentPercent = 1.0f - clamp (percent, 0.f, 1.f);
        }

        for (int i = 0; i < 3; ++i)
        {
            float scale = i == m_Selection ? 1.0f + (m_SelectionTimer / MENU_ANIMATION_TIME) * 0.2f : 1.0f;
            float targetXPos = winSize.Width * 0.5f + 250.f;
            float xPos = winSize.Width - (winSize.Width - targetXPos) * currentPercent;

            font.DrawText (FONT_NAME_MENU_ITEM_SMALL, menuItems[i], menuItemColor, xPos,
                m_Selection == i ? menuItemStartY + i * menuItemSpacing - offset * 0.5f
                                 : menuItemStartY + i * menuItemSpacing,
                scale, ALLEGRO_ALIGN_CENTER);
        }
    }

    //--------------------------------------------------------------------------------------------------

    void MainMenuState::RenderExitItems ()
    {
        const Point winSize = m_MainLoop->GetScreen ()->GetWindowSize ();
        std::string menuItems[3] = {"SURE?", "YES", "NO"};

        Font& font = m_MainLoop->GetScreen ()->GetFont ();
        Point textSize = font.GetTextDimensions (FONT_NAME_MENU_ITEM_NORMAL, menuItems[m_Selection - MENU_ITEM_EXIT]);

        int offset = 10;
        int menuItemSpacing = static_cast<int> (textSize.Height + offset);
        int menuItemStartY = static_cast<int> (winSize.Height * 0.6f + menuItemSpacing);

        ALLEGRO_COLOR menuItemColor = al_map_rgb (140, 140, 160);

        for (int i = 0; i < 3; ++i)
        {
            float currentPercent = m_SelectionTimer / MENU_ANIMATION_TIME;
            float scale = i == (m_Selection - MENU_ITEM_EXIT) ? 1.0f + currentPercent * 0.2f : 1.0f;

            font.DrawText (FONT_NAME_MENU_ITEM_SMALL, menuItems[i], menuItemColor, winSize.Width * 0.5f + 250.f,
                i == (m_Selection - MENU_ITEM_EXIT) ? menuItemStartY + i * menuItemSpacing - offset * 0.5f
                                                    : menuItemStartY + i * menuItemSpacing,
                scale, ALLEGRO_ALIGN_CENTER);
        }
    }

    //--------------------------------------------------------------------------------------------------
}
