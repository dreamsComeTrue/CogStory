// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "Editor.h"
#include "Screen.h"
#include "ui/Button.h"
#include "ui/ButtonImage.h"
#include "ui/Frame.h"
#include "ui/Image.h"
#include "ui/Label.h"
#include "ui/Menu.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    const int TILE_SIZE = 64;

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

    void Editor::DrawTiles () {}

    //--------------------------------------------------------------------------------------------------

    void Editor::InitializeUI ()
    {
        Menu* mainMenu = new Menu (&m_UIManager, Point{ 4, 4 });
        MenuItem* editorSubMenu = new MenuItem ("EDITOR", mainMenu);

        MenuItem* newSceneMenu = new MenuItem ("NEW SCENE", mainMenu);
        editorSubMenu->AddChild (newSceneMenu);

        MenuItem* exitMenu = new MenuItem ("EXIT", mainMenu);
        editorSubMenu->AddChild (exitMenu);

        MenuItem* objectSubMenu = new MenuItem ("OBJECT", mainMenu);

        mainMenu->AddItem (editorSubMenu);
        mainMenu->AddItem (objectSubMenu);

        m_UIManager.AddWidget (mainMenu);

        Button* button = new Button (&m_UIManager, Point{ 30, 200 }, "MENU");
        button->SetBackgroundColor (COLOR_GREEN);
        button->SetTextColor (COLOR_BLACK);
        button->SetBorderColor (COLOR_WHITE);
        button->SetDrawBorder (true);

        m_UIManager.AddWidget (button);

        ButtonImage* buttonImage = new ButtonImage (&m_UIManager, Point (100, 100), GetDataPath () + "gfx/crate_sprite.png");
        buttonImage->SetSize (100, 100);
        m_UIManager.AddWidget (buttonImage);

        int tilesCount = 8;
        const Point& screenSize = m_Screen->GetScreenSize ();
        double centerX = screenSize.Width * 0.5;
        double beginning = centerX - (tilesCount - 1) * 0.5 * TILE_SIZE - TILE_SIZE * 0.5;

        //  Back frame
        for (int i = 0; i < tilesCount; ++i)
        {
            float advance = beginning + i * TILE_SIZE;

            Frame* frame =
              new Frame (&m_UIManager, Rect{ { advance, screenSize.Height - TILE_SIZE }, { TILE_SIZE, TILE_SIZE } }, true, 1.0);
            frame->SetBorderColor (COLOR_GREEN);
            frame->SetDrawBorder (true);

            m_UIManager.AddWidget (frame);
        }
    }

    //--------------------------------------------------------------------------------------------------
}
