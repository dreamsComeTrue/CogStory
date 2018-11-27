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

    float MENU_SELECTION_ROTATION_SPEED = 0.5;
    float MENU_ANIMATION_TIME_STAGE1 = 1500.f;
    float MENU_ANIMATION_TIME_STAGE2 = 500.f;

    MainMenuState::MainMenuState (MainLoop* mainLoop)
        : State (mainLoop, MAIN_MENU_STATE_NAME)
        , m_Image (nullptr)
        , m_SelectionAngle (0.0f)
        , m_Selection (0)
        , m_AnimationStage (0)
        , m_AnimationTimer (0.f)
        , m_SelectionTimer (0.f)
        , m_AnimationUp (true)
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
    }

    //--------------------------------------------------------------------------------------------------

    void MainMenuState::AfterLeave () {}

    //--------------------------------------------------------------------------------------------------

    bool MainMenuState::ProcessEvent (ALLEGRO_EVENT* event, float)
    {
        if (event->type == ALLEGRO_EVENT_KEY_DOWN)
        {
            switch (event->keyboard.keycode)
            {
            case ALLEGRO_KEY_UP:
            case ALLEGRO_KEY_W:
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
            case ALLEGRO_KEY_S:
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
            case ALLEGRO_KEY_X:
            {
                m_SelectSample->Play ();
                HandleSelection ();

                break;
            }
            }
        }

        return true;
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
        std::string menuItems[3] = {"NEW STORY", "CONTINUE", "EXIT"};

        Font& font = m_MainLoop->GetScreen ()->GetFont ();
        Point textSize = font.GetTextDimensions (FONT_NAME_MENU_ITEM_NORMAL, menuItems[m_Selection]);

        int offset = 10;
        int menuItemSpacing = static_cast<int> (textSize.Height + offset);
        int menuItemStartY = static_cast<int> (winSize.Height * 0.6f + menuItemSpacing);

        ALLEGRO_COLOR menuItemColor = al_map_rgb (140, 140, 160);
        ALLEGRO_COLOR backColor = al_map_rgb (60, 60, 70);

        float percent = 1.0f;

        if (m_AnimationStage < 2)
        {
            m_AnimationTimer += deltaTime * 1000.f;

            float maxTime = m_AnimationStage == 0 ? MENU_ANIMATION_TIME_STAGE1 : MENU_ANIMATION_TIME_STAGE2;

            if (m_AnimationTimer > maxTime)
            {
                ++m_AnimationStage;
                m_AnimationTimer = 0.f;
            }

            percent = m_AnimationTimer / maxTime;
        }

        if (m_AnimationStage >= 0)
        {
            float currentPercent = m_AnimationStage > 0 ? 1.0f : percent;

            al_draw_bitmap (m_Image, winSize.Width * 0.5f - al_get_bitmap_width (m_Image) * 0.5f,
                winSize.Height * 0.5f - al_get_bitmap_height (m_Image) * 0.5f, 0);
            al_draw_filled_rectangle (0, winSize.Height * currentPercent, winSize.Width, winSize.Height, backColor);

            if (m_AnimationStage == 0 && currentPercent > 0.7f)
            {
                m_AnimationTimer = MENU_ANIMATION_TIME_STAGE1;
            }
        }

        if (m_AnimationStage >= 1)
        {
            float currentPercent = m_AnimationStage > 1 ? 1.0f : percent;
            float scale = 1.3f * currentPercent;

            m_SelectionAngle += MENU_SELECTION_ROTATION_SPEED * deltaTime;
            m_SelectItemAtlas->DrawRegion ("cog", winSize.Width * 0.5f, winSize.Height * 0.5f - 70, scale, scale,
                m_SelectionAngle, false, COLOR_WHITE);
        }

        if (m_AnimationUp)
        {
            m_SelectionTimer += deltaTime * 1000.f;

            if (m_SelectionTimer > MENU_ANIMATION_TIME_STAGE2)
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

        for (int i = 0; i < 3; ++i)
        {
            float currentPercent = m_SelectionTimer / MENU_ANIMATION_TIME_STAGE2;
            float scale = i == m_Selection ? 1.0f + currentPercent * 0.2f : 1.0f;

            font.DrawText (FONT_NAME_MENU_ITEM_SMALL, menuItems[i], menuItemColor, winSize.Width * 0.5f + 250.f,
                m_Selection == i ? menuItemStartY + i * menuItemSpacing - offset * 0.5f
                                 : menuItemStartY + i * menuItemSpacing,
                scale, ALLEGRO_ALIGN_CENTER);
        }

        font.DrawText (FONT_NAME_SMALL, "v 0.1 (C) Dominik 'squall' Jasinski", al_map_rgb (130, 130, 130), 5,
            winSize.Height - 20, 1.0f, ALLEGRO_ALIGN_LEFT);
    }

    //--------------------------------------------------------------------------------------------------
}
