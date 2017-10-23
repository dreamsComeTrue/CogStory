// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "Editor.h"
#include "AtlasManager.h"
#include "MainLoop.h"
#include "Scene.h"
#include "Screen.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_a5.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    const int TILES_COUNT = 14;
    const int TILE_SIZE = 50;

    static int CURRENT_ID = 0;

    //--------------------------------------------------------------------------------------------------
    //--------------------------------------------------------------------------------------------------

    Editor::Editor (MainLoop* mainLoop)
      : m_MainLoop (mainLoop)
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
      , m_PhysPoint (nullptr)
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

        InitializeUI ();

        ImGui_ImplA5_Init (m_MainLoop->GetScreen ()->GetDisplay ());
        ImGui::GetIO ().IniFilename = nullptr;

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    bool Editor::Destroy ()
    {
        ImGui_ImplA5_Shutdown ();

        return Lifecycle::Destroy ();
    }

    //--------------------------------------------------------------------------------------------------

    bool Editor::Update (double deltaTime) { return true; }

    //--------------------------------------------------------------------------------------------------

    bool openTest = false;

    void Editor::ProcessEvent (ALLEGRO_EVENT* event, double deltaTime)
    {
        ImGui_ImplA5_ProcessEvent (event);

        if (ImGui::GetIO ().WantCaptureMouse)
        {
            return;
        }

        bool tileSelected = false;
        if (event->type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN)
        {
            if (event->mouse.button == 1 && m_IsDrawTiles)
            {
                tileSelected = ChooseTile (event->mouse.x, event->mouse.y);
            }
        }

        if (event->type == ALLEGRO_EVENT_KEY_CHAR)
        {
            switch (event->keyboard.keycode)
            {
                case ALLEGRO_KEY_R:
                {
                    ChangeRotation (event->keyboard.modifiers == ALLEGRO_KEYMOD_SHIFT);
                    break;
                }

                case ALLEGRO_KEY_G:
                {
                    ChangeGridSize (event->keyboard.modifiers == ALLEGRO_KEYMOD_SHIFT);
                    break;
                }

                case ALLEGRO_KEY_Z:
                {
                    ChangeZOrder (event->keyboard.modifiers == ALLEGRO_KEYMOD_SHIFT);
                    break;
                }

                case ALLEGRO_KEY_X:
                {
                    RemoveSelectedTile ();
                    break;
                }

                case ALLEGRO_KEY_C:
                {
                    CopySelectedTile ();
                    break;
                }
            }
        }

        if (event->type == ALLEGRO_EVENT_KEY_UP)
        {
            switch (event->keyboard.keycode)
            {
                case ALLEGRO_KEY_F1:
                {
                    openTest = !openTest;
                    break;
                }

                case ALLEGRO_KEY_F5:
                {
                    MenuItemPlay ();
                    break;
                }

                case ALLEGRO_KEY_SPACE:
                {
                    m_IsDrawTiles = !m_IsDrawTiles;

                    break;
                }

                case ALLEGRO_KEY_S:
                {
                    m_IsSnapToGrid = !m_IsSnapToGrid;
                    break;
                }
            }
        }

        if (event->type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN)
        {
            m_IsMousePan = event->mouse.button == 3;

            if (event->mouse.button == 1)
            {
                if (m_CursorMode == CursorMode::TileSelectMode)
                {
                    Rect r;
                    m_SelectedTile = GetTileUnderCursor (event->mouse.x, event->mouse.y, std::move (r));

                    if (m_SelectedTile)
                    {
                        m_CursorMode = CursorMode::TileEditMode;
                        m_Rotation = m_SelectedTile->Rotation;
                    }
                }
                else if (m_CursorMode == CursorMode::TileEditMode && !tileSelected)
                {
                    Rect r;
                    Tile* newSelectedTile = GetTileUnderCursor (event->mouse.x, event->mouse.y, std::move (r));

                    if (newSelectedTile != m_SelectedTile || !newSelectedTile)
                    {
                        if (newSelectedTile)
                        {
                            m_SelectedTile = newSelectedTile;
                            m_CursorMode = CursorMode::TileEditMode;
                            m_Rotation = m_SelectedTile->Rotation;
                        }
                        else
                        {
                            m_CursorMode = CursorMode::TileSelectMode;
                            m_SelectedTile = nullptr;
                        }
                    }
                }
                else if (m_CursorMode == CursorMode::EditPhysBodyMode)
                {
                    InsertPhysPointAtCursor (event->mouse.x, event->mouse.y);
                }
            }

            if (event->mouse.button == 2)
            {
                if (m_CursorMode == CursorMode::EditPhysBodyMode)
                {
                    RemovePhysPointUnderCursor (event->mouse.x, event->mouse.y);
                }
                else
                {
                    m_CursorMode = CursorMode::TileSelectMode;
                }
            }
        }
        else if (event->type == ALLEGRO_EVENT_MOUSE_BUTTON_UP)
        {
            m_IsMousePan = false;
        }
        else if (event->type == ALLEGRO_EVENT_MOUSE_AXES)
        {
            if (m_CursorMode == CursorMode::TileEditMode)
            {
                MoveSelectedTile ();
            }
            else if (m_CursorMode == CursorMode::EditPhysBodyMode)
            {
                MoveSelectedPhysPoint ();
            }

            HandleCameraMovement (event->mouse);
        }
        else if (event->type == ALLEGRO_EVENT_DISPLAY_RESIZE)
        {
            Resize ();
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::HandleCameraMovement (const ALLEGRO_MOUSE_EVENT& event)
    {
        if (event.dz < 0.0)
        {
            m_MainLoop->GetSceneManager ().GetCamera ().Scale (0.75f, 0.75f, event.x, event.y);
            m_GridSize = std::max (1.0, m_BaseGridSize * m_MainLoop->GetSceneManager ().GetCamera ().GetScale ().X);
        }
        else if (event.dz > 0.0)
        {
            m_MainLoop->GetSceneManager ().GetCamera ().Scale (1.25f, 1.25f, event.x, event.y);
            m_GridSize = std::max (1.0, m_BaseGridSize * m_MainLoop->GetSceneManager ().GetCamera ().GetScale ().X);
        }

        if (m_IsMousePan)
        {
            m_MainLoop->GetSceneManager ().GetCamera ().Move (event.dx, event.dy);
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::RemoveSelectedTile ()
    {
        if (m_SelectedTile)
        {
            m_MainLoop->GetSceneManager ().GetActiveScene ()->RemoveTile (m_SelectedTile);
            m_SelectedTile = nullptr;
            m_CursorMode = CursorMode::TileSelectMode;
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::CopySelectedTile ()
    {
        if (m_TileUnderCursor)
        {
            m_SelectedAtlasRegion = m_Atlas->GetRegion (m_TileUnderCursor->Name);

            ALLEGRO_MOUSE_STATE state;
            al_get_mouse_state (&state);

            Tile* currentTile = m_SelectedTile;

            m_SelectedTile = AddTile (state.x, state.y);

            if (currentTile)
            {
                m_SelectedTile->PhysVertices = currentTile->PhysVertices;
            }

            m_CursorMode = CursorMode::TileEditMode;
            m_Rotation = m_SelectedTile->Rotation;
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::MoveSelectedTile ()
    {
        ALLEGRO_MOUSE_STATE state;
        al_get_mouse_state (&state);

        if (state.buttons == 1)
        {
            Point translate = m_MainLoop->GetSceneManager ().GetCamera ().GetTranslate ();
            Point scale = m_MainLoop->GetSceneManager ().GetCamera ().GetScale ();
            Point point = CalculateCursorPoint (state.x, state.y);

            m_SelectedTile->Bounds.TopLeft = { (translate.X + point.X) * 1 / scale.X, (translate.Y + point.Y) * 1 / scale.Y };
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::MoveSelectedPhysPoint ()
    {
        ALLEGRO_MOUSE_STATE state;
        al_get_mouse_state (&state);

        if (m_PhysPoint && state.buttons == 1)
        {
            Point p = CalculateCursorPoint (state.x, state.y);
            Point translate = m_MainLoop->GetSceneManager ().GetCamera ().GetTranslate ();
            Point scale = m_MainLoop->GetSceneManager ().GetCamera ().GetScale ();
            Point origin = m_SelectedTile->Bounds.TopLeft;

            m_PhysPoint->X = (translate.X + p.X) * 1 / scale.X - origin.X;
            m_PhysPoint->Y = (translate.Y + p.Y) * 1 / scale.Y - origin.Y;
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::ChangeRotation (bool clockwise)
    {
        m_Rotation += clockwise ? 15 : -15;

        if (m_Rotation <= -360)
        {
            m_Rotation = 0;
        }

        if (m_Rotation >= 360)
        {
            m_Rotation = 0;
        }

        if (m_SelectedTile)
        {
            m_SelectedTile->Rotation = m_Rotation;
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::ChangeGridSize (bool clockwise)
    {
        m_BaseGridSize *= clockwise ? 0.5 : 2;
        m_BaseGridSize = std::max (1, std::min (m_BaseGridSize, 1024));
        m_GridSize = std::max (1.0, m_BaseGridSize * m_MainLoop->GetSceneManager ().GetCamera ().GetScale ().X);
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::ChangeZOrder (bool clockwise)
    {
        if (m_SelectedTile)
        {
            m_SelectedTile->ZOrder += clockwise ? -1 : 1;

            int currentID = m_SelectedTile->ID;

            m_MainLoop->GetSceneManager ().GetActiveScene ()->SortTiles ();

            std::vector<Tile*>& tiles = m_MainLoop->GetSceneManager ().GetActiveScene ()->GetTiles ();

            for (Tile* tile : tiles)
            {
                if (tile->ID == currentID)
                {
                    m_SelectedTile = tile;
                    break;
                }
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::Render (double deltaTime)
    {
        if (m_IsSnapToGrid)
        {
            DrawGrid ();
        }

        m_MainLoop->GetSceneManager ().Render (deltaTime);

        ALLEGRO_MOUSE_STATE state;
        al_get_mouse_state (&state);

        m_MainLoop->GetSceneManager ().GetCamera ().UseIdentityTransform ();

        if (m_SelectedTile)
        {
            Rect b = GetRenderBounds (m_SelectedTile);
            al_draw_rectangle (b.TopLeft.X, b.TopLeft.Y, b.BottomRight.Width, b.BottomRight.Height, COLOR_RED, 2);
        }

        if (m_CursorMode == CursorMode::TileSelectMode)
        {
            Rect r;
            m_TileUnderCursor = GetTileUnderCursor (state.x, state.y, std::move (r));

            if (m_TileUnderCursor)
            {
                al_draw_rectangle (r.TopLeft.X, r.TopLeft.Y, r.BottomRight.Width, r.BottomRight.Height, COLOR_YELLOW, 2);
            }
        }
        else if (m_CursorMode == CursorMode::EditPhysBodyMode)
        {
            DrawPhysBody (state.x, state.y);
        }

        if (m_IsDrawTiles)
        {
            DrawTiles ();
            RenderUI ();
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::SetDrawUITiles (bool draw) { m_IsDrawTiles = draw; }

    //--------------------------------------------------------------------------------------------------

    void Editor::DrawTiles ()
    {
        std::vector<AtlasRegion>& regions = m_Atlas->GetRegions ();
        const Point windowSize = m_MainLoop->GetScreen ()->GetWindowSize ();
        double beginning = windowSize.Width * 0.5 - (TILES_COUNT - 1) * 0.5 * TILE_SIZE - TILE_SIZE * 0.5;
        float advance = 0;

        for (int i = 0; i < TILES_COUNT; ++i)
        {
            advance = beginning + i * TILE_SIZE;

            al_draw_rectangle (advance, windowSize.Height - TILE_SIZE, advance + TILE_SIZE, windowSize.Height, COLOR_GREEN, 1);

            if (i < regions.size () - 1)
            {
                Rect region = regions[i].Bounds;
                al_draw_scaled_bitmap (m_Atlas->GetImage (),
                                       region.TopLeft.X,
                                       region.TopLeft.Y,
                                       region.BottomRight.Width,
                                       region.BottomRight.Height,
                                       advance + 1,
                                       windowSize.Height - TILE_SIZE + 1,
                                       TILE_SIZE - 2,
                                       TILE_SIZE - 2,
                                       0);
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::DrawGrid ()
    {
        const ALLEGRO_COLOR LIGHT_GRAY{ 0.5f, 0.5f, 0.5f, 1.0f };

        Point t = m_MainLoop->GetSceneManager ().GetCamera ().GetTranslate ();
        Point scale = m_MainLoop->GetSceneManager ().GetCamera ().GetScale ();

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

        const Point screenSize = m_MainLoop->GetScreen ()->GetWindowSize ();

        int horEndX = 300 + std::fabs (horBeginX);
        int horEndY = screenSize.Height + std::fabs (horBeginY);

        for (int i = horBeginX; i < horEndX; ++i)
        {
            float xOffset = i * m_GridSize - t.X;

            //  |
            al_draw_line (xOffset, horBeginY, xOffset, horEndY, LIGHT_GRAY, 1);
        }

        horEndX = screenSize.Width + std::fabs (horBeginX);
        horEndY = 300 + std::fabs (horBeginY);

        for (int i = horBeginY; i < horEndY; ++i)
        {
            float yOffset = i * m_GridSize - t.Y;

            //  --
            al_draw_line (horBeginX, yOffset, horEndX, yOffset, LIGHT_GRAY, 1);
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::DrawPhysBody (float mouseX, float mouseY)
    {
        if (!m_SelectedTile)
        {
            return;
        }

        Point translate = m_MainLoop->GetSceneManager ().GetCamera ().GetTranslate ();
        Point scale = m_MainLoop->GetSceneManager ().GetCamera ().GetScale ();
        Point origin = m_SelectedTile->Bounds.TopLeft;

        std::vector<float> vertices;

        for (const Point& p : m_SelectedTile->PhysVertices)
        {
            float xPoint = (origin.X + p.X) * scale.X - translate.X;
            float yPoint = (origin.Y + p.Y) * scale.Y - translate.Y;

            vertices.push_back (xPoint);
            vertices.push_back (yPoint);
        }

        al_draw_polygon (vertices.data (), vertices.size () / 2, 0, COLOR_YELLOW, 2, 0);

        Point* selectedPoint = GetPhysPointUnderCursor (mouseX, mouseY);

        for (int i = 0; i < vertices.size (); i += 2)
        {
            Point& point = m_SelectedTile->PhysVertices[i / 2];
            ALLEGRO_COLOR color;

            if (selectedPoint != nullptr && point.X == selectedPoint->X && point.Y == selectedPoint->Y)
            {
                color = COLOR_RED;
            }
            else if (i == 0)
            {
                color = COLOR_GREEN;
            }
            else
            {
                color = COLOR_YELLOW;
            }

            if (m_PhysPoint && point.X == m_PhysPoint->X && point.Y == m_PhysPoint->Y)
            {
                color = COLOR_BLUE;
            }

            al_draw_filled_circle (vertices[i], vertices[i + 1], 4, color);
        }
    }

    //--------------------------------------------------------------------------------------------------

    bool Editor::ChooseTile (int mouseX, int mouseY)
    {
        const Point screenSize = m_MainLoop->GetScreen ()->GetWindowSize ();
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
                m_SelectedAtlasRegion = regions[i];
                m_SelectedTile = AddTile (-10000000, -10000000);
                m_CursorMode = CursorMode::TileEditMode;
                m_Rotation = m_SelectedTile->Rotation;

                return true;
            }
        }

        return false;
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::InsertPhysPointAtCursor (int mouseX, int mouseY)
    {
        if (!m_SelectedTile)
        {
            return;
        }

        Point p = CalculateCursorPoint (mouseX, mouseY);
        Point translate = m_MainLoop->GetSceneManager ().GetCamera ().GetTranslate ();
        Point scale = m_MainLoop->GetSceneManager ().GetCamera ().GetScale ();
        Point origin = m_SelectedTile->Bounds.TopLeft;

        Point pointToInsert = { (translate.X + p.X) * 1 / scale.X - origin.X, (translate.Y + p.Y) * 1 / scale.Y - origin.Y };

        bool inserted = false;

        //  After we select one of physics point, we can insert next one accordingly
        Point* againSelected = GetPhysPointUnderCursor (mouseX, mouseY);

        if (m_PhysPoint && !againSelected)
        {
            for (int i = 0; i < m_SelectedTile->PhysVertices.size (); ++i)
            {
                if (m_PhysPoint && m_SelectedTile->PhysVertices[i].X == m_PhysPoint->X &&
                    m_SelectedTile->PhysVertices[i].Y == m_PhysPoint->Y)
                {
                    m_SelectedTile->PhysVertices.insert (m_SelectedTile->PhysVertices.begin () + i + 1, pointToInsert);
                    m_PhysPoint = nullptr;
                    inserted = true;
                    break;
                }
            }
        }

        m_PhysPoint = GetPhysPointUnderCursor (mouseX, mouseY);

        if (!inserted && !m_PhysPoint)
        {
            m_SelectedTile->PhysVertices.push_back (pointToInsert);
        }
    }

    //--------------------------------------------------------------------------------------------------

    Point* Editor::GetPhysPointUnderCursor (int mouseX, int mouseY)
    {
        if (!m_SelectedTile)
        {
            return nullptr;
        }

        Point translate = m_MainLoop->GetSceneManager ().GetCamera ().GetTranslate ();
        Point scale = m_MainLoop->GetSceneManager ().GetCamera ().GetScale ();
        Point origin = m_SelectedTile->Bounds.TopLeft;

        for (int i = 0; i < m_SelectedTile->PhysVertices.size (); ++i)
        {
            const Point& point = m_SelectedTile->PhysVertices[i];

            int outsets = 4;
            Rect r = Rect{ { point.X + origin.X - outsets, point.Y + origin.Y - outsets },
                           { point.X + origin.X + outsets, point.Y + origin.Y + outsets } };

            if (InsideRect ((mouseX + translate.X) * 1 / scale.X, (mouseY + translate.Y) * 1 / scale.Y, r))
            {
                return &m_SelectedTile->PhysVertices[i];
            }
        }

        return nullptr;
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::RemovePhysPointUnderCursor (int mouseX, int mouseY)
    {
        Point* point = GetPhysPointUnderCursor (mouseX, mouseY);

        if (m_SelectedTile && point)
        {
            for (int i = 0; i < m_SelectedTile->PhysVertices.size (); ++i)
            {
                if (m_SelectedTile->PhysVertices[i] == *point)
                {
                    m_SelectedTile->PhysVertices.erase (m_SelectedTile->PhysVertices.begin () + i);
                    break;
                }
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    Point Editor::CalculateCursorPoint (int mouseX, int mouseY)
    {
        Point translate = m_MainLoop->GetSceneManager ().GetCamera ().GetTranslate ();
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

    Tile* Editor::AddTile (int mouseX, int mouseY)
    {
        Tile* tile = new Tile ();
        AtlasRegion region = m_Atlas->GetRegion (m_SelectedAtlasRegion.Name);
        Point translate = m_MainLoop->GetSceneManager ().GetCamera ().GetTranslate ();
        Point point = CalculateCursorPoint (mouseX, mouseY);

        tile->ID = ++CURRENT_ID;
        tile->Tileset = m_Atlas->GetName ();
        tile->Name = m_SelectedAtlasRegion.Name;
        tile->Bounds = { { (translate.X + point.X), (translate.Y + point.Y) },
                         { region.Bounds.BottomRight.Width, region.Bounds.BottomRight.Height } };
        tile->Rotation = m_Rotation;

        m_MainLoop->GetSceneManager ().GetActiveScene ()->AddTile (tile);

        return tile;
    }

    //--------------------------------------------------------------------------------------------------

    Tile* Editor::GetTileUnderCursor (int mouseX, int mouseY, Rect&& outRect)
    {
        std::vector<Tile*>& tiles = m_MainLoop->GetSceneManager ().GetActiveScene ()->GetTiles ();
        Tile* result = nullptr;

        for (Tile* tile : tiles)
        {
            Rect r = GetRenderBounds (tile);

            if (InsideRect (mouseX, mouseY, r))
            {
                if ((result == nullptr) || (result && ((result->ZOrder < tile->ZOrder) || (result->RenderID < tile->RenderID))))
                {
                    outRect = r;
                    result = tile;
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

    Rect Editor::GetRenderBounds (Tile* tile)
    {
        Point translate = m_MainLoop->GetSceneManager ().GetCamera ().GetTranslate ();
        Point scale = m_MainLoop->GetSceneManager ().GetCamera ().GetScale ();

        Rect b = tile->Bounds;
        int width = b.BottomRight.Width * 0.5;
        int height = b.BottomRight.Height * 0.5;

        float x1 = (b.TopLeft.X - translate.X * (1 / scale.X) - width) * (scale.X);
        float y1 = (b.TopLeft.Y - translate.Y * (1 / scale.Y) - height) * (scale.Y);
        float x2 = (b.TopLeft.X - translate.X * (1 / scale.X) + width) * (scale.X);
        float y2 = (b.TopLeft.Y - translate.Y * (1 / scale.Y) + height) * (scale.Y);

        Point origin = { x1 + (x2 - x1) * 0.5, y1 + (y2 - y1) * 0.5 };
        Point pointA = RotatePoint (x1, y1, origin, tile->Rotation);
        Point pointB = RotatePoint (x1, y2, origin, tile->Rotation);
        Point pointC = RotatePoint (x2, y1, origin, tile->Rotation);
        Point pointD = RotatePoint (x2, y2, origin, tile->Rotation);

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
        m_Atlas = m_MainLoop->GetSceneManager ().GetAtlasManager ()->GetAtlas (GetBaseName (GetResourcePath (PACK_0_0_HOME)));

        //  Back frame
        for (int i = 0; i < TILES_COUNT; ++i)
        {
            std::ostringstream nameBack;
            nameBack << "TileRectBack" << i;
        }

        Resize ();
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::OnNewScene ()
    {
        m_MainLoop->GetSceneManager ().GetActiveScene ()->Reset ();

        OnResetScale ();
        OnResetTranslate ();
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::OnLoadScene (const std::string& openFileName)
    {
        std::string path = GetDataPath () + "scenes/" + openFileName;

        if (boost::filesystem::exists (path))
        {
            Scene* scene = Scene::LoadScene (&m_MainLoop->GetSceneManager (), path);

            if (scene)
            {
                m_MainLoop->GetSceneManager ().RemoveScene (m_MainLoop->GetSceneManager ().GetActiveScene ());
                m_MainLoop->GetSceneManager ().SetActiveScene (scene);

                std::vector<Tile*>& tiles = scene->GetTiles ();
                int maxTileID = 0;

                for (Tile* t : tiles)
                {
                    if (t->ID > maxTileID)
                    {
                        maxTileID = t->ID + 1;
                    }
                }

                CURRENT_ID = maxTileID;
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::OnSaveScene (const std::string& saveFileName)
    {
        std::string path = GetDataPath () + "scenes/" + saveFileName;

        Scene::SaveScene (m_MainLoop->GetSceneManager ().GetActiveScene (), path);
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::OnPlay () { MenuItemPlay (); }

    //--------------------------------------------------------------------------------------------------

    void Editor::MenuItemPlay () { m_MainLoop->GetStateManager ().SetActiveState ("GAMEPLAY_STATE"); }

    //--------------------------------------------------------------------------------------------------

    void Editor::OnExit () { m_MainLoop->Exit (); }

    //--------------------------------------------------------------------------------------------------

    void OnExitYesButton () {}

    //--------------------------------------------------------------------------------------------------

    void OnExitNoButton () {}

    //--------------------------------------------------------------------------------------------------

    void Editor::OnTileSelected () {}

    //--------------------------------------------------------------------------------------------------

    void Editor::OnResetTranslate ()
    {
        const Point& windowSize = m_MainLoop->GetScreen ()->GetWindowSize ();
        m_MainLoop->GetSceneManager ().GetCamera ().SetTranslate (windowSize.Width * 0.5, windowSize.Height * 0.5);
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::OnResetScale ()
    {
        ALLEGRO_MOUSE_STATE state;
        al_get_mouse_state (&state);

        Point scale = m_MainLoop->GetSceneManager ().GetCamera ().GetScale ();

        m_MainLoop->GetSceneManager ().GetCamera ().Scale (1 / scale.X, 1 / scale.Y, state.x, state.y);
        m_GridSize = std::max (1.0, m_BaseGridSize * m_MainLoop->GetSceneManager ().GetCamera ().GetScale ().X);
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::OnShowGrid () { m_IsSnapToGrid = !m_IsSnapToGrid; }

    //--------------------------------------------------------------------------------------------------

    void Editor::OnGridIncrease ()
    {
        m_BaseGridSize /= 2;

        m_BaseGridSize = std::max (1, std::min (m_BaseGridSize, 1024));
        m_GridSize = std::max (1.0, m_BaseGridSize * m_MainLoop->GetSceneManager ().GetCamera ().GetScale ().X);
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::OnGridDecrease ()
    {
        m_BaseGridSize *= 2;

        m_BaseGridSize = std::max (1, std::min (m_BaseGridSize, 1024));
        m_GridSize = std::max (1.0, m_BaseGridSize * m_MainLoop->GetSceneManager ().GetCamera ().GetScale ().X);
    }

    //--------------------------------------------------------------------------------------------------

    bool askNewScene = false;
    char menuFileName[512] = {};

    void Editor::RenderUI ()
    {
        ImGui_ImplA5_NewFrame ();

        Point translate = m_MainLoop->GetSceneManager ().GetCamera ().GetTranslate ();
        Point scale = m_MainLoop->GetSceneManager ().GetCamera ().GetScale ();
        const Point windowSize = m_MainLoop->GetScreen ()->GetWindowSize ();
        bool open = true;

        int winSize = 140.0f;
        int xOffset = 5.0f;

        ImGui::SetNextWindowPos (ImVec2 (xOffset, xOffset), ImGuiCond_FirstUseEver);
        ImGui::Begin ("FileMenu",
                      &open,
                      ImVec2 (winSize, 100.f),
                      0.0f,
                      ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
        ImVec2 buttonSize (100, 20);

        ImGui::TextColored (ImVec4 (0, 1, 0, 1),
                            std::string ("SCENE: " + m_MainLoop->GetSceneManager ().GetActiveScene ()->GetName ()).c_str ());

        static bool newSceneDontAsk = false;
        if (ImGui::Button ("NEW SCENE", buttonSize))
        {
            if (newSceneDontAsk)
            {
                OnNewScene ();
            }
            else
            {
                ImGui::OpenPopup ("Erase?");
            }
        }

        if (ImGui::BeginPopupModal ("Erase?", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text ("All those beautiful files will be deleted.\nThis operation cannot be undone!\n\n");
            ImGui::Separator ();

            ImGui::PushStyleVar (ImGuiStyleVar_FramePadding, ImVec2 (0, 0));
            ImGui::Checkbox ("Don't ask me next time", &newSceneDontAsk);
            ImGui::PopStyleVar ();

            if (ImGui::Button ("OK", ImVec2 (120, 0)))
            {
                OnNewScene ();
                ImGui::CloseCurrentPopup ();
            }

            ImGui::SameLine ();

            if (ImGui::Button ("Cancel", ImVec2 (120, 0)))
            {
                ImGui::CloseCurrentPopup ();
            }

            ImGui::EndPopup ();
        }

        if (ImGui::Button ("OPEN SCENE", buttonSize))
        {
            ImGui::OpenPopup ("Open Scene");
        }

        if (ImGui::BeginPopupModal ("Open Scene", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text ("File path:");

            if (ImGui::IsRootWindowOrAnyChildFocused () && !ImGui::IsAnyItemActive () && !ImGui::IsMouseClicked (0))
            {
                ImGui::SetKeyboardFocusHere (0);
            }

            if (ImGui::InputText ("##edit", menuFileName, sizeof (menuFileName), ImGuiInputTextFlags_EnterReturnsTrue))
            {
                OnLoadScene (menuFileName);
                ImGui::CloseCurrentPopup ();
            }

            ImGui::Separator ();

            if (ImGui::Button ("OK", ImVec2 (120, 0)))
            {
                OnLoadScene (menuFileName);
                ImGui::CloseCurrentPopup ();
            }

            ImGui::SameLine ();

            if (ImGui::Button ("Cancel", ImVec2 (120, 0)))
            {
                ImGui::CloseCurrentPopup ();
            }

            ImGui::EndPopup ();
        }

        if (ImGui::Button ("SAVE SCENE", buttonSize))
        {
            ImGui::OpenPopup ("Save Scene");
        }

        if (ImGui::BeginPopupModal ("Save Scene", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text ("File path:");

            if (ImGui::IsRootWindowOrAnyChildFocused () && !ImGui::IsAnyItemActive () && !ImGui::IsMouseClicked (0))
            {
                ImGui::SetKeyboardFocusHere (0);
            }

            if (ImGui::InputText ("##edit", menuFileName, sizeof (menuFileName), ImGuiInputTextFlags_EnterReturnsTrue))
            {
                OnSaveScene (menuFileName);
                ImGui::CloseCurrentPopup ();
            }
            ImGui::Separator ();

            if (ImGui::Button ("OK", ImVec2 (120, 0)))
            {
                OnSaveScene (menuFileName);
                ImGui::CloseCurrentPopup ();
            }

            ImGui::SameLine ();

            if (ImGui::Button ("Cancel", ImVec2 (120, 0)))
            {
                ImGui::CloseCurrentPopup ();
            }

            ImGui::EndPopup ();
        }

        ImGui::End ();

        ImGui::SetNextWindowPos (ImVec2 (xOffset, 120), ImGuiCond_FirstUseEver);
        ImGui::Begin ("ToolbarMenu",
                      &open,
                      ImVec2 (winSize, 120.f),
                      0.0f,
                      ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
        if (ImGui::Button ("RESET MOVE", buttonSize))
        {
            OnResetTranslate ();
        }

        if (ImGui::Button ("RESET SCALE", buttonSize))
        {
            OnResetScale ();
        }

        if (ImGui::Button (m_IsSnapToGrid ? "HIDE GRID" : "SHOW GRID", buttonSize))
        {
            OnShowGrid ();
        }

        if (ImGui::Button ("+++", ImVec2 (buttonSize.x / 2 - 4, buttonSize.y)))
        {
            OnGridIncrease ();
        }

        ImGui::SameLine ();

        if (ImGui::Button ("---", ImVec2 (buttonSize.x / 2 - 4, buttonSize.y)))
        {
            OnGridDecrease ();
        }

        ImGui::End ();

        ImGui::SetNextWindowPos (ImVec2 (xOffset, 240), ImGuiCond_FirstUseEver);
        ImGui::Begin ("Physics",
                      &open,
                      ImVec2 (winSize, 120.f),
                      0.0f,
                      ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

        if (m_SelectedTile)
        {
            if (ImGui::Button (m_CursorMode != CursorMode::EditPhysBodyMode ? "TILE MODE" : "PHYS MODE", buttonSize))
            {
                if (m_CursorMode != CursorMode::EditPhysBodyMode)
                {
                    m_CursorMode = CursorMode::EditPhysBodyMode;
                }
                else
                {
                    m_CursorMode = CursorMode::TileSelectMode;
                }
            }
        }

        ImGui::End ();

        ImGui::SetNextWindowPos (ImVec2 (xOffset, 360), ImGuiCond_FirstUseEver);
        ImGui::Begin ("GameMenu",
                      &open,
                      ImVec2 (winSize, 120.f),
                      0.0f,
                      ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
        if (ImGui::Button ("PLAY", buttonSize))
        {
            MenuItemPlay ();
        }

        ImGui::End ();

        winSize = 140.0f;
        xOffset = windowSize.Width - winSize - 5.0f;

        ImGui::SetNextWindowPos (ImVec2 (xOffset, 5.0f), ImGuiCond_Always);
        ImGui::Begin ("ToolBox",
                      &open,
                      ImVec2 (winSize, 220.f),
                      0.0f,
                      ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
        ImGui::SetWindowFontScale (1.2);

        ImGui::Text (" AVG: %.2f ms", 1000.0f / ImGui::GetIO ().Framerate);
        ImGui::Text (" FPS: %.1f", ImGui::GetIO ().Framerate);

        ImGui::TextColored (ImVec4 (0, 1, 0, 1), std::string ("   X: " + ToString (translate.X * (1 / scale.X))).c_str ());
        ImGui::TextColored (ImVec4 (0, 1, 0, 1), std::string ("   Y: " + ToString (translate.Y * (1 / scale.Y))).c_str ());
        ImGui::TextColored (ImVec4 (0, 1, 0, 1),
                            std::string ("   W: " + ToString (m_SelectedAtlasRegion.Bounds.BottomRight.Width)).c_str ());
        ImGui::TextColored (ImVec4 (0, 1, 0, 1),
                            std::string ("   H: " + ToString (m_SelectedAtlasRegion.Bounds.BottomRight.Height)).c_str ());
        ImGui::TextColored (ImVec4 (0, 1, 0, 1),
                            std::string ("   A: " + (m_SelectedTile ? ToString (m_SelectedTile->Rotation) : "-")).c_str ());
        ImGui::TextColored (ImVec4 (0, 1, 0, 1),
                            std::string ("ZORD: " + (m_SelectedTile ? ToString (m_SelectedTile->ZOrder) : "-")).c_str ());
        ImGui::TextColored (ImVec4 (0, 1, 0, 1), std::string ("   S: " + ToString (scale.X)).c_str ());
        ImGui::TextColored (ImVec4 (0, 1, 0, 1), std::string ("SNAP: " + ToString (m_IsSnapToGrid ? "YES" : "NO")).c_str ());
        ImGui::TextColored (ImVec4 (0, 1, 0, 1), std::string ("GRID: " + ToString (m_BaseGridSize)).c_str ());

        ImGui::End ();

        if (openTest)
        {
            static bool openTestWindow = false;
            ImGui::SetNextWindowPos (ImVec2 (5, 5), ImGuiCond_FirstUseEver);
            ImGui::ShowTestWindow (&openTestWindow);
        }

        ImGui::Render ();
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::Resize ()
    {
        const Point windowSize = m_MainLoop->GetScreen ()->GetWindowSize ();

        double beginning = windowSize.Width * 0.5 - (TILES_COUNT - 1) * 0.5 * TILE_SIZE - TILE_SIZE * 0.5;
        float advance = 0;

        for (int i = 0; i < TILES_COUNT; ++i)
        {
            advance = beginning + i * TILE_SIZE;

            std::ostringstream nameBack;
            nameBack << "TileRectBack" << i;
        }
    }

    //--------------------------------------------------------------------------------------------------
}
