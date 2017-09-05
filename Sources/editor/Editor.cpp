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
        , m_TileUnderCursor (nullptr)
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
        bool tileSelected = false;

        if (event->type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN)
        {
            if (event->mouse.button == 1)
            {
                tileSelected = ChooseTile (event->mouse.x, event->mouse.y);
            }
        }

        bool guiProcessed = m_GworkInput.ProcessMessage (*event);

        if (guiProcessed)
        {
            return;
        }

        if (event->type == ALLEGRO_EVENT_KEY_CHAR)
        {
            switch (event->keyboard.keycode)
            {
            case ALLEGRO_KEY_R:
            {
                m_Rotation += event->keyboard.modifiers == ALLEGRO_KEYMOD_SHIFT ? 15 : -15;

                if (m_Rotation <= -360)
                {
                    m_Rotation = 0;
                }

                if (m_Rotation >= 360)
                {
                    m_Rotation = 0;
                }

                UpdateLabels ();

                break;
            }

            case ALLEGRO_KEY_G:
            {
                m_BaseGridSize *= event->keyboard.modifiers == ALLEGRO_KEYMOD_SHIFT ? 0.5 : 2;
                m_BaseGridSize = std::max (1, std::min (m_BaseGridSize, 1024));
                m_GridSize
                    = std::max (1.0, m_BaseGridSize * m_MainLoop->GetSceneManager ()->GetCamera ().GetScale ().X);

                UpdateLabels ();

                break;
            }

            case ALLEGRO_KEY_Z:
            {
                int zOrder = 0;

                if (m_SelectedTile)
                {
                    zOrder = m_SelectedTile->ZOrder;

                    if (event->keyboard.modifiers == ALLEGRO_KEYMOD_SHIFT)
                    {
                        zOrder = std::max (0, --zOrder);
                    }
                    else
                    {
                        ++zOrder;
                    }

                    m_SelectedTile->ZOrder = zOrder;

                    m_MainLoop->GetSceneManager ()->GetActiveScene ()->SortTiles ();
                }

                Gwk::Controls::Label* gridSizeLabel
                    = (Gwk::Controls::Label*)m_MainCanvas->GetNamedChildren ("ZOrderLabel").list.front ();
                gridSizeLabel->SetText (std::string ("ZORD: ") + ToString (zOrder));

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

            case ALLEGRO_KEY_C:
            {
                if (m_TileUnderCursor)
                {
                    m_SelectedAtlasRegion = m_Atlas->GetRegion (m_TileUnderCursor->Name);
                    m_SelectedTile = nullptr;
                    m_CursorMode = CursorMode::TileInsertMode;
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

            case ALLEGRO_KEY_SPACE:
            {
                m_IsDrawTiles = !m_IsDrawTiles;

                for (int i = 0; i < TILES_COUNT; ++i)
                {
                    std::ostringstream name;
                    name << "TileRect" << i;

                    Gwk::Controls::Rectangle* rect
                        = (Gwk::Controls::Rectangle*)m_MainCanvas->GetNamedChildren (name.str ()).list.front ();
                    rect->SetHidden (!m_IsDrawTiles);

                    Gwk::Controls::Button* nextAssetsButton
                        = (Gwk::Controls::Button*)m_MainCanvas->GetNamedChildren ("NextAssetsButton").list.front ();
                    nextAssetsButton->SetHidden (!m_IsDrawTiles);

                    Gwk::Controls::Button* prevAssetsButton
                        = (Gwk::Controls::Button*)m_MainCanvas->GetNamedChildren ("PrevAssetsButton").list.front ();
                    prevAssetsButton->SetHidden (!m_IsDrawTiles);
                }

                break;
            }

            case ALLEGRO_KEY_S:
            {
                m_IsSnapToGrid = !m_IsSnapToGrid;
                UpdateLabels ();
                break;
            }
            }
        }

        if (event->type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN)
        {
            m_IsMousePan = event->mouse.button == 3;

            if (event->mouse.button == 1)
            {
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
                        m_CursorMode = CursorMode::TileEditMode;
                        m_Rotation = m_SelectedTile->Rotation;

                        UpdateLabels ();
                    }
                }
                else if (m_CursorMode == CursorMode::TileEditMode)
                {
                    m_CursorMode = CursorMode::TileSelectMode;
                }
            }

            if (event->mouse.button == 2)
            {
                Gwk::Controls::Label* widthLabel
                    = (Gwk::Controls::Label*)m_MainCanvas->GetNamedChildren ("WidthLabel").list.front ();
                widthLabel->SetText ("W: 0");

                Gwk::Controls::Label* heightLabel
                    = (Gwk::Controls::Label*)m_MainCanvas->GetNamedChildren ("HeightLabel").list.front ();
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
                m_GridSize
                    = std::max (1.0, m_BaseGridSize * m_MainLoop->GetSceneManager ()->GetCamera ().GetScale ().X);
                UpdateLabels ();
            }
            else if (event->mouse.dz > 0.0)
            {
                m_MainLoop->GetSceneManager ()->GetCamera ().Scale (1.25f, 1.25f, event->mouse.x, event->mouse.y);
                m_GridSize
                    = std::max (1.0, m_BaseGridSize * m_MainLoop->GetSceneManager ()->GetCamera ().GetScale ().X);
            }

            if (m_IsMousePan)
            {
                m_MainLoop->GetSceneManager ()->GetCamera ().Move (event->mouse.dx, event->mouse.dy);
            }

            UpdateLabels ();
        }
        else if (event->type == ALLEGRO_EVENT_DISPLAY_RESIZE)
        {
            m_MainCanvas->SetSize (event->display.width, event->display.height);
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::Render (double deltaTime)
    {
        if (m_IsSnapToGrid)
        {
            DrawGrid ();
        }

        m_MainLoop->GetSceneManager ()->Render (deltaTime);

        if (m_IsDrawTiles)
        {
            DrawTiles ();
        }

        ALLEGRO_MOUSE_STATE state;
        al_get_mouse_state (&state);

        m_MainLoop->GetSceneManager ()->GetCamera ().UseIdentityTransform ();

        Point translate = m_MainLoop->GetSceneManager ()->GetCamera ().GetTranslate ();
        Point scale = m_MainLoop->GetSceneManager ()->GetCamera ().GetScale ();
        Point point = CalculateCursorPoint (state.x, state.y);

        if (m_CursorMode == CursorMode::TileInsertMode)
        {
            m_Atlas->DrawRegion (
                m_SelectedAtlasRegion.Name, point.X, point.Y, scale.X, scale.Y, DegressToRadians (m_Rotation));
        }

        if (m_CursorMode == CursorMode::TileSelectMode)
        {
            Rect r;
            m_TileUnderCursor = GetTileUnderCursor (state.x, state.y, std::move (r));

            if (m_TileUnderCursor)
            {
                al_draw_rectangle (
                    r.TopLeft.X, r.TopLeft.Y, r.BottomRight.Width, r.BottomRight.Height, COLOR_YELLOW, 2);
            }
        }

        if (m_CursorMode == CursorMode::TileEditMode)
        {
            if (m_SelectedTile)
            {
                m_SelectedTile->Rotation = m_Rotation;
                m_SelectedTile->Bounds.TopLeft = { (translate.X + point.X), (translate.Y + point.Y) };

                Rect b = GetRenderBounds (*m_SelectedTile);
                al_draw_rectangle (b.TopLeft.X, b.TopLeft.Y, b.BottomRight.Width, b.BottomRight.Height, COLOR_RED, 2);
            }
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

            al_draw_scaled_bitmap (m_Atlas->GetImage (), region.TopLeft.X, region.TopLeft.Y, region.BottomRight.Width,
                region.BottomRight.Height, advance, screenSize.Height - TILE_SIZE - 1, TILE_SIZE, TILE_SIZE - 2, 0);
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::DrawGrid ()
    {
        const ALLEGRO_COLOR LIGHT_GRAY{ 0.5f, 0.5f, 0.5f, 1.0f };

        Point t = m_MainLoop->GetSceneManager ()->GetCamera ().GetTranslate ();
        Point scale = m_MainLoop->GetSceneManager ()->GetCamera ().GetScale ();

        int horBeginX = t.X / 2 * (1 / scale.X);

        if (t.X > 0)
        {
            horBeginX *= -1;
        }

        int horBeginY = t.Y / 1 * (1 / scale.Y);

        if (t.Y > 0)
        {
            horBeginY *= -1;
        }

        int horEndX = 300 + std::fabs (horBeginX);
        int horEndY = 1000 + std::fabs (horBeginY);

        for (int i = horBeginX; i < horEndX; ++i)
        {
            float xOffset = i * m_GridSize - t.X;

            //  |
            al_draw_line (xOffset, horBeginY, xOffset, horEndY, LIGHT_GRAY, 1);
        }

        horEndX = 1000 + std::fabs (horBeginX);
        horEndY = 300 + std::fabs (horBeginY);

        for (int i = horBeginY; i < horEndY; ++i)
        {
            float yOffset = i * m_GridSize - t.Y;

            //  --
            al_draw_line (horBeginX, yOffset, horEndX, yOffset, LIGHT_GRAY, 1);
        }
    }

    //--------------------------------------------------------------------------------------------------

    bool Editor::ChooseTile (int mouseX, int mouseY)
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

                UpdateLabels ();

                return true;
            }
        }

        return false;
    }

    //--------------------------------------------------------------------------------------------------

    Point Editor::CalculateCursorPoint (int mouseX, int mouseY)
    {
        Point translate = m_MainLoop->GetSceneManager ()->GetCamera ().GetTranslate ();
        Point scale = m_MainLoop->GetSceneManager ()->GetCamera ().GetScale ();
        int finalX = (mouseX + translate.X);
        int finalY = (mouseY + translate.Y);

        if (m_IsSnapToGrid)
        {
            finalX = std::floor ((finalX + m_GridSize * 0.5) / m_GridSize) * m_GridSize;
            finalY = std::floor ((finalY + m_GridSize * 0.5) / m_GridSize) * m_GridSize;
        }

        finalX = (finalX - translate.X);
        finalY = (finalY - translate.Y);

        return { finalX, finalY };
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::AddTile (int mouseX, int mouseY)
    {
        Tile tile;
        AtlasRegion region = m_Atlas->GetRegion (m_SelectedAtlasRegion.Name);
        Point translate = m_MainLoop->GetSceneManager ()->GetCamera ().GetTranslate ();
        Point point = CalculateCursorPoint (mouseX, mouseY);

        tile.Tileset = m_Atlas->GetName ();
        tile.Name = m_SelectedAtlasRegion.Name;
        tile.Bounds = { { (translate.X + point.X), (translate.Y + point.Y) },
            { region.Bounds.BottomRight.Width, region.Bounds.BottomRight.Height } };
        tile.Rotation = m_Rotation;

        m_MainLoop->GetSceneManager ()->GetActiveScene ()->AddTile (tile);
    }

    //--------------------------------------------------------------------------------------------------

    Tile* Editor::GetTileUnderCursor (int mouseX, int mouseY, Rect&& outRect)
    {
        std::vector<Tile>& tiles = m_MainLoop->GetSceneManager ()->GetActiveScene ()->GetTiles ();
        Tile* result = nullptr;

        for (Tile& tile : tiles)
        {
            Rect r = GetRenderBounds (tile);

            if (InsideRect (mouseX, mouseY, r))
            {
                if ((result == nullptr)
                    || (result && (result->ZOrder < tile.ZOrder) || (result->RenderID < tile.RenderID)))
                {
                    outRect = r;
                    result = &tile;
                }
            }
        }

        return result;
    }

    //--------------------------------------------------------------------------------------------------

    Point RotatePoint (float x, float y, const Point& origin, float angle)
    {
        float s = std::sin (DegressToRadians (angle));
        float c = std::cos (DegressToRadians (angle));

        // translate point back to origin:
        x -= origin.X;
        y -= origin.Y;

        double nx = (x * c) - (y * s);
        double ny = (x * s) + (y * c);

        // translate point back:
        return { nx + origin.X, ny + origin.Y };
    }

    //--------------------------------------------------------------------------------------------------

    Rect Editor::GetRenderBounds (Tile& tile)
    {
        Point translate = m_MainLoop->GetSceneManager ()->GetCamera ().GetTranslate ();
        Point scale = m_MainLoop->GetSceneManager ()->GetCamera ().GetScale ();

        Rect b = tile.Bounds;
        int width = b.BottomRight.Width * 0.5;
        int height = b.BottomRight.Height * 0.5;

        float x1 = (b.TopLeft.X - translate.X * (1 / scale.X) - width) * (scale.X);
        float y1 = (b.TopLeft.Y - translate.Y * (1 / scale.Y) - height) * (scale.Y);
        float x2 = (b.TopLeft.X - translate.X * (1 / scale.X) + width) * (scale.X);
        float y2 = (b.TopLeft.Y - translate.Y * (1 / scale.Y) + height) * (scale.Y);

        Point origin = { x1 + (x2 - x1) * 0.5, y1 + (y2 - y1) * 0.5 };
        Point pointA = RotatePoint (x1, y1, origin, tile.Rotation);
        Point pointB = RotatePoint (x1, y2, origin, tile.Rotation);
        Point pointC = RotatePoint (x2, y1, origin, tile.Rotation);
        Point pointD = RotatePoint (x2, y2, origin, tile.Rotation);

        float minX, minY, maxX, maxY;

        minX = std::min (pointA.X, std::min (pointB.X, std::min (pointC.X, pointD.X)));
        minY = std::min (pointA.Y, std::min (pointB.Y, std::min (pointC.Y, pointD.Y)));
        maxX = std::max (pointA.X, std::max (pointB.X, std::max (pointC.X, pointD.X)));
        maxY = std::max (pointA.Y, std::max (pointB.Y, std::max (pointC.Y, pointD.Y)));

        return { { minX, minY }, { maxX, maxY } };
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
                Gwk::Controls::MenuItem* exitMenu
                    = editorMenu->GetMenu ()->AddItem ("EXIT")->SetAction (this, &Editor::OnMenuItemExit);
            }

            Gwk::Controls::MenuItem* objectMenu = menu->AddItem ("OBJECT");

            Gwk::Controls::MenuItem* gameMenu = menu->AddItem ("GAME");
            {
                Gwk::Controls::MenuItem* playMenu
                    = gameMenu->GetMenu ()->AddItem ("PLAY", "", "F5")->SetAction (this, &Editor::OnMenuItemPlay);
            }
        }

        m_Atlas = m_MainLoop->GetSceneManager ()->GetAtlasManager ()->GetAtlas (
            GetBaseName (GetResourcePath (PACK_0_0_HOME)));

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

        Gwk::Controls::Button* nextAssetsButton = new Gwk::Controls::Button (m_MainCanvas, "NextAssetsButton");
        nextAssetsButton->SetText (">>");
        nextAssetsButton->SetPos (advance + TILE_SIZE, screenSize.Height - TILE_SIZE - 2);
        nextAssetsButton->SetSize (40, 22);

        Gwk::Controls::Button* prevAssetsButton = new Gwk::Controls::Button (m_MainCanvas, "PrevAssetsButton");
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

        Gwk::Controls::Label* zOrderLabel = new Gwk::Controls::Label (m_MainCanvas, "ZOrderLabel");
        zOrderLabel->SetText (std::string ("ZORD: ") + ToString (0));
        zOrderLabel->SetTextColor (Gwk::Color (0, 255, 0));
        zOrderLabel->SetHeight (labelHeight);
        zOrderLabel->SetPos (screenSize.Width - sideOffset, angleLabel->Bottom ());

        Gwk::Controls::Label* scaleLabel = new Gwk::Controls::Label (m_MainCanvas, "ScaleLabel");
        scaleLabel->SetText (
            std::string ("S: ") + ToString (m_MainLoop->GetSceneManager ()->GetCamera ().GetScale ().X));
        scaleLabel->SetTextColor (Gwk::Color (0, 255, 0));
        scaleLabel->SetHeight (labelHeight);
        scaleLabel->SetPos (screenSize.Width - sideOffset, zOrderLabel->Bottom ());

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

        Gwk::Controls::Button* resetTranslateButton = new Gwk::Controls::Button (m_MainCanvas, "ResetTranslateButton");
        resetTranslateButton->SetText ("ZERO MOVE");
        resetTranslateButton->SetPos (10, 30);
        resetTranslateButton->onPress.Add (this, &Editor::OnResetTranslate);

        Gwk::Controls::Button* resetScaleButton = new Gwk::Controls::Button (m_MainCanvas, "ResetScaleButton");
        resetScaleButton->SetText ("ZERO SCALE");
        resetScaleButton->SetPos (10, resetTranslateButton->Bottom () + 5);
        resetScaleButton->onPress.Add (this, &Editor::OnResetScale);

        Gwk::Controls::Button* showGridButton = new Gwk::Controls::Button (m_MainCanvas, "ShowGridButton");
        showGridButton->SetText ("SHOW GRID");
        showGridButton->SetIsToggle (true);
        showGridButton->SetPos (10, resetScaleButton->Bottom () + 20);
        showGridButton->onToggleOn.Add (this, &Editor::OnShowGrid);
        showGridButton->onToggleOff.Add (this, &Editor::OnHideGrid);
        showGridButton->Toggle ();

        Gwk::Controls::Button* gridIncreaseButton = new Gwk::Controls::Button (m_MainCanvas, "GridIncreaseButton");
        gridIncreaseButton->SetText ("++");
        gridIncreaseButton->SetWidth (48);
        gridIncreaseButton->SetPos (10, showGridButton->Bottom () + 5);
        gridIncreaseButton->onPress.Add (this, &Editor::OnGridIncrease);

        Gwk::Controls::Button* gridDecreaseButton = new Gwk::Controls::Button (m_MainCanvas, "GridDecreaseButton");
        gridDecreaseButton->SetText ("--");
        gridDecreaseButton->SetWidth (47);
        gridDecreaseButton->SetPos (gridIncreaseButton->Right () + 5, showGridButton->Bottom () + 5);
        gridDecreaseButton->onPress.Add (this, &Editor::OnGridDecrease);
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

    void Editor::OnResetTranslate (Gwk::Controls::Base* control)
    {
        m_MainLoop->GetSceneManager ()->GetCamera ().SetOffset (0, 0);
        UpdateLabels ();
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::OnResetScale (Gwk::Controls::Base* control)
    {
        ALLEGRO_MOUSE_STATE state;
        al_get_mouse_state (&state);
        Point scale = m_MainLoop->GetSceneManager ()->GetCamera ().GetScale ();

        m_MainLoop->GetSceneManager ()->GetCamera ().Scale (1 / scale.X, 1 / scale.Y, state.x, state.y);
        m_GridSize = std::max (1.0, m_BaseGridSize * m_MainLoop->GetSceneManager ()->GetCamera ().GetScale ().X);
        UpdateLabels ();
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::OnShowGrid (Gwk::Controls::Base* control)
    {
        Gwk::Controls::Button* button = (Gwk::Controls::Button*)control;
        button->SetText ("HIDE GRID");

        m_IsSnapToGrid = true;
        UpdateLabels ();
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::OnHideGrid (Gwk::Controls::Base* control)
    {
        Gwk::Controls::Button* button = (Gwk::Controls::Button*)control;
        button->SetText ("SHOW GRID");

        m_IsSnapToGrid = false;
        UpdateLabels ();
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::OnGridIncrease (Gwk::Controls::Base* control)
    {
        m_BaseGridSize /= 2;

        m_BaseGridSize = std::max (1, std::min (m_BaseGridSize, 1024));
        m_GridSize = std::max (1.0, m_BaseGridSize * m_MainLoop->GetSceneManager ()->GetCamera ().GetScale ().X);

        UpdateLabels ();
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::OnGridDecrease (Gwk::Controls::Base* control)
    {
        m_BaseGridSize *= 2;

        m_BaseGridSize = std::max (1, std::min (m_BaseGridSize, 1024));
        m_GridSize = std::max (1.0, m_BaseGridSize * m_MainLoop->GetSceneManager ()->GetCamera ().GetScale ().X);

        UpdateLabels ();
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::UpdateLabels ()
    {
        Point translate = m_MainLoop->GetSceneManager ()->GetCamera ().GetTranslate ();
        Point scale = m_MainLoop->GetSceneManager ()->GetCamera ().GetScale ();

        Gwk::Controls::Label* xPositionLabel
            = (Gwk::Controls::Label*)m_MainCanvas->GetNamedChildren ("XPositionLabel").list.front ();
        std::string xPosString = " X: " + ToString (translate.X * (1 / scale.X));
        xPositionLabel->SetText (xPosString);

        Gwk::Controls::Label* yPositionLabel
            = (Gwk::Controls::Label*)m_MainCanvas->GetNamedChildren ("YPositionLabel").list.front ();
        std::string yPosString = " Y: " + ToString (translate.Y * (1 / scale.Y));
        yPositionLabel->SetText (yPosString);

        Gwk::Controls::Label* scaleLabel
            = (Gwk::Controls::Label*)m_MainCanvas->GetNamedChildren ("ScaleLabel").list.front ();
        scaleLabel->SetText (
            std::string ("S: ") + ToString (m_MainLoop->GetSceneManager ()->GetCamera ().GetScale ().X));

        Gwk::Controls::Label* snapToGridLabel
            = (Gwk::Controls::Label*)m_MainCanvas->GetNamedChildren ("SnapToGrid").list.front ();
        snapToGridLabel->SetText (std::string ("SNAP: ") + (m_IsSnapToGrid ? "YES" : "NO"));

        Gwk::Controls::Label* gridSizeLabel
            = (Gwk::Controls::Label*)m_MainCanvas->GetNamedChildren ("GridSize").list.front ();
        gridSizeLabel->SetText (std::string ("GRID: ") + ToString (m_BaseGridSize));

        Gwk::Controls::Label* widthLabel
            = (Gwk::Controls::Label*)m_MainCanvas->GetNamedChildren ("WidthLabel").list.front ();
        std::string widthString = "W: " + ToString (m_SelectedAtlasRegion.Bounds.BottomRight.Width);
        widthLabel->SetText (widthString);

        Gwk::Controls::Label* heightLabel
            = (Gwk::Controls::Label*)m_MainCanvas->GetNamedChildren ("HeightLabel").list.front ();
        std::string heightString = "H: " + ToString (m_SelectedAtlasRegion.Bounds.BottomRight.Height);
        heightLabel->SetText (heightString);

        Gwk::Controls::Label* angleLabel
            = (Gwk::Controls::Label*)m_MainCanvas->GetNamedChildren ("AngleLabel").list.front ();
        std::string angleText;

        if (m_SelectedTile)
        {
            angleText = std::string ("A: ") + ToString (m_SelectedTile->Rotation);
        }
        else
        {
            angleText = "A: 0";
        }

        angleLabel->SetText (angleText);

        Gwk::Controls::Label* zOrderLabel
            = (Gwk::Controls::Label*)m_MainCanvas->GetNamedChildren ("ZOrderLabel").list.front ();
        std::string zOrderText;

        if (m_SelectedTile)
        {
            zOrderText = std::string ("ZORD: ") + ToString (m_SelectedTile->ZOrder);
        }
        else
        {
            zOrderText = "ZORD: -";
        }

        zOrderLabel->SetText (zOrderText);
    }

    //--------------------------------------------------------------------------------------------------
}
