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

    bool askNewScene = false;
    char menuFileName[512] = "0_home/0_0_home.scn";

    bool askFlagPoint = false;
    char flagPointName[64] = {};

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
        , m_PhysPointIndex (-1)
        , m_PhysPoly (nullptr)
        , m_FlagPoint ("")
    {
    }

    //--------------------------------------------------------------------------------------------------

    Editor::~Editor ()
    {
        if (!IsDestroyed ())
        {
            Destroy ();
        }
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

    bool Editor::Update (float deltaTime) { return true; }

    //--------------------------------------------------------------------------------------------------

    bool openTest = false;
    bool saveRequested = false;

    void Editor::ProcessEvent (ALLEGRO_EVENT* event, float deltaTime)
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

            case ALLEGRO_KEY_S:
            {
                if (event->keyboard.modifiers == ALLEGRO_KEYMOD_CTRL)
                {
                    saveRequested = true;
                }
                else
                {
                    m_IsSnapToGrid = !m_IsSnapToGrid;
                }
                break;
            }

            case ALLEGRO_KEY_P:
            {
                m_MainLoop->GetSceneManager ().GetActiveScene ()->SetDrawPhysData (
                    !m_MainLoop->GetSceneManager ().GetActiveScene ()->IsDrawPhysData ());
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
                MenuItemPlay ();
                break;
            }

            case ALLEGRO_KEY_F5:
            {
                m_IsDrawTiles = !m_IsDrawTiles;
                break;
            }

            case ALLEGRO_KEY_SPACE:
            {
                break;
            }

            case ALLEGRO_KEY_TAB:
            {
                if (m_SelectedTile)
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
                break;
            }
            }
        }

        if (event->type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN)
        {
            m_IsMousePan = event->mouse.button == 3;

            if (event->mouse.button == 1)
            {
                m_FlagPoint = GetFlagPointUnderCursor (event->mouse.x, event->mouse.y);

                if (m_CursorMode == CursorMode::TileSelectMode && m_FlagPoint == "")
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
                else if (m_CursorMode == CursorMode::EditFlagPointsMode)
                {
                    InsertFlagPointAtCursor (event->mouse.x, event->mouse.y);
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

                RemoveFlagPointUnderCursor (event->mouse.x, event->mouse.y);
            }
        }
        else if (event->type == ALLEGRO_EVENT_MOUSE_BUTTON_UP)
        {
            m_IsMousePan = false;

            if (m_PhysPoint && m_SelectedTile && event->mouse.button == 1)
            {
                m_SelectedTile->UpdatePhysPolygon ();

                if (m_PhysPointIndex > -1)
                {
                    //                    m_PhysPoint = &(*m_PhysPoly)[m_PhysPointIndex];
                }
            }
        }
        else if (event->type == ALLEGRO_EVENT_MOUSE_AXES)
        {
            if (!MoveSelectedFlagPoint ())
            {
                if (m_CursorMode == CursorMode::TileEditMode)
                {
                    MoveSelectedTile ();
                }
                else if (m_CursorMode == CursorMode::EditPhysBodyMode)
                {
                    MoveSelectedPhysPoint ();
                }
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
            m_GridSize = std::max (1.0f, m_BaseGridSize * m_MainLoop->GetSceneManager ().GetCamera ().GetScale ().X);
        }
        else if (event.dz > 0.0)
        {
            m_MainLoop->GetSceneManager ().GetCamera ().Scale (1.25f, 1.25f, event.x, event.y);
            m_GridSize = std::max (1.0f, m_BaseGridSize * m_MainLoop->GetSceneManager ().GetCamera ().GetScale ().X);
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
                m_SelectedTile->PhysPoints = currentTile->PhysPoints;
            }

            m_CursorMode = CursorMode::TileEditMode;
            m_Rotation = m_SelectedTile->Rotation;
        }
    }

    //--------------------------------------------------------------------------------------------------

    bool Editor::MoveSelectedTile ()
    {
        ALLEGRO_MOUSE_STATE state;
        al_get_mouse_state (&state);

        if (state.buttons == 1)
        {
            Point translate = m_MainLoop->GetSceneManager ().GetCamera ().GetTranslate ();
            Point scale = m_MainLoop->GetSceneManager ().GetCamera ().GetScale ();
            Point point = CalculateCursorPoint (state.x, state.y);

            m_SelectedTile->Bounds.Transform.Pos
                = { (translate.X + point.X) * 1 / scale.X, (translate.Y + point.Y) * 1 / scale.Y };
            m_SelectedTile->SetPhysOffset (m_SelectedTile->Bounds.Transform.Pos);

            QuadTreeNode& quadTree = m_MainLoop->GetSceneManager ().GetActiveScene ()->GetQuadTree ();
            quadTree.Remove (m_SelectedTile);
            quadTree.Insert (m_SelectedTile);
            quadTree.UpdateStructures ();
        }

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    bool Editor::MoveSelectedPhysPoint ()
    {
        ALLEGRO_MOUSE_STATE state;
        al_get_mouse_state (&state);

        if (m_PhysPoint && state.buttons == 1)
        {
            Point p = CalculateCursorPoint (state.x, state.y);
            Point translate = m_MainLoop->GetSceneManager ().GetCamera ().GetTranslate ();
            Point scale = m_MainLoop->GetSceneManager ().GetCamera ().GetScale ();
            Point origin = m_SelectedTile->Bounds.Transform.Pos;

            m_PhysPoint->X = (translate.X + p.X) * 1 / scale.X - origin.X;
            m_PhysPoint->Y = (translate.Y + p.Y) * 1 / scale.Y - origin.Y;

            return true;
        }

        return false;
    }

    //--------------------------------------------------------------------------------------------------

    bool Editor::MoveSelectedFlagPoint ()
    {
        ALLEGRO_MOUSE_STATE state;
        al_get_mouse_state (&state);

        if (state.buttons == 1)
        {
            if (m_FlagPoint != "")
            {
                std::map<std::string, Point>& flagPoints
                    = m_MainLoop->GetSceneManager ().GetActiveScene ()->GetFlagPoints ();

                Point p = CalculateCursorPoint (state.x, state.y);
                Point translate = m_MainLoop->GetSceneManager ().GetCamera ().GetTranslate ();
                Point scale = m_MainLoop->GetSceneManager ().GetCamera ().GetScale ();

                flagPoints[m_FlagPoint].X = (translate.X + p.X) * 1 / scale.X;
                flagPoints[m_FlagPoint].Y = (translate.Y + p.Y) * 1 / scale.Y;

                return true;
            }
        }

        return false;
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::ChangeRotation (bool clockwise)
    {
        m_Rotation += clockwise ? -15 : 15;

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
        m_GridSize = std::max (1.0f, m_BaseGridSize * m_MainLoop->GetSceneManager ().GetCamera ().GetScale ().X);
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

    void Editor::Render (float deltaTime)
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
            al_draw_rectangle (
                b.Transform.Pos.X, b.Transform.Pos.Y, b.Transform.Size.Width, b.Transform.Size.Height, COLOR_RED, 2);
        }

        if (m_CursorMode == CursorMode::TileSelectMode)
        {
            Rect r;
            m_TileUnderCursor = GetTileUnderCursor (state.x, state.y, std::move (r));

            if (m_TileUnderCursor)
            {
                al_draw_rectangle (r.Transform.Pos.X, r.Transform.Pos.Y, r.Transform.Size.Width,
                    r.Transform.Size.Height, COLOR_YELLOW, 2);
            }
        }
        else if (m_CursorMode == CursorMode::EditPhysBodyMode)
        {
            DrawPhysBody (state.x, state.y);
        }

        DrawFlagPoints (state.x, state.y);

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
        float beginning = windowSize.Width * 0.5 - (TILES_COUNT - 1) * 0.5 * TILE_SIZE - TILE_SIZE * 0.5;
        float advance = 0;

        for (int i = 0; i < TILES_COUNT; ++i)
        {
            advance = beginning + i * TILE_SIZE;

            al_draw_rectangle (
                advance, windowSize.Height - TILE_SIZE, advance + TILE_SIZE, windowSize.Height, COLOR_GREEN, 1);

            if (i < regions.size () - 1)
            {
                Rect region = regions[i].Bounds;
                al_draw_scaled_bitmap (m_Atlas->GetImage (), region.Transform.Pos.X, region.Transform.Pos.Y,
                    region.Transform.Size.Width, region.Transform.Size.Height, advance + 1,
                    windowSize.Height - TILE_SIZE + 1, TILE_SIZE - 2, TILE_SIZE - 2, 0);
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
        Point origin = m_SelectedTile->Bounds.Transform.Pos;
        Point* selectedPoint = GetPhysPointUnderCursor (mouseX, mouseY);

        for (std::vector<Point>& points : m_SelectedTile->PhysPoints)
        {
            if (!points.empty ())
            {
                int i = 0;
                int selectedIndex = std::numeric_limits<int>::min ();

                for (int j = 0; j < points.size (); ++j)
                {
                    if (m_PhysPoint && points[j] == *m_PhysPoint)
                    {
                        selectedIndex = j;
                        break;
                    }
                }

                for (const Point& p : points)
                {
                    float xPoint = (origin.X + p.X) * scale.X - translate.X;
                    float yPoint = (origin.Y + p.Y) * scale.Y - translate.Y;

                    ALLEGRO_COLOR color;

                    if (i == 0)
                    {
                        color = COLOR_GREEN;
                    }
                    else
                    {
                        color = COLOR_YELLOW;
                    }

                    if (i == selectedIndex)
                    {
                        //  Mark selected corner
                        color = COLOR_BLUE;
                    }
                    else if ((i == 0 && selectedIndex == points.size () - 1) || (i == selectedIndex + 1))
                    {
                        //  Mark also next corner
                        color = COLOR_LIGHTBLUE;
                    }

                    if (selectedPoint != nullptr && p == *selectedPoint)
                    {
                        color = COLOR_RED;
                    }

                    if (m_MainLoop->GetSceneManager ().GetActiveScene ()->IsDrawPhysData () && false)
                    {
                        m_MainLoop->GetScreen ()->GetFont ().DrawText (FONT_NAME_MAIN_SMALL, al_map_rgb (0, 255, 0),
                            xPoint, yPoint, ToString (i), ALLEGRO_ALIGN_CENTER);
                    }

                    ++i;

                    al_draw_filled_circle (xPoint, yPoint, 4, color);
                }
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::DrawFlagPoints (float mouseX, float mouseY)
    {
        std::map<std::string, Point>& flagPoints = m_MainLoop->GetSceneManager ().GetActiveScene ()->GetFlagPoints ();
        int outsets = 4;

        for (std::map<std::string, Point>::iterator it = flagPoints.begin (); it != flagPoints.end (); ++it)
        {
            Point translate = m_MainLoop->GetSceneManager ().GetCamera ().GetTranslate ();
            Point scale = m_MainLoop->GetSceneManager ().GetCamera ().GetScale ();

            float xPoint = it->second.X * scale.X - translate.X;
            float yPoint = it->second.Y * scale.Y - translate.Y;

            m_MainLoop->GetScreen ()->GetFont ().DrawText (
                FONT_NAME_MAIN_SMALL, al_map_rgb (0, 255, 0), xPoint, yPoint - 15, it->first, ALLEGRO_ALIGN_CENTER);

            Point p = { it->second.X, it->second.Y };

            if (IsMouseWithinPointRect (mouseX, mouseY, p, outsets))
            {
                al_draw_filled_circle (xPoint, yPoint, 4, COLOR_BLUE);
            }
            else
            {
                al_draw_filled_circle (xPoint, yPoint, 4, COLOR_GREEN);
                al_draw_filled_circle (xPoint, yPoint, 2, COLOR_RED);
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    bool Editor::ChooseTile (int mouseX, int mouseY)
    {
        const Point screenSize = m_MainLoop->GetScreen ()->GetWindowSize ();
        float beginning = screenSize.Width * 0.5 - (TILES_COUNT - 1) * 0.5 * TILE_SIZE - TILE_SIZE * 0.5;
        float advance = 0;
        std::vector<AtlasRegion> regions = m_Atlas->GetRegions ();

        for (int i = 0; i < TILES_COUNT; ++i)
        {
            if (i >= regions.size () - 1)
            {
                break;
            }

            advance = beginning + i * TILE_SIZE;
            float x = advance;
            float y = screenSize.Height - TILE_SIZE - 1;
            Rect r = Rect{ { x, y }, { x + TILE_SIZE, y + TILE_SIZE - 2 } };

            if (InsideRect (mouseX, mouseY, r))
            {
                m_SelectedAtlasRegion = regions[i];
                m_SelectedTile = AddTile (mouseX, mouseY);
                m_CursorMode = CursorMode::TileEditMode;
                m_Rotation = m_SelectedTile->Rotation;

                if (!m_SelectedTile->PhysPoints.empty ())
                {
                    m_PhysPoly = &m_SelectedTile->PhysPoints[0];
                }

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
        Point origin = m_SelectedTile->Bounds.Transform.Pos;

        Point pointToInsert
            = { (translate.X + p.X) * 1 / scale.X - origin.X, (translate.Y + p.Y) * 1 / scale.Y - origin.Y };

        bool inserted = false;

        //  After we select one of physics point, we can insert next one accordingly
        Point* againSelected = GetPhysPointUnderCursor (mouseX, mouseY);

        if (m_SelectedTile->PhysPoints.empty ())
        {
            m_SelectedTile->PhysPoints.push_back ({});
        }

        if (!m_PhysPoly)
        {
            m_PhysPoly = &m_SelectedTile->PhysPoints[0];
        }

        if (m_PhysPoint && !againSelected)
        {
            for (int i = 0; i < m_PhysPoly->size (); ++i)
            {
                if (m_PhysPoint && (*m_PhysPoly)[i] == *m_PhysPoint)
                {
                    m_PhysPoly->insert (m_PhysPoly->begin () + i + 1, pointToInsert);
                    m_PhysPoint = nullptr;
                    inserted = true;
                    break;
                }
            }
        }

        m_PhysPointIndex = -1;
        m_PhysPoint = GetPhysPointUnderCursor (mouseX, mouseY);

        if (!inserted && !m_PhysPoint)
        {
            m_PhysPoly->push_back (pointToInsert);
            m_PhysPoint = &(*m_PhysPoly)[m_PhysPoly->size () - 1];
            inserted = true;
        }

        if (inserted)
        {
            m_SelectedTile->SetPhysOffset (origin);

            if (m_PhysPointIndex > -1)
            {
                //    m_PhysPoint = &(*m_PhysPoly)[m_PhysPointIndex];
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::InsertFlagPointAtCursor (int mouseX, int mouseY)
    {
        if (std::string (flagPointName) != "")
        {
            if (GetFlagPointUnderCursor (mouseX, mouseY) == "")
            {
                Point p = CalculateCursorPoint (mouseX, mouseY);
                Point translate = m_MainLoop->GetSceneManager ().GetCamera ().GetTranslate ();
                Point scale = m_MainLoop->GetSceneManager ().GetCamera ().GetScale ();

                Point pointToInsert = { (translate.X + p.X) * 1 / scale.X, (translate.Y + p.Y) * 1 / scale.Y };

                m_MainLoop->GetSceneManager ().GetActiveScene ()->AddFlagPoint (flagPointName, pointToInsert);
                m_FlagPoint = flagPointName;
            }

            strset (flagPointName, 0);
        }
        else
        {
            askFlagPoint = true;
        }
    }

    //--------------------------------------------------------------------------------------------------

    bool Editor::IsMouseWithinPointRect (int mouseX, int mouseY, Point point, int outsets)
    {
        Point translate = m_MainLoop->GetSceneManager ().GetCamera ().GetTranslate ();
        Point scale = m_MainLoop->GetSceneManager ().GetCamera ().GetScale ();

        Rect r = Rect{ { point.X - outsets, point.Y - outsets }, { point.X + outsets, point.Y + outsets } };

        if (InsideRect ((mouseX + translate.X) * 1 / scale.X, (mouseY + translate.Y) * 1 / scale.Y, r))
        {
            return true;
        }

        return false;
    }

    //--------------------------------------------------------------------------------------------------

    Point* Editor::GetPhysPointUnderCursor (int mouseX, int mouseY)
    {
        if (!m_SelectedTile)
        {
            return nullptr;
        }

        Point origin = m_SelectedTile->Bounds.Transform.Pos;

        int outsets = 4;
        for (std::vector<Point>& points : m_SelectedTile->PhysPoints)
        {
            int index = 0;
            for (Point& point : points)
            {
                Point p = { point.X + origin.X, point.Y + origin.Y };

                if (IsMouseWithinPointRect (mouseX, mouseY, p, outsets))
                {
                    m_PhysPoly = &points;
                    m_PhysPointIndex = index;
                    return &point;
                }

                index++;
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
            for (int j = 0; j < m_SelectedTile->PhysPoints.size (); ++j)
            {
                std::vector<Point>& points = m_SelectedTile->PhysPoints[j];

                for (int i = 0; i < points.size (); ++i)
                {
                    if (points[i] == *point)
                    {
                        points.erase (points.begin () + i);

                        //  If no points left, remove polygon itself
                        if (points.empty ())
                        {
                            m_SelectedTile->PhysPoints.erase (m_SelectedTile->PhysPoints.begin () + j);
                            m_PhysPoly = nullptr;
                        }

                        m_SelectedTile->UpdatePhysPolygon ();

                        if (m_PhysPointIndex == i)
                        {
                            m_PhysPointIndex = -1;
                        }

                        return;
                    }
                }
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    bool Editor::RemoveFlagPointUnderCursor (int mouseX, int mouseY)
    {
        std::string flgPoint = GetFlagPointUnderCursor (mouseX, mouseY);

        if (flgPoint != "")
        {
            m_MainLoop->GetSceneManager ().GetActiveScene ()->GetFlagPoints ().erase (flgPoint);
            return true;
        }

        return false;
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
        Tile* tile = new Tile (&m_MainLoop->GetPhysicsManager ());
        AtlasRegion region = m_Atlas->GetRegion (m_SelectedAtlasRegion.Name);
        Point translate = m_MainLoop->GetSceneManager ().GetCamera ().GetTranslate ();
        Point point = CalculateCursorPoint (mouseX, mouseY);

        tile->ID = ++CURRENT_ID;
        tile->Tileset = m_Atlas->GetName ();
        tile->Name = m_SelectedAtlasRegion.Name;
        tile->Bounds = { { (translate.X + point.X), (translate.Y + point.Y) },
            { region.Bounds.Transform.Size.Width, region.Bounds.Transform.Size.Height } };
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
                if ((result == nullptr) || (result && (result->RenderID < tile->RenderID)))
                {
                    outRect = r;
                    result = tile;
                }
            }
        }

        return result;
    }

    //--------------------------------------------------------------------------------------------------

    std::string Editor::GetFlagPointUnderCursor (int mouseX, int mouseY)
    {
        std::map<std::string, Point>& flagPoints = m_MainLoop->GetSceneManager ().GetActiveScene ()->GetFlagPoints ();
        int outsets = 4;

        for (std::map<std::string, Point>::iterator it = flagPoints.begin (); it != flagPoints.end (); ++it)
        {
            Point p = { it->second.X, it->second.Y };

            if (IsMouseWithinPointRect (mouseX, mouseY, p, outsets))
            {
                return it->first;
            }
        }

        return "";
    }

    //--------------------------------------------------------------------------------------------------

    Point RotatePoint (float x, float y, const Point& origin, float angle)
    {
        float s = std::sin (DegressToRadians (angle));
        float c = std::cos (DegressToRadians (angle));

        // translate point back to origin:
        x -= origin.X;
        y -= origin.Y;

        float nx = (x * c) - (y * s);
        float ny = (x * s) + (y * c);

        // translate point back:
        return { nx + origin.X, ny + origin.Y };
    }

    //--------------------------------------------------------------------------------------------------

    Rect Editor::GetRenderBounds (Tile* tile)
    {
        Point translate = m_MainLoop->GetSceneManager ().GetCamera ().GetTranslate ();
        Point scale = m_MainLoop->GetSceneManager ().GetCamera ().GetScale ();

        Rect b = tile->Bounds;
        int width = b.Transform.Size.Width * 0.5f;
        int height = b.Transform.Size.Height * 0.5f;

        float x1 = (b.Transform.Pos.X - translate.X * (1 / scale.X) - width) * (scale.X);
        float y1 = (b.Transform.Pos.Y - translate.Y * (1 / scale.Y) - height) * (scale.Y);
        float x2 = (b.Transform.Pos.X - translate.X * (1 / scale.X) + width) * (scale.X);
        float y2 = (b.Transform.Pos.Y - translate.Y * (1 / scale.Y) + height) * (scale.Y);

        Point origin = { x1 + (x2 - x1) * 0.5f, y1 + (y2 - y1) * 0.5f };
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
        m_Atlas = m_MainLoop->GetSceneManager ().GetAtlasManager ()->GetAtlas (
            GetBaseName (GetResourcePath (PACK_0_0_HOME)));

        Resize ();
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::OnNewScene ()
    {
        m_MainLoop->GetSceneManager ().GetActiveScene ()->Reset ();

        ResetSettings ();
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
                ResetSettings ();
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

    void Editor::ResetSettings ()
    {
        m_SelectedTile = nullptr;
        m_TileUnderCursor = nullptr;
        m_PhysPoint = nullptr;
        m_PhysPointIndex = -1;
        m_PhysPoly = nullptr;

        OnResetScale ();
        OnResetTranslate ();
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
        m_GridSize = std::max (1.0f, m_BaseGridSize * m_MainLoop->GetSceneManager ().GetCamera ().GetScale ().X);
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::OnShowGrid () { m_IsSnapToGrid = !m_IsSnapToGrid; }

    //--------------------------------------------------------------------------------------------------

    void Editor::OnGridIncrease ()
    {
        m_BaseGridSize /= 2;

        m_BaseGridSize = std::max (1, std::min (m_BaseGridSize, 1024));
        m_GridSize = std::max (1.0f, m_BaseGridSize * m_MainLoop->GetSceneManager ().GetCamera ().GetScale ().X);
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::OnGridDecrease ()
    {
        m_BaseGridSize *= 2;

        m_BaseGridSize = std::max (1, std::min (m_BaseGridSize, 1024));
        m_GridSize = std::max (1.0f, m_BaseGridSize * m_MainLoop->GetSceneManager ().GetCamera ().GetScale ().X);
    }

    //--------------------------------------------------------------------------------------------------

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
        ImGui::Begin ("FileMenu", &open, ImVec2 (winSize, 100.f), 0.0f,
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

        if (ImGui::Button ("SAVE SCENE", buttonSize) || saveRequested)
        {
            ImGui::OpenPopup ("Save Scene");
            saveRequested = false;
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
        ImGui::Begin ("ToolbarMenu", &open, ImVec2 (winSize, 120.f), 0.0f,
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
        ImGui::Begin ("Points", &open, ImVec2 (winSize, 40.f), 0.0f,
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

        if (m_CursorMode != CursorMode::EditFlagPointsMode)
        {
            if (ImGui::Button ("FLAG POINT", buttonSize))
            {
                askFlagPoint = true;
            }
        }

        if (askFlagPoint)
        {
            ImGui::OpenPopup ("Flag Point");
            askFlagPoint = false;
        }

        if (ImGui::BeginPopupModal ("Flag Point", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text ("Flag Point name:");

            if (ImGui::IsRootWindowOrAnyChildFocused () && !ImGui::IsAnyItemActive () && !ImGui::IsMouseClicked (0))
            {
                ImGui::SetKeyboardFocusHere (0);
            }

            if (ImGui::InputText (
                    "##flagPoint", flagPointName, sizeof (flagPointName), ImGuiInputTextFlags_EnterReturnsTrue))
            {
                m_CursorMode = CursorMode::EditFlagPointsMode;
                ImGui::CloseCurrentPopup ();
            }
            ImGui::Separator ();

            if (ImGui::Button ("OK", ImVec2 (120, 0)))
            {
                m_CursorMode = CursorMode::EditFlagPointsMode;
                ImGui::CloseCurrentPopup ();
            }

            ImGui::SameLine ();

            if (ImGui::Button ("Cancel", ImVec2 (120, 0)))
            {
                strset (flagPointName, 0);
                m_CursorMode = CursorMode::TileSelectMode;
                ImGui::CloseCurrentPopup ();
            }

            ImGui::EndPopup ();
        }

        ImGui::End ();

        ImGui::SetNextWindowPos (ImVec2 (xOffset, 280), ImGuiCond_FirstUseEver);
        ImGui::Begin ("Physics", &open, ImVec2 (winSize, 60.f), 0.0f,
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

            if (m_CursorMode == CursorMode::EditPhysBodyMode)
            {
                if (ImGui::Button ("NEW POLY", buttonSize))
                {
                    m_PhysPoly = nullptr;
                    m_PhysPoint = nullptr;
                    m_SelectedTile->PhysPoints.push_back ({});
                    m_PhysPoly = &m_SelectedTile->PhysPoints[m_SelectedTile->PhysPoints.size () - 1];
                }
            }
        }

        ImGui::End ();

        ImGui::SetNextWindowPos (ImVec2 (xOffset, 340), ImGuiCond_FirstUseEver);
        ImGui::Begin ("GameMenu", &open, ImVec2 (winSize, 40.f), 0.0f,
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
        if (ImGui::Button ("PLAY", buttonSize))
        {
            MenuItemPlay ();
        }

        ImGui::End ();

        winSize = 140.0f;
        xOffset = windowSize.Width - winSize - 5.0f;

        ImGui::SetNextWindowPos (ImVec2 (xOffset, 5.0f), ImGuiCond_Always);
        ImGui::Begin ("ToolBox", &open, ImVec2 (winSize, 220.f), 0.0f,
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
        ImGui::SetWindowFontScale (1.2);

        ImGui::Text (" AVG: %.2f ms", 1000.0f / ImGui::GetIO ().Framerate);
        ImGui::Text (" FPS: %.1f", ImGui::GetIO ().Framerate);

        ImGui::TextColored (
            ImVec4 (0, 1, 0, 1), std::string ("   X: " + ToString (translate.X * (1 / scale.X))).c_str ());
        ImGui::TextColored (
            ImVec4 (0, 1, 0, 1), std::string ("   Y: " + ToString (translate.Y * (1 / scale.Y))).c_str ());
        ImGui::TextColored (ImVec4 (0, 1, 0, 1),
            std::string ("   W: " + ToString (m_SelectedAtlasRegion.Bounds.Transform.Size.Width)).c_str ());
        ImGui::TextColored (ImVec4 (0, 1, 0, 1),
            std::string ("   H: " + ToString (m_SelectedAtlasRegion.Bounds.Transform.Size.Height)).c_str ());
        ImGui::TextColored (ImVec4 (0, 1, 0, 1),
            std::string ("   A: " + (m_SelectedTile ? ToString (m_SelectedTile->Rotation) : "-")).c_str ());
        ImGui::TextColored (ImVec4 (0, 1, 0, 1),
            std::string ("ZORD: " + (m_SelectedTile ? ToString (m_SelectedTile->ZOrder) : "-")).c_str ());
        ImGui::TextColored (ImVec4 (0, 1, 0, 1), std::string ("   S: " + ToString (scale.X)).c_str ());
        ImGui::TextColored (
            ImVec4 (0, 1, 0, 1), std::string ("SNAP: " + ToString (m_IsSnapToGrid ? "YES" : "NO")).c_str ());
        ImGui::TextColored (ImVec4 (0, 1, 0, 1), std::string ("GRID: " + ToString (m_BaseGridSize)).c_str ());

        ImGui::End ();

        winSize = 150.0f;
        xOffset = windowSize.Width - winSize - 2.0f;

        ImGui::SetNextWindowPos (ImVec2 (xOffset, 230.0f), ImGuiCond_Always);
        ImGui::Begin ("ScriptsBox", &open, ImVec2 (winSize, 220.f), 0.0f,
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
        ImGui::PushItemWidth (-1);

        static int scriptsSelectedIndex = 0;

        std::vector<ScriptMetaData>& scripts = m_MainLoop->GetSceneManager ().GetActiveScene ()->GetScripts ();
        std::vector<const char*> scriptsList;

        for (auto& sc : scripts)
        {
            scriptsList.push_back (sc.Name.c_str ());
        }

        ImGui::ListBox ("##scripts", &scriptsSelectedIndex, scriptsList.data (), scriptsList.size (), 4);

        if (ImGui::Button ("RELOAD", ImVec2 (winSize - 17, 20)))
        {
            m_MainLoop->GetSceneManager ().GetActiveScene ()->ReloadScript (scriptsList[scriptsSelectedIndex]);

            boost::optional<ScriptMetaData&> metaScript
                = m_MainLoop->GetSceneManager ().GetActiveScene ()->GetScriptByName (scriptsList[scriptsSelectedIndex]);

            if (metaScript.is_initialized ())
            {
                metaScript.get ().ScriptObj->Run ("void Start ()");
            }
        }

        ImGui::PopItemWidth ();
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

        float beginning = windowSize.Width * 0.5 - (TILES_COUNT - 1) * 0.5 * TILE_SIZE - TILE_SIZE * 0.5;
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
