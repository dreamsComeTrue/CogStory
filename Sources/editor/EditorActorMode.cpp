// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "EditorActorMode.h"
#include "ActorFactory.h"
#include "AtlasManager.h"
#include "Editor.h"
#include "MainLoop.h"
#include "SceneManager.h"
#include "Screen.h"
#include "actors/TileActor.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    const int TILES_COUNT = 14;
    const int TILE_SIZE = 50;

    //--------------------------------------------------------------------------------------------------

    EditorActorMode::EditorActorMode (Editor* editor)
        : m_Editor (editor)
        , m_SelectedActor (nullptr)
        , m_ActorUnderCursor (nullptr)
        , m_Rotation (0)
    {
    }

    //--------------------------------------------------------------------------------------------------

    EditorActorMode::~EditorActorMode () {}

    //--------------------------------------------------------------------------------------------------

    bool EditorActorMode::AddOrUpdateActor (const std::string& name, const std::string& actorType, Point pos,
                                            float rotation, int zOrder)
    {
        Actor* actor = m_Editor->GetMainLoop ()->GetSceneManager ().GetActiveScene ()->GetActor (name);

        if (actor)
        {
            actor->Name = name;
            actor->Bounds.Pos = pos;
            actor->TemplateBounds.Pos = pos;
            actor->Rotation = rotation;
            actor->ZOrder = zOrder;
        }
        else
        {
            actor = ActorFactory::GetActor (&m_Editor->GetMainLoop ()->GetSceneManager (), actorType);
            actor->Name = name;
            actor->Bounds.Pos = pos;
            actor->TemplateBounds.Pos = pos;
            actor->Rotation = rotation;
            actor->ZOrder = zOrder;

            actor->Initialize ();

            m_Editor->GetMainLoop ()->GetSceneManager ().GetActiveScene ()->AddActor (name, actor);
        }

        m_Editor->GetMainLoop ()->GetSceneManager ().GetActiveScene ()->SortActors ();

        m_Editor->GetEditorActorMode ().Clear ();

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    void EditorActorMode::RemoveActor (const std::string& name)
    {
        m_Editor->GetMainLoop ()->GetSceneManager ().GetActiveScene ()->RemoveActor (name);
        m_Editor->GetMainLoop ()->GetSceneManager ().GetActiveScene ()->SortActors ();
    }

    //--------------------------------------------------------------------------------------------------

    void EditorActorMode::Clear () {}

    //--------------------------------------------------------------------------------------------------

    bool EditorActorMode::MoveSelectedActor ()
    {
        ALLEGRO_MOUSE_STATE state;
        al_get_mouse_state (&state);

        if (state.buttons == 1 && m_SelectedActor)
        {
            Point point
                = m_Editor->CalculateCursorPoint (state.x + m_TileSelectionOffset.X, state.y + m_TileSelectionOffset.Y);

            m_SelectedActor->Bounds.SetPos (point);
            m_SelectedActor->TemplateBounds = m_SelectedActor->Bounds;

            m_SelectedActor->SetPhysOffset (m_SelectedActor->Bounds.GetPos () + m_SelectedActor->Bounds.GetHalfSize ());

            QuadTreeNode& quadTree = m_Editor->GetMainLoop ()->GetSceneManager ().GetActiveScene ()->GetQuadTree ();
            quadTree.Remove (m_SelectedActor);
            quadTree.UpdateStructures ();
            quadTree.Insert (m_SelectedActor);
            quadTree.UpdateStructures ();
        }

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    Actor* EditorActorMode::GetActorUnderCursor (int mouseX, int mouseY, Rect&& outRect)
    {
        std::vector<Actor*>& actors = m_Editor->GetMainLoop ()->GetSceneManager ().GetActiveScene ()->GetActors ();
        Actor* result = nullptr;

        for (Actor* actorIt : actors)
        {
            Rect r = m_Editor->GetMainLoop ()->GetSceneManager ().GetActiveScene ()->GetRenderBounds (actorIt);

            if (InsideRect (mouseX, mouseY, r))
            {
                if ((result == nullptr) || (result && (result->RenderID < actorIt->RenderID)))
                {
                    outRect = r;
                    result = actorIt;

                    m_TileSelectionOffset = { r.GetTopLeft ().X - mouseX, r.GetTopLeft ().Y - mouseY };
                }
            }
        }

        return result;
    }

    //--------------------------------------------------------------------------------------------------

    void EditorActorMode::ChangeRotation (bool clockwise)
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

        if (m_SelectedActor)
        {
            m_SelectedActor->Rotation = m_Rotation;
        }
    }

    //--------------------------------------------------------------------------------------------------

    void EditorActorMode::ChangeZOrder (bool clockwise)
    {
        if (m_SelectedActor)
        {
            m_SelectedActor->ZOrder += clockwise ? -1 : 1;

            int currentID = m_SelectedActor->ID;

            m_Editor->GetMainLoop ()->GetSceneManager ().GetActiveScene ()->SortActors ();

            std::vector<Actor*>& actors = m_Editor->GetMainLoop ()->GetSceneManager ().GetActiveScene ()->GetActors ();

            for (Actor* actor : actors)
            {
                if (actor->ID == currentID)
                {
                    m_SelectedActor = actor;
                    break;
                }
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    bool EditorActorMode::ChooseTile (int mouseX, int mouseY)
    {
        const Point screenSize = m_Editor->GetMainLoop ()->GetScreen ()->GetWindowSize ();
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
                m_SelectedActor = AddTile (mouseX, mouseY);
                m_Editor->SetCursorMode (CursorMode::TileEditMode);
                m_Rotation = m_SelectedActor->Rotation;

                if (!m_SelectedActor->PhysPoints.empty ())
                {
                    m_Editor->GetEditorPhysMode ().SetPhysPoly (&m_SelectedActor->PhysPoints[0]);
                }

                return true;
            }
        }

        return false;
    }

    //--------------------------------------------------------------------------------------------------

    void EditorActorMode::RemoveSelectedTile ()
    {
        if (m_SelectedActor && m_SelectedActor->GetTypeName () == TileActor::TypeName)
        {
            m_Editor->GetMainLoop ()->GetSceneManager ().GetActiveScene ()->RemoveTile (
                dynamic_cast<TileActor*> (m_SelectedActor));
            m_SelectedActor = nullptr;
            m_Editor->SetCursorMode (CursorMode::TileSelectMode);
        }
    }

    //--------------------------------------------------------------------------------------------------

    void EditorActorMode::CopySelectedTile ()
    {
        if (m_SelectedActor && m_SelectedActor->GetTypeName () == TileActor::TypeName)
        {
            m_SelectedAtlasRegion = m_Atlas->GetRegion (m_SelectedActor->Name);

            ALLEGRO_MOUSE_STATE state;
            al_get_mouse_state (&state);

            Actor* currentTile = m_SelectedActor;

            m_SelectedActor = AddTile (state.x, state.y);

            if (currentTile)
            {
                m_SelectedActor->PhysPoints = currentTile->PhysPoints;
            }

            m_Editor->SetCursorMode (CursorMode::TileEditMode);
            m_Rotation = m_SelectedActor->Rotation;
        }
    }

    //--------------------------------------------------------------------------------------------------

    TileActor* EditorActorMode::AddTile (int mouseX, int mouseY)
    {
        TileActor* tile = new TileActor (&m_Editor->GetMainLoop ()->GetSceneManager ());
        AtlasRegion region = m_Atlas->GetRegion (m_SelectedAtlasRegion.Name);
        Point point
            = m_Editor->CalculateCursorPoint (mouseX + m_TileSelectionOffset.X, mouseY + m_TileSelectionOffset.Y);

        tile->ID = Entity::GetNextID ();
        tile->Tileset = m_Atlas->GetName ();
        tile->TileName = m_SelectedAtlasRegion.Name;
        tile->Name = tile->TileName;

        tile->Bounds = { { point.X, point.Y },
                         { point.X + region.Bounds.GetSize ().Width, point.Y + region.Bounds.GetSize ().Height } };
        tile->Rotation = m_Rotation;

        m_Editor->GetMainLoop ()->GetSceneManager ().GetActiveScene ()->AddTile (tile);
        m_Editor->GetMainLoop ()->GetSceneManager ().GetActiveScene ()->SortActors ();

        return tile;
    }

    //--------------------------------------------------------------------------------------------------

    void EditorActorMode::InitializeUI ()
    {
        m_Atlas = m_Editor->GetMainLoop ()->GetSceneManager ().GetAtlasManager ()->GetAtlas (
            GetBaseName (GetResourcePath (PACK_0_0_HOME)));
    }

    //--------------------------------------------------------------------------------------------------

    void EditorActorMode::DrawTiles ()
    {
        std::vector<AtlasRegion>& regions = m_Atlas->GetRegions ();
        const Point windowSize = m_Editor->GetMainLoop ()->GetScreen ()->GetWindowSize ();
        float beginning = windowSize.Width * 0.5 - (TILES_COUNT - 1) * 0.5 * TILE_SIZE - TILE_SIZE * 0.5;
        float advance = 0;

        for (int i = 0; i < TILES_COUNT; ++i)
        {
            advance = beginning + i * TILE_SIZE;

            al_draw_rectangle (advance, windowSize.Height - TILE_SIZE, advance + TILE_SIZE, windowSize.Height,
                               COLOR_GREEN, 1);

            if (i < regions.size () - 1)
            {
                Rect region = regions[i].Bounds;
                al_draw_scaled_bitmap (m_Atlas->GetImage (), region.GetPos ().X, region.GetPos ().Y,
                                       region.GetSize ().Width, region.GetSize ().Height, advance + 1,
                                       windowSize.Height - TILE_SIZE + 1, TILE_SIZE - 2, TILE_SIZE - 2, 0);
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    void EditorActorMode::ResetSettings ()
    {
        m_SelectedActor = nullptr;
        m_TileUnderCursor = nullptr;
    }

    //--------------------------------------------------------------------------------------------------

    void EditorActorMode::ChangeAtlas (const std::string& newAtlasName)
    {
        m_Atlas = m_Editor->GetMainLoop ()->GetSceneManager ().GetAtlasManager ()->GetAtlas (newAtlasName);
    }

    //--------------------------------------------------------------------------------------------------
}
