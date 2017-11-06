// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "EditorTileMode.h"
#include "AtlasManager.h"
#include "Editor.h"
#include "MainLoop.h"
#include "Screen.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    const int TILES_COUNT = 14;
    const int TILE_SIZE = 50;

    static int CURRENT_ID = 0;

    //--------------------------------------------------------------------------------------------------
    //--------------------------------------------------------------------------------------------------

    EditorTileMode::EditorTileMode (Editor* editor)
        : m_Editor (editor)
        , m_IsDrawTiles (true)
        , m_Rotation (0)
        , m_SelectedTile (nullptr)
        , m_TileUnderCursor (nullptr)
    {
    }

    //--------------------------------------------------------------------------------------------------

    EditorTileMode::~EditorTileMode () {}

    //--------------------------------------------------------------------------------------------------

    void EditorTileMode::RemoveSelectedTile ()
    {
        if (m_SelectedTile)
        {
            m_Editor->m_MainLoop->GetSceneManager ().GetActiveScene ()->RemoveTile (m_SelectedTile);
            m_SelectedTile = nullptr;
            m_Editor->m_CursorMode = CursorMode::TileSelectMode;
        }
    }

    //--------------------------------------------------------------------------------------------------

    void EditorTileMode::CopySelectedTile ()
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

            m_Editor->m_CursorMode = CursorMode::TileEditMode;
            m_Rotation = m_SelectedTile->Rotation;
        }
    }

    //--------------------------------------------------------------------------------------------------

    bool EditorTileMode::MoveSelectedTile ()
    {
        ALLEGRO_MOUSE_STATE state;
        al_get_mouse_state (&state);

        if (state.buttons == 1)
        {
            Point translate = m_Editor->m_MainLoop->GetSceneManager ().GetCamera ().GetTranslate ();
            Point scale = m_Editor->m_MainLoop->GetSceneManager ().GetCamera ().GetScale ();
            Point point = m_Editor->CalculateCursorPoint (state.x, state.y);

            m_SelectedTile->Bounds.Transform.Pos
                = { (translate.X + point.X) * 1 / scale.X, (translate.Y + point.Y) * 1 / scale.Y };
            m_SelectedTile->SetPhysOffset (m_SelectedTile->Bounds.Transform.Pos);

            QuadTreeNode& quadTree = m_Editor->m_MainLoop->GetSceneManager ().GetActiveScene ()->GetQuadTree ();
            quadTree.Remove (m_SelectedTile);
            quadTree.Insert (m_SelectedTile);
            quadTree.UpdateStructures ();
        }

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    void EditorTileMode::ChangeRotation (bool clockwise)
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

    void EditorTileMode::ChangeZOrder (bool clockwise)
    {
        if (m_SelectedTile)
        {
            m_SelectedTile->ZOrder += clockwise ? -1 : 1;

            int currentID = m_SelectedTile->ID;

            m_Editor->m_MainLoop->GetSceneManager ().GetActiveScene ()->SortTiles ();

            std::vector<Tile*>& tiles = m_Editor->m_MainLoop->GetSceneManager ().GetActiveScene ()->GetTiles ();

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

    void EditorTileMode::DrawTiles ()
    {
        std::vector<AtlasRegion>& regions = m_Atlas->GetRegions ();
        const Point windowSize = m_Editor->m_MainLoop->GetScreen ()->GetWindowSize ();
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

    bool EditorTileMode::ChooseTile (int mouseX, int mouseY)
    {
        const Point screenSize = m_Editor->m_MainLoop->GetScreen ()->GetWindowSize ();
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
                m_Editor->m_CursorMode = CursorMode::TileEditMode;
                m_Rotation = m_SelectedTile->Rotation;

                if (!m_SelectedTile->PhysPoints.empty ())
                {
                    m_Editor->m_EditorPhysMode.m_PhysPoly = &m_SelectedTile->PhysPoints[0];
                }

                return true;
            }
        }

        return false;
    }

    //--------------------------------------------------------------------------------------------------

    Tile* EditorTileMode::AddTile (int mouseX, int mouseY)
    {
        Tile* tile = new Tile (&m_Editor->m_MainLoop->GetPhysicsManager ());
        AtlasRegion region = m_Atlas->GetRegion (m_SelectedAtlasRegion.Name);
        Point translate = m_Editor->m_MainLoop->GetSceneManager ().GetCamera ().GetTranslate ();
        Point point = m_Editor->CalculateCursorPoint (mouseX, mouseY);

        tile->ID = ++CURRENT_ID;
        tile->Tileset = m_Atlas->GetName ();
        tile->Name = m_SelectedAtlasRegion.Name;
        tile->Bounds = { { (translate.X + point.X), (translate.Y + point.Y) },
            { region.Bounds.Transform.Size.Width, region.Bounds.Transform.Size.Height } };
        tile->Rotation = m_Rotation;

        m_Editor->m_MainLoop->GetSceneManager ().GetActiveScene ()->AddTile (tile);

        return tile;
    }

    //--------------------------------------------------------------------------------------------------

    Tile* EditorTileMode::GetTileUnderCursor (int mouseX, int mouseY, Rect&& outRect)
    {
        std::vector<Tile*>& tiles = m_Editor->m_MainLoop->GetSceneManager ().GetActiveScene ()->GetTiles ();
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

    Rect EditorTileMode::GetRenderBounds (Tile* tile)
    {
        Point translate = m_Editor->m_MainLoop->GetSceneManager ().GetCamera ().GetTranslate ();
        Point scale = m_Editor->m_MainLoop->GetSceneManager ().GetCamera ().GetScale ();

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

    void EditorTileMode::InitializeUI ()
    {
        m_Atlas = m_Editor->m_MainLoop->GetSceneManager ().GetAtlasManager ()->GetAtlas (
            GetBaseName (GetResourcePath (PACK_0_0_HOME)));
    }

    //--------------------------------------------------------------------------------------------------

    void EditorTileMode::ResetSettings ()
    {
        m_SelectedTile = nullptr;
        m_TileUnderCursor = nullptr;
    }

    //--------------------------------------------------------------------------------------------------

    void EditorTileMode::SetCurrentID (int id) { CURRENT_ID = id; }

    //--------------------------------------------------------------------------------------------------
}