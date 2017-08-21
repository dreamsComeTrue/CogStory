// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "Editor.h"
#include "AtlasManager.h"
#include "MainLoop.h"
#include "Scene.h"
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
      , m_IsDrawTiles (true)
      , m_IsSnapToGrid (true)
      , m_IsMousePan (false)
      , m_IsMouseWheel (false)
      , m_Rotation (0)
      , m_BaseGridSize (16)
      , m_GridSize (16)
      , m_CursorMode (CursorMode::TileSelectMode)
      , m_SelectedTile (nullptr)
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

        if (event->type == ALLEGRO_EVENT_KEY_CHAR)
        {
            switch (event->keyboard.keycode)
            {
                case ALLEGRO_KEY_R:
                {
                    if (event->keyboard.modifiers == ALLEGRO_KEYMOD_SHIFT)
                    {
                        m_Rotation -= 15;
                    }
                    else
                    {
                        m_Rotation += 15;
                    }

                    if (m_Rotation <= -360)
                    {
                        m_Rotation = 0;
                    }

                    if (m_Rotation >= 360)
                    {
                        m_Rotation = 0;
                    }

                    Gwk::Controls::Label* angleLabel = (Gwk::Controls::Label*)m_MainCanvas->GetNamedChildren ("AngleLabel").list.front ();
                    angleLabel->SetText (std::string ("A: ") + ToString (m_Rotation));

                    break;
                }

                case ALLEGRO_KEY_G:
                {
                    if (event->keyboard.modifiers == ALLEGRO_KEYMOD_SHIFT)
                    {
                        m_BaseGridSize /= 2;
                    }
                    else
                    {
                        m_BaseGridSize *= 2;
                    }

                    m_BaseGridSize = std::max (1, std::min (m_BaseGridSize, 1024));

                    m_GridSize = m_BaseGridSize * m_MainLoop->GetSceneManager ()->GetCamera ().GetScale ().X;

                    Gwk::Controls::Label* gridSizeLabel = (Gwk::Controls::Label*)m_MainCanvas->GetNamedChildren ("GridSize").list.front ();
                    gridSizeLabel->SetText (std::string ("GRID: ") + ToString (m_BaseGridSize));

                    break;
                }

                case ALLEGRO_KEY_X:
                {
                    if (m_SelectedTile)
                    {
                        m_MainLoop->GetSceneManager ()->GetActiveScene ()->RemoveTile (*m_SelectedTile);
                        m_SelectedTile = nullptr;
                        m_CursorMode = CursorMode::TileSelectMode;
                    }

                    break;
                }
            }
        }
        if (event->type == ALLEGRO_EVENT_KEY_UP)
        {
            switch (event->keyboard.keycode)
            {
                case ALLEGRO_KEY_F5:
                {
                    MenuItemPlay ();
                    break;
                }

                case ALLEGRO_KEY_S:
                {
                    m_IsSnapToGrid = !m_IsSnapToGrid;
                    Gwk::Controls::Label* snapToGridLabel =
                      (Gwk::Controls::Label*)m_MainCanvas->GetNamedChildren ("SnapToGrid").list.front ();
                    snapToGridLabel->SetText (std::string ("SNAP: ") + (m_IsSnapToGrid ? "YES" : "NO"));
                    break;
                }
            }
        }
        if (event->type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN)
        {
            m_IsMousePan = event->mouse.button == 3;

            if (event->mouse.button == 1)
            {
                bool tileSelected = SelectTile (event->mouse.x, event->mouse.y);

                if (m_CursorMode == CursorMode::TileInsertMode && !tileSelected)
                {
                    AddTile (event->mouse.x, event->mouse.y);
                }

                if (m_CursorMode == CursorMode::TileSelectMode)
                {
                    Rect r;
                    m_SelectedTile = GetTileUnderCursor (event->mouse.x, event->mouse.y, std::move (r));

                    if (m_SelectedTile)
                    {
                        m_Rotation = m_SelectedTile->Rotation;
                        m_CursorMode = CursorMode::TileEditMode;
                    }
                }
                else if (m_CursorMode == CursorMode::TileEditMode)
                {
                    m_CursorMode = CursorMode::TileSelectMode;
                }
            }

            if (event->mouse.button == 2)
            {
                Gwk::Controls::Label* widthLabel = (Gwk::Controls::Label*)m_MainCanvas->GetNamedChildren ("WidthLabel").list.front ();
                widthLabel->SetText ("W: 0");

                Gwk::Controls::Label* heightLabel = (Gwk::Controls::Label*)m_MainCanvas->GetNamedChildren ("HeightLabel").list.front ();
                heightLabel->SetText ("H: 0");

                m_CursorMode = CursorMode::TileSelectMode;
            }
        }
        else if (event->type == ALLEGRO_EVENT_MOUSE_BUTTON_UP)
        {
            m_IsMousePan = false;
        }
        else if (event->type == ALLEGRO_EVENT_MOUSE_AXES)
        {
            if (event->mouse.dz < 0.0)
            {
                m_MainLoop->GetSceneManager ()->GetCamera ().Scale (0.75f, 0.75f, event->mouse.x, event->mouse.y);
                m_GridSize = m_BaseGridSize * m_MainLoop->GetSceneManager ()->GetCamera ().GetScale ().X;
            }
            else if (event->mouse.dz > 0.0)
            {
                m_MainLoop->GetSceneManager ()->GetCamera ().Scale (1.25f, 1.25f, event->mouse.x, event->mouse.y);
                m_GridSize = m_BaseGridSize * m_MainLoop->GetSceneManager ()->GetCamera ().GetScale ().X;
            }

            Gwk::Controls::Label* scaleLabel = (Gwk::Controls::Label*)m_MainCanvas->GetNamedChildren ("ScaleLabel").list.front ();
            scaleLabel->SetText (std::string ("S: ") + ToString (m_MainLoop->GetSceneManager ()->GetCamera ().GetScale ().X));

            if (m_IsMousePan)
            {
                m_MainLoop->GetSceneManager ()->GetCamera ().Move (event->mouse.dx, event->mouse.dy);
            }

            Point translate = m_MainLoop->GetSceneManager ()->GetCamera ().GetTranslate ();
            Point scale = m_MainLoop->GetSceneManager ()->GetCamera ().GetScale ();

            Gwk::Controls::Label* xPositionLabel = (Gwk::Controls::Label*)m_MainCanvas->GetNamedChildren ("XPositionLabel").list.front ();
            std::string xPosString = " X: " + ToString (translate.X * (1 / scale.X));
            xPositionLabel->SetText (xPosString);

            Gwk::Controls::Label* yPositionLabel = (Gwk::Controls::Label*)m_MainCanvas->GetNamedChildren ("YPositionLabel").list.front ();
            std::string yPosString = " Y: " + ToString (translate.Y * (1 / scale.Y));
            yPositionLabel->SetText (yPosString);
        }
        else if (event->type == ALLEGRO_EVENT_DISPLAY_RESIZE)
        {
            m_MainCanvas->SetSize (event->display.width, event->display.height);
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::Render (double deltaTime)
    {
        ALLEGRO_MOUSE_STATE state;
        al_get_mouse_state (&state);

        Point translate = m_MainLoop->GetSceneManager ()->GetCamera ().GetTranslate ();
        Point scale = m_MainLoop->GetSceneManager ()->GetCamera ().GetScale ();

        int finalX = state.x;
        int finalY = state.y;

        if (m_IsSnapToGrid)
        {
            finalX = (finalX / m_GridSize) * m_GridSize;
            finalY = (finalY / m_GridSize) * m_GridSize;
        }

        if (m_CursorMode == CursorMode::TileInsertMode)
        {
            m_Atlas->DrawRegion (m_SelectedAtlasRegion.Name, finalX, finalY, scale.X, scale.Y, DegressToRadians (m_Rotation));
        }

        if (m_CursorMode == CursorMode::TileSelectMode)
        {
            Rect r;
            Tile* tile = GetTileUnderCursor (state.x, state.y, std::move (r));

            if (tile)
            {
                al_draw_rectangle (r.TopLeft.X, r.TopLeft.Y, r.BottomRight.Width, r.BottomRight.Height, COLOR_YELLOW, 2);
            }
        }

        if (m_CursorMode == CursorMode::TileEditMode)
        {
            if (m_SelectedTile)
            {
                Rect b = m_SelectedTile->Bounds;
                int width = b.BottomRight.Width * 0.5;
                int height = b.BottomRight.Height * 0.5;

                m_SelectedTile->Rotation = m_Rotation;
                m_SelectedTile->Bounds.TopLeft = { (translate.X + finalX) * (1 / scale.X), (translate.Y + finalY) * (1 / scale.Y) };

                int x1 = (b.TopLeft.X - translate.X * (1 / scale.X) - width) * (scale.X);
                int y1 = (b.TopLeft.Y - translate.Y * (1 / scale.Y) - height) * (scale.Y);
                int x2 = (b.TopLeft.X - translate.X * (1 / scale.X) + width) * (scale.X);
                int y2 = (b.TopLeft.Y - translate.Y * (1 / scale.Y) + height) * (scale.Y);

                al_draw_rectangle (x1, y1, x2, y2, COLOR_RED, 2);
            }
        }

        if (m_IsDrawTiles)
        {
            DrawTiles ();
        }

        m_MainCanvas->RenderCanvas ();
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::DrawTiles ()
    {
        std::vector<AtlasRegion>& regions = m_Atlas->GetRegions ();
        const Point screenSize = m_MainLoop->GetScreen ()->GetScreenSize ();
        double beginning = screenSize.Width * 0.5 - (TILES_COUNT - 1) * 0.5 * TILE_SIZE - TILE_SIZE * 0.5;
        float advance = 0;

        for (int i = 0; i < TILES_COUNT; ++i)
        {
            if (i >= regions.size () - 1)
            {
                break;
            }

            advance = beginning + i * TILE_SIZE;

            Rect region = regions[i].Bounds;

            al_draw_scaled_bitmap (m_Atlas->GetImage (),
                                   region.TopLeft.X,
                                   region.TopLeft.Y,
                                   region.BottomRight.Width,
                                   region.BottomRight.Height,
                                   advance,
                                   screenSize.Height - TILE_SIZE - 1,
                                   TILE_SIZE,
                                   TILE_SIZE - 2,
                                   0);
        }
    }

    //--------------------------------------------------------------------------------------------------

    bool Editor::SelectTile (int mouseX, int mouseY)
    {
        const Point screenSize = m_MainLoop->GetScreen ()->GetScreenSize ();
        double beginning = screenSize.Width * 0.5 - (TILES_COUNT - 1) * 0.5 * TILE_SIZE - TILE_SIZE * 0.5;
        float advance = 0;
        std::vector<AtlasRegion> regions = m_Atlas->GetRegions ();

        for (int i = 0; i < TILES_COUNT; ++i)
        {
            if (i >= regions.size () - 1)
            {
                break;
            }

            advance = beginning + i * TILE_SIZE;
            double x = advance;
            double y = screenSize.Height - TILE_SIZE - 1;
            Rect r = Rect{ { x, y }, { x + TILE_SIZE, y + TILE_SIZE - 2 } };

            if (InsideRect (mouseX, mouseY, r))
            {
                m_CursorMode = CursorMode::TileInsertMode;
                m_SelectedAtlasRegion = regions[i];

                Gwk::Controls::Label* widthLabel = (Gwk::Controls::Label*)m_MainCanvas->GetNamedChildren ("WidthLabel").list.front ();
                std::string widthString = "W: " + ToString (m_SelectedAtlasRegion.Bounds.BottomRight.Width);
                widthLabel->SetText (widthString);

                Gwk::Controls::Label* heightLabel = (Gwk::Controls::Label*)m_MainCanvas->GetNamedChildren ("HeightLabel").list.front ();
                std::string heightString = "H: " + ToString (m_SelectedAtlasRegion.Bounds.BottomRight.Height);
                heightLabel->SetText (heightString);

                return true;
            }
        }

        return false;
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::AddTile (int mouseX, int mouseY)
    {
        Tile tile;

        Point translate = m_MainLoop->GetSceneManager ()->GetCamera ().GetTranslate ();
        Point scale = m_MainLoop->GetSceneManager ()->GetCamera ().GetScale ();
        AtlasRegion region = m_Atlas->GetRegion (m_SelectedAtlasRegion.Name);

        int finalX = mouseX;
        int finalY = mouseY;

        if (m_IsSnapToGrid)
        {
            finalX = (finalX / m_GridSize) * m_GridSize;
            finalY = (finalY / m_GridSize) * m_GridSize;
        }

        tile.Tileset = m_Atlas->GetName ();
        tile.Name = m_SelectedAtlasRegion.Name;
        tile.Bounds = { { (translate.X + finalX) * (1 / scale.X), (translate.Y + finalY) * (1 / scale.Y) },
                        { region.Bounds.BottomRight.Width, region.Bounds.BottomRight.Height } };
        tile.Rotation = m_Rotation;

        m_MainLoop->GetSceneManager ()->GetActiveScene ()->AddTile (tile);
    }

    //--------------------------------------------------------------------------------------------------

    Tile* Editor::GetTileUnderCursor (int mouseX, int mouseY, Rect&& outRect)
    {
        Point translate = m_MainLoop->GetSceneManager ()->GetCamera ().GetTranslate ();
        Point scale = m_MainLoop->GetSceneManager ()->GetCamera ().GetScale ();

        std::vector<Tile>& tiles = m_MainLoop->GetSceneManager ()->GetActiveScene ()->GetTiles ();

        for (Tile& tile : tiles)
        {
            Rect b = tile.Bounds;
            int width = b.BottomRight.Width * 0.5;
            int height = b.BottomRight.Height * 0.5;

            int x1 = (b.TopLeft.X - translate.X * (1 / scale.X) - width) * (scale.X);
            int y1 = (b.TopLeft.Y - translate.Y * (1 / scale.Y) - height) * (scale.Y);
            int x2 = (b.TopLeft.X - translate.X * (1 / scale.X) + width) * (scale.X);
            int y2 = (b.TopLeft.Y - translate.Y * (1 / scale.Y) + height) * (scale.Y);

            Rect r = { { x1, y1 }, { x2, y2 } };

            if (InsideRect (mouseX, mouseY, r))
            {
                outRect = r;
                return &tile;
            }
        }

        return nullptr;
    }

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

        const Point screenSize = m_MainLoop->GetScreen ()->GetScreenSize ();

        // Create a canvas (it's root, on which all other Gwork panels are created)
        m_MainCanvas = new Gwk::Controls::Canvas (m_Skin);
        m_MainCanvas->SetSize (screenSize.Width, screenSize.Height);
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

        m_Atlas = m_MainLoop->GetSceneManager ()->GetAtlasManager ()->GetAtlas (GetBaseName (GetResourcePath (PACK_0_0_HOME)));

        double beginning = screenSize.Width * 0.5 - (TILES_COUNT - 1) * 0.5 * TILE_SIZE - TILE_SIZE * 0.5;
        float advance = 0;

        //  Back frame
        for (int i = 0; i < TILES_COUNT; ++i)
        {
            advance = beginning + i * TILE_SIZE;

            std::ostringstream name;
            name << "TileRect" << i;

            Gwk::Controls::Rectangle* rect = new Gwk::Controls::Rectangle (m_MainCanvas, name.str ());
            rect->SetColor (Gwk::Color (0, 255, 0, 255));
            rect->SetShouldDrawBackground (false);
            rect->SetBounds (advance, screenSize.Height - TILE_SIZE - 2, TILE_SIZE, TILE_SIZE);
        }

        Gwk::Controls::Button* nextAssetsButton = new Gwk::Controls::Button (m_MainCanvas);
        nextAssetsButton->SetText (">>");
        nextAssetsButton->SetPos (advance + TILE_SIZE, screenSize.Height - TILE_SIZE - 2);
        nextAssetsButton->SetSize (40, 22);

        Gwk::Controls::Button* prevAssetsButton = new Gwk::Controls::Button (m_MainCanvas);
        prevAssetsButton->SetText ("<<");
        prevAssetsButton->SetPos (advance + TILE_SIZE, screenSize.Height - TILE_SIZE - 2 + nextAssetsButton->Height ());
        prevAssetsButton->SetSize (40, 22);

        int sideOffset = 70;
        int labelHeight = 15;
        Gwk::Controls::Label* xPositionLabel = new Gwk::Controls::Label (m_MainCanvas, "XPositionLabel");
        xPositionLabel->SetText (" X:");
        xPositionLabel->SetTextColor (Gwk::Color (0, 255, 0));
        xPositionLabel->SetHeight (labelHeight);
        xPositionLabel->SetPos (screenSize.Width - sideOffset, 25);

        Gwk::Controls::Label* yPositionLabel = new Gwk::Controls::Label (m_MainCanvas, "YPositionLabel");
        yPositionLabel->SetText (" Y:");
        yPositionLabel->SetTextColor (Gwk::Color (0, 255, 0));
        yPositionLabel->SetHeight (labelHeight);
        yPositionLabel->SetPos (screenSize.Width - sideOffset, xPositionLabel->Bottom ());

        Gwk::Controls::Label* widthLabel = new Gwk::Controls::Label (m_MainCanvas, "WidthLabel");
        widthLabel->SetText ("W:");
        widthLabel->SetTextColor (Gwk::Color (0, 255, 0));
        widthLabel->SetHeight (labelHeight);
        widthLabel->SetPos (screenSize.Width - sideOffset, yPositionLabel->Bottom ());

        Gwk::Controls::Label* heightLabel = new Gwk::Controls::Label (m_MainCanvas, "HeightLabel");
        heightLabel->SetText ("H:");
        heightLabel->SetTextColor (Gwk::Color (0, 255, 0));
        heightLabel->SetHeight (labelHeight);
        heightLabel->SetPos (screenSize.Width - sideOffset, widthLabel->Bottom ());

        Gwk::Controls::Label* angleLabel = new Gwk::Controls::Label (m_MainCanvas, "AngleLabel");
        angleLabel->SetText (std::string ("A: ") + ToString (m_Rotation));
        angleLabel->SetTextColor (Gwk::Color (0, 255, 0));
        angleLabel->SetHeight (labelHeight);
        angleLabel->SetPos (screenSize.Width - sideOffset, heightLabel->Bottom ());

        Gwk::Controls::Label* scaleLabel = new Gwk::Controls::Label (m_MainCanvas, "ScaleLabel");
        scaleLabel->SetText (std::string ("S: ") + ToString (m_MainLoop->GetSceneManager ()->GetCamera ().GetScale ().X));
        scaleLabel->SetTextColor (Gwk::Color (0, 255, 0));
        scaleLabel->SetHeight (labelHeight);
        scaleLabel->SetPos (screenSize.Width - sideOffset, angleLabel->Bottom ());

        Gwk::Controls::Label* snapToGridLabel = new Gwk::Controls::Label (m_MainCanvas, "SnapToGrid");
        snapToGridLabel->SetText (std::string ("SNAP: ") + (m_IsSnapToGrid ? "YES" : "NO"));
        snapToGridLabel->SetTextColor (Gwk::Color (0, 255, 0));
        snapToGridLabel->SetHeight (labelHeight);
        snapToGridLabel->SetPos (screenSize.Width - sideOffset, scaleLabel->Bottom ());

        Gwk::Controls::Label* gridLabel = new Gwk::Controls::Label (m_MainCanvas, "GridSize");
        gridLabel->SetText (std::string ("GRID: ") + ToString (m_GridSize));
        gridLabel->SetTextColor (Gwk::Color (0, 255, 0));
        gridLabel->SetHeight (labelHeight);
        gridLabel->SetPos (screenSize.Width - sideOffset, snapToGridLabel->Bottom ());
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::OnMenuItemPlay (Gwk::Event::Info info) { MenuItemPlay (); }

    //--------------------------------------------------------------------------------------------------

    void Editor::MenuItemPlay () { m_MainLoop->GetStateManager ()->SetActiveState ("GAMEPLAY_STATE"); }

    //--------------------------------------------------------------------------------------------------

    void Editor::OnMenuItemExit (Gwk::Event::Info info) { m_MainLoop->Exit (); }

    //--------------------------------------------------------------------------------------------------

    void Editor::OnTileSelected (Gwk::Event::Info info) {}

    //--------------------------------------------------------------------------------------------------
}
