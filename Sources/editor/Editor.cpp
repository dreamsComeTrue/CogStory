// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "Editor.h"
#include "Screen.h"
#include "ui/Button.h"
#include "ui/Label.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    const int TILE_SIZE = 128;

    //--------------------------------------------------------------------------------------------------
    //--------------------------------------------------------------------------------------------------

    Editor::Editor (Screen* screen)
        : m_Screen (screen)
        , m_UIManager (screen)
        , m_DrawTiles (true)
    {
    }

    //--------------------------------------------------------------------------------------------------

    Editor::~Editor ()
    {
        if (!IsDestroyed ())
        {
            Destroy ();
        }

        Lifecycle::Destroy ();
    }

    //--------------------------------------------------------------------------------------------------

    bool Editor::Initialize ()
    {
        m_UIManager.Initialize ();
        InitializeUI ();

        Lifecycle::Initialize ();
    }

    //--------------------------------------------------------------------------------------------------

    bool Editor::Destroy ()
    {
        m_UIManager.Destroy ();

        Lifecycle::Destroy ();
    }

    //--------------------------------------------------------------------------------------------------

    bool Editor::Update (double deltaTime)
    {
        m_UIManager.Update (deltaTime);
        return true;
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::ProcessEvent (ALLEGRO_EVENT* event, double deltaTime)
    {
        m_UIManager.ProcessEvent (event, deltaTime);

        if (event->type == ALLEGRO_EVENT_KEY_UP)
        {
            if (event->keyboard.keycode == ALLEGRO_KEY_SPACE)
            {
                m_DrawTiles = !m_DrawTiles;
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::Render (double deltaTime)
    {
        m_UIManager.Render (deltaTime);

        if (m_DrawTiles)
        {
            DrawTiles ();
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::DrawTiles ()
    {
        const Point& screenSize = m_Screen->GetScreenSize ();
        double centerX = screenSize.Width * 0.5;
        double beginning = centerX - 2 * TILE_SIZE - TILE_SIZE * 0.5;

        //  Back frame
        for (int i = 0; i < 5; ++i)
        {
            int advance = beginning + i * TILE_SIZE;
            al_draw_rectangle (advance, screenSize.Height - TILE_SIZE, advance + TILE_SIZE, screenSize.Height, COLOR_GREEN, 1);
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::InitializeUI ()
    {
        Label* label = new Label (m_Screen, "MENU");
        label->SetPosition (20, 20);
        label->SetDrawBorder (true);
        label->SetBorderColor (COLOR_GREEN);
        label->SetTextColor (COLOR_RED);

        m_UIManager.AddWidget (0, label);
    }

    //--------------------------------------------------------------------------------------------------
}
