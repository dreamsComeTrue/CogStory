// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "Editor.h"
#include "MainLoop.h"
#include "Screen.h"
#include "ui/Button.h"
#include "ui/ButtonImage.h"
#include "ui/Frame.h"
#include "ui/Image.h"
#include "ui/Label.h"
#include "ui/Menu.h"
#include "ui/TextBox.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    const int TILE_SIZE = 64;

    //--------------------------------------------------------------------------------------------------
    //--------------------------------------------------------------------------------------------------

    Editor::Editor (MainLoop* mainLoop)
      : m_MainLoop (mainLoop)
      , m_UIManager (mainLoop->GetScreen ())
      , m_DrawTiles (true)
      , m_MousePan (false)
      , m_MouseWheel (false)
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

                for (Frame* frame : m_TilesFrames)
                {
                    frame->SetVisible (m_DrawTiles);
                }
            }
        }
        else if (event->type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN)
        {
            m_MousePan = event->mouse.button == 3;
        }
        else if (event->type == ALLEGRO_EVENT_MOUSE_BUTTON_UP)
        {
            m_MousePan = false;
        }
        else if (event->type == ALLEGRO_EVENT_MOUSE_AXES)
        {
            if (event->mouse.dz < 0.0)
            {
                m_MainLoop->GetSceneManager ()->GetCamera ().Scale (0.75f, 0.75f, event->mouse.x, event->mouse.y);
            }
            else if (event->mouse.dz > 0.0)
            {
                m_MainLoop->GetSceneManager ()->GetCamera ().Scale (1.25f, 1.25f, event->mouse.x, event->mouse.y);
            }

            if (m_MousePan)
            {
                m_MainLoop->GetSceneManager ()->GetCamera ().Move (event->mouse.dx, event->mouse.dy);
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
        const Point& screenSize = m_MainLoop->GetScreen ()->GetScreenSize ();
        double centerX = screenSize.Width * 0.5;
        double centerY = screenSize.Height * 0.5;

        Menu* mainMenu = new Menu (&m_UIManager, Point{ 4, 4 });
        MenuItem* editorSubMenu = new MenuItem ("EDITOR", mainMenu);

        MenuItem* newSceneMenu = new MenuItem ("NEW SCENE", mainMenu);
        newSceneMenu->OnClick = [&] { m_NewSceneTitle->SetVisible (!m_NewSceneTitle->IsVisible ()); };
        editorSubMenu->AddChild (newSceneMenu);

        MenuItem* exitMenu = new MenuItem ("EXIT", mainMenu);
        exitMenu->OnClick = [&] { m_MainLoop->Exit (); };
        editorSubMenu->AddChild (exitMenu);

        MenuItem* objectSubMenu = new MenuItem ("OBJECT", mainMenu);

        MenuItem* gameSubMenu = new MenuItem ("GAME", mainMenu);
        MenuItem* restartMenu = new MenuItem ("RESTART", mainMenu);
        gameSubMenu->AddChild (restartMenu);

        mainMenu->AddItem (editorSubMenu);
        mainMenu->AddItem (objectSubMenu);
        mainMenu->AddItem (gameSubMenu);

        m_UIManager.AddWidget (mainMenu);

        ButtonImage* buttonImage = new ButtonImage (&m_UIManager, Point (100, 100), GetDataPath () + "gfx/crate_sprite.png");
        buttonImage->SetSize (100, 100);
        m_UIManager.AddWidget (buttonImage);

        m_NewSceneTitle = new TextBox (&m_UIManager, { 300, 300 }, "Hello");
        m_UIManager.AddWidget (m_NewSceneTitle);
        m_NewSceneTitle->SetPosition (centerX - m_NewSceneTitle->GetSize ().Width * 0.5f,
                                      centerY - m_NewSceneTitle->GetSize ().Height * 0.5f);

        int tilesCount = 8;
        double beginning = centerX - (tilesCount - 1) * 0.5 * TILE_SIZE - TILE_SIZE * 0.5;

        //  Back frame
        for (int i = 0; i < tilesCount; ++i)
        {
            float advance = beginning + i * TILE_SIZE;

            Frame* frame =
              new Frame (&m_UIManager, Rect{ { advance, screenSize.Height - TILE_SIZE }, { TILE_SIZE, TILE_SIZE } }, true, 1.0);
            frame->SetBorderColor (COLOR_GREEN);
            frame->SetDrawBorder (true);

            m_TilesFrames[i] = frame;

            m_UIManager.AddWidget (frame);
        }
    }

    //--------------------------------------------------------------------------------------------------
}
