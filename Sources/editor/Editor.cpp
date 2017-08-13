// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "Editor.h"
#include "MainLoop.h"
#include "Screen.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    const int TILES_COUNT = 8;
    const int TILE_SIZE = 90;

    //--------------------------------------------------------------------------------------------------
    //--------------------------------------------------------------------------------------------------

    Editor::Editor (MainLoop* mainLoop)
      : m_MainLoop (mainLoop)
      , m_Renderer (nullptr)
      , m_Skin (nullptr)
      , m_MainCanvas (nullptr)
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
        InitializeUI ();

        Lifecycle::Initialize ();
    }

    //--------------------------------------------------------------------------------------------------

    bool Editor::Destroy () { Lifecycle::Destroy (); }

    //--------------------------------------------------------------------------------------------------

    bool Editor::Update (double deltaTime) { return true; }

    //--------------------------------------------------------------------------------------------------

    void Editor::ProcessEvent (ALLEGRO_EVENT* event, double deltaTime)
    {
        m_GworkInput.ProcessMessage (*event);

        if (event->type == ALLEGRO_EVENT_KEY_UP)
        {
            if (event->keyboard.keycode == ALLEGRO_KEY_F5)
            {
                MenuItemPlay ();
            }
        }
        if (event->type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN)
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
        else if (event->type == ALLEGRO_EVENT_DISPLAY_RESIZE)
        {
            m_MainCanvas->SetSize (event->display.width, event->display.height);
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::Render (double deltaTime)
    {
        m_MainCanvas->RenderCanvas ();

        if (m_DrawTiles)
        {
            DrawTiles ();
        }

        m_Atlas.DrawRegion ("MAIN", 200, 200);
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::DrawTiles () {}

    //--------------------------------------------------------------------------------------------------

    void Editor::InitializeUI ()
    {
        m_Renderer = new Gwk::Renderer::Allegro ();

        m_Skin = new Gwk::Skin::TexturedBase (m_Renderer);
        m_Skin->SetRender (m_Renderer);
        m_Skin->Init (GetResourcePath (GFX_DEFAULT_SKIN));

        // The fonts work differently in Allegro - it can't use
        // system fonts. So force the skin to use a local one.
        m_Skin->SetDefaultFont (GetResourcePath (FONT_EDITOR), 13);

        // Create a Canvas (it's root, on which all other Gwork panels are created)
        const Point screenSize = m_MainLoop->GetScreen ()->GetScreenSize ();

        m_MainCanvas = new Gwk::Controls::Canvas (m_Skin);
        m_MainCanvas->SetSize (screenSize.Width, screenSize.Height);
        // m_MainCanvas->SetDrawBackground (true);
        m_MainCanvas->SetBackgroundColor (Gwk::Color (150, 170, 170, 255));

        m_GworkInput.Initialize (m_MainCanvas);

        Gwk::Controls::MenuStrip* menu = new Gwk::Controls::MenuStrip (m_MainCanvas);
        {
            Gwk::Controls::MenuItem* editorMenu = menu->AddItem ("EDITOR");
            {
                Gwk::Controls::MenuItem* exitMenu = editorMenu->GetMenu ()->AddItem ("EXIT")->SetAction (this, &Editor::OnMenuItemExit);
            }

            Gwk::Controls::MenuItem* objectMenu = menu->AddItem ("OBJECT");

            Gwk::Controls::MenuItem* gameMenu = menu->AddItem ("GAME");
            {
                Gwk::Controls::MenuItem* playMenu =
                  gameMenu->GetMenu ()->AddItem ("PLAY", "", "F5")->SetAction (this, &Editor::OnMenuItemPlay);
            }
        }

        m_Atlas.LoadFromFile (GetResourcePath (PACK_0_0_HOME));

        double beginning = screenSize.Width * 0.5 - (TILES_COUNT - 1) * 0.5 * TILE_SIZE - TILE_SIZE * 0.5;
        float advance = 0;

        //  Back frame
        for (int i = 0; i < TILES_COUNT; ++i)
        {
            advance = beginning + i * TILE_SIZE;

            Gwk::Controls::Rectangle* rect = new Gwk::Controls::Rectangle (m_MainCanvas);
            rect->SetColor (Gwk::Color (0, 255, 0, 255));
            rect->SetShouldDrawBackground (false);
            rect->SetBounds (advance, screenSize.Height - TILE_SIZE - 2, TILE_SIZE, TILE_SIZE);

            Gwk::Controls::ImagePanel* imagePanel = new Gwk::Controls::ImagePanel (m_MainCanvas);
            const Gwk::Rect bounds = rect->GetBounds ();
            imagePanel->SetBounds (bounds.x + 1, bounds.y + 1, bounds.w - 2, bounds.h - 2);
        }

        Gwk::Controls::Button* nextAssetsButton = new Gwk::Controls::Button (m_MainCanvas);
        nextAssetsButton->SetText (">>");
        nextAssetsButton->SetPos (advance + TILE_SIZE, screenSize.Height - TILE_SIZE - 2);
        nextAssetsButton->SetSize (40, 22);

        Gwk::Controls::Button* prevAssetsButton = new Gwk::Controls::Button (m_MainCanvas);
        prevAssetsButton->SetText ("<<");
        prevAssetsButton->SetPos (advance + TILE_SIZE, screenSize.Height - TILE_SIZE - 2 + nextAssetsButton->Height ());
        prevAssetsButton->SetSize (40, 22);
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::OnMenuItemPlay (Gwk::Event::Info info) { MenuItemPlay (); }

    //--------------------------------------------------------------------------------------------------

    void Editor::MenuItemPlay () { m_MainLoop->GetStateManager ()->SetActiveState ("GAMEPLAY_STATE"); }

    //--------------------------------------------------------------------------------------------------

    void Editor::OnMenuItemExit (Gwk::Event::Info info) { m_MainLoop->Exit (); }

    //--------------------------------------------------------------------------------------------------
}
