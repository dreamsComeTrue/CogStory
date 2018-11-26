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

    float MENU_SELECTION_ROTATION_SPEED = 1;
    float MENU_ANIMATION_TIME = 500.f;

    MainMenuState::MainMenuState (MainLoop* mainLoop)
        : State (mainLoop, MAIN_MENU_STATE_NAME)
        , m_SelectionAngle (0.0f)
        , m_Selection (0)
        , m_AnimationStage (0)
        , m_AnimationTimer (0.f)
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

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    bool MainMenuState::Destroy () { return Lifecycle::Destroy (); }

    //--------------------------------------------------------------------------------------------------

    void MainMenuState::BeforeEnter ()
    {
        al_hide_mouse_cursor (m_MainLoop->GetScreen ()->GetDisplay ());
        m_MainLoop->GetScreen ()->SetBackgroundColor (al_map_rgb (60, 60, 70));

        m_MainLoop->GetAudioManager ().SetEnabled (true);

        //  TODO: remove
        m_MainLoop->GetAudioManager ().SetEnabled (false);

        m_AnimationStage = 0;
        m_AnimationTimer = 0.f;
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
        int menuItemStartY = static_cast<int> (winSize.Height * 0.5f + menuItemSpacing);

        ALLEGRO_COLOR menuItemColor = al_map_rgb (140, 140, 160);
        ALLEGRO_COLOR backColor = al_map_rgb (60, 60, 70);

        //        font.DrawText (FONT_NAME_MENU_TITLE, menuItemColor, winSize.Width * 0.5f, menuItemStartY - 4 *
        //        menuItemSpacing,
        //           "ROBOT TALES", ALLEGRO_ALIGN_CENTER);

        float sideOffset = (winSize.Width - 620) * 0.5f;
        float topPos = winSize.Height * 0.5f - 180;
        float deltaHeight = 150;
        float bottomPos = topPos + deltaHeight;
        float halfTop = topPos + (bottomPos - topPos) * 0.5f;
        float lineThickness = 15;

        m_AnimationTimer += deltaTime * 1000.f;

        if (m_AnimationTimer > MENU_ANIMATION_TIME)
        {
            ++m_AnimationStage;
            m_AnimationTimer = 0.f;
        }

        float percent = m_AnimationTimer / MENU_ANIMATION_TIME;

        if (m_AnimationStage >= 0)
        {
            float currentPercent = m_AnimationStage > 0 ? 1.0f : percent;

            //  Main top bar
            DrawPolyLine (sideOffset, topPos, sideOffset + (winSize.Width - 2 * sideOffset - 9) * currentPercent,
                topPos, menuItemColor);
        }

        if (m_AnimationStage >= 1)
        {
            float currentPercent = m_AnimationStage > 1 ? 1.0f : percent;

            //  Pi
            al_draw_line (sideOffset + 60, topPos, sideOffset + 60, topPos + deltaHeight * currentPercent,
                menuItemColor, lineThickness);
            al_draw_line (sideOffset + 120, topPos, sideOffset + 120, topPos + deltaHeight * currentPercent,
                menuItemColor, lineThickness);
        }

        if (m_AnimationStage >= 1)
        {
            float currentPercent = m_AnimationStage > 1 ? 1.0f : percent;

            //  Hide
            al_draw_line (
                sideOffset + 180, topPos, sideOffset + 180 + 30 * currentPercent, topPos, backColor, lineThickness);
        }

        if (m_AnimationStage >= 1)
        {
            float currentPercent = m_AnimationStage > 1 ? 1.0f : percent;

            //  T
            al_draw_line (sideOffset + 290, topPos, sideOffset + 290, topPos + (deltaHeight + 200) * currentPercent,
                menuItemColor, lineThickness);
        }

        if (m_AnimationStage >= 1)
        {
            float currentPercent = m_AnimationStage > 1 ? 1.0f : percent;

            //  Hide
            al_draw_line (sideOffset + 380 - lineThickness * 0.5f, topPos,
                sideOffset + 380 - lineThickness * 0.5f + 30 * currentPercent, topPos, backColor, lineThickness);
        }

        if (m_AnimationStage >= 1)
        {
            float currentPercent = m_AnimationStage > 1 ? 1.0f : percent;

            //  E
            al_draw_line (sideOffset + 410, topPos, sideOffset + 410, topPos + deltaHeight * currentPercent,
                menuItemColor, lineThickness);
            al_draw_line (sideOffset + 410, halfTop, sideOffset + 410 + 50 * currentPercent, halfTop, menuItemColor,
                lineThickness);
            al_draw_line (sideOffset + 410, bottomPos - lineThickness * 0.5f, sideOffset + 410 + 80 * currentPercent,
                bottomPos - lineThickness * 0.5f, menuItemColor, lineThickness);
        }

        if (m_AnimationStage >= 1)
        {
            float currentPercent = m_AnimationStage > 1 ? 1.0f : percent;

            //  Hide
            al_draw_line (sideOffset + 490, topPos, sideOffset + 490 - lineThickness * 0.5f + 50 * currentPercent,
                topPos, backColor, lineThickness);
        }

        if (m_AnimationStage >= 1)
        {
            float currentPercent = m_AnimationStage > 1 ? 1.0f : percent;

            // R
            al_draw_line (sideOffset + 540, topPos, sideOffset + 540, topPos + deltaHeight * currentPercent,
                menuItemColor, lineThickness);
            al_draw_line (sideOffset + 610 - 70 * currentPercent, topPos + 2 + 70 * currentPercent, sideOffset + 610,
                topPos + 2, menuItemColor, lineThickness);
            al_draw_line (sideOffset + 540, halfTop, sideOffset + 540 + 70 * currentPercent,
                halfTop + 70 * currentPercent, menuItemColor, lineThickness);
        }

        for (int i = 0; i < 3; ++i)
        {
            font.DrawText (m_Selection == i ? FONT_NAME_MENU_ITEM_NORMAL : FONT_NAME_MENU_ITEM_SMALL, menuItemColor,
                sideOffset + 290 + 200.f,
                m_Selection == i ? menuItemStartY + i * menuItemSpacing - offset * 0.5f
                                 : menuItemStartY + i * menuItemSpacing,
                menuItems[i], ALLEGRO_ALIGN_CENTER);
        }

        int xPos = static_cast<int> (sideOffset + 290 + 200.f - textSize.Width * 0.5f - 30.f);
        int yPos = static_cast<int> (menuItemStartY + textSize.Height * 0.5f + m_Selection * menuItemSpacing - 7.f);

        m_SelectionAngle += MENU_SELECTION_ROTATION_SPEED * deltaTime;

        float scale = 0.6f;

        m_SelectItemAtlas->DrawRegion ("cog", xPos, yPos, scale, scale, m_SelectionAngle, false, COLOR_WHITE);

        font.DrawText (FONT_NAME_SMALL, al_map_rgb (130, 130, 130), winSize.Width - 5, winSize.Height - 20, "v 0.1",
            ALLEGRO_ALIGN_RIGHT);
    }

    //--------------------------------------------------------------------------------------------------
}
