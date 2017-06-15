// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "Editor.h"
#include "Screen.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    Editor::Editor (Screen* screen)
        : m_Screen (screen)
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
        Lifecycle::Initialize ();
    }

    //--------------------------------------------------------------------------------------------------

    bool Editor::Destroy ()
    {
        Lifecycle::Destroy ();
    }

    //--------------------------------------------------------------------------------------------------

    bool Editor::Update (double deltaTime)
    {
        return true;
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::ProcessEvent (ALLEGRO_EVENT* event, double deltaTime)
    {
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
        const Point& screenSize = m_Screen->GetScreenSize ();

        if (m_DrawTiles)
        {
            al_draw_rectangle (0.5, screenSize.Height - 2 * 64, screenSize.Width, screenSize.Height, al_map_rgb (0, 255, 0), 1);
        }
    }

    //--------------------------------------------------------------------------------------------------
}
