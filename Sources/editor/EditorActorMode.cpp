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
        , m_Atlas (nullptr)
    {
    }

    //--------------------------------------------------------------------------------------------------

    EditorActorMode::~EditorActorMode () {}

    //--------------------------------------------------------------------------------------------------

    Actor* EditorActorMode::AddOrUpdateActor (
        int id, const std::string& name, const std::string& actorType, Point pos, float rotation, int zOrder)
    {
        Scene* activeScene = m_Editor->GetMainLoop ()->GetSceneManager ().GetActiveScene ();
        Actor* actor = activeScene->GetActor (id);

        if (actor && actor->GetTypeName () == actorType)
        {
            actor->Name = name;
            actor->Bounds.Pos = pos;
            actor->TemplateBounds.Pos = pos;
            actor->Rotation = rotation;
            actor->ZOrder = zOrder;
        }
        else
        {
            std::vector<ScriptMetaData> pendingScripts;

            if (actor && actor->GetTypeName () != actorType)
			{
				pendingScripts = actor->GetScripts ();
				activeScene->RemoveActor (actor);
            }

            actor = ActorFactory::GetActor (&m_Editor->GetMainLoop ()->GetSceneManager (), actorType);
            actor->Name = name;
            actor->Bounds.Pos = pos;
            actor->TemplateBounds.Pos = pos;
            actor->Rotation = rotation;
            actor->ZOrder = zOrder;

            actor->Initialize ();

            if (!pendingScripts.empty ())
            {
                for (ScriptMetaData scriptData : pendingScripts)
                {
                    actor->AttachScript (scriptData.Name, scriptData.Path);
                }
            }

            activeScene->AddActor (actor);
        }

        activeScene->SortActors ();
        m_Editor->GetEditorActorMode ().Clear ();

        m_SelectedActor = actor;

        return actor;
    }

    //--------------------------------------------------------------------------------------------------

	void EditorActorMode::RemoveActor (const std::string& name)
	{
        Scene* activeScene = m_Editor->GetMainLoop ()->GetSceneManager ().GetActiveScene ();

        activeScene->RemoveActor (name);
        activeScene->SortActors ();
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
        Actor* resultActor = nullptr;

        for (Actor* actorIt : actors)
        {
            Rect r = m_Editor->GetMainLoop ()->GetSceneManager ().GetActiveScene ()->GetRenderBounds (actorIt);

            if (InsideRect (mouseX, mouseY, r))
            {
                if ((resultActor == nullptr) || (resultActor && (resultActor->RenderID < actorIt->RenderID)))
                {
                    outRect = r;
                    resultActor = actorIt;

                    m_TileSelectionOffset = {r.GetTopLeft ().X - mouseX, r.GetTopLeft ().Y - mouseY};
                }
            }
        }

        return resultActor;
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

            Scene* activeScene = m_Editor->GetMainLoop ()->GetSceneManager ().GetActiveScene ();

            activeScene->SortActors ();
            activeScene->RecomputeVisibleEntities (true);

            std::vector<Actor*>& actors = activeScene->GetActors ();

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
        if (!m_Atlas)
        {
            return false;
        }

        const Point screenSize = m_Editor->GetMainLoop ()->GetScreen ()->GetWindowSize ();
        float beginning = screenSize.Width * 0.5 - (TILES_COUNT - 1) * 0.5 * TILE_SIZE - TILE_SIZE * 0.5;
        float advance = 0;
        std::vector<AtlasRegion> regions = m_Atlas->GetRegions ();

        for (int i = 0; i < TILES_COUNT; ++i)
        {
            if (i < regions.size ())
            {
                advance = beginning + i * TILE_SIZE;
                float x = advance;
                float y = screenSize.Height - TILE_SIZE - 1;
                Rect r = Rect{{x, y}, {x + TILE_SIZE, y + TILE_SIZE - 2}};

                if (InsideRect (mouseX, mouseY, r))
                {
                    m_SelectedAtlasRegion = regions[i];
                    m_SelectedActor = AddTile (mouseX, mouseY);
                    m_Rotation = m_SelectedActor->Rotation;

                    //  Orient mouse cursor in middle of tile regarding camera scaling
                    m_TileSelectionOffset = -m_SelectedActor->Bounds.GetHalfSize ()
                        * m_Editor->GetMainLoop ()->GetSceneManager ().GetCamera ().GetScale ();

                    if (!m_SelectedActor->PhysPoints.empty ())
                    {
                        m_Editor->GetEditorPhysMode ().SetPhysPoly (&m_SelectedActor->PhysPoints[0]);
                    }

                    m_Editor->SetCursorMode (CursorMode::TileEditMode);

                    return true;
                }
            }
        }

        return false;
    }

    //--------------------------------------------------------------------------------------------------

    void EditorActorMode::RemoveSelectedActor ()
    {
        if (m_SelectedActor)
        {
            m_Editor->GetMainLoop ()->GetSceneManager ().GetActiveScene ()->RemoveActor (m_SelectedActor);
            m_SelectedActor = nullptr;
            m_Editor->SetCursorMode (CursorMode::TileSelectMode);
        }
    }

    //--------------------------------------------------------------------------------------------------

    void EditorActorMode::CopySelectedActor ()
    {
        if (m_SelectedActor)
        {
            m_SelectedAtlasRegion = m_Atlas->GetRegion (m_SelectedActor->Name);

            ALLEGRO_MOUSE_STATE state;
            al_get_mouse_state (&state);

            Point point = m_Editor->CalculateCursorPoint (state.x, state.y);
            Point regionSize = m_SelectedActor->Bounds.GetSize ();

            Actor* newActor = ActorFactory::GetActor (
                &m_Editor->GetMainLoop ()->GetSceneManager (), m_SelectedActor->GetTypeName ());
            newActor->Name = m_SelectedActor->Name + "_" + ToString (newActor->ID);
            newActor->Bounds = Rect (point.X - regionSize.Width * 0.5f, point.Y - regionSize.Height * 0.5f,
                regionSize.Width, regionSize.Height);
            newActor->TemplateBounds.Pos = m_SelectedActor->Bounds.Pos;
            newActor->Rotation = m_SelectedActor->Rotation;
            newActor->ZOrder = m_SelectedActor->ZOrder;
            newActor->SetAtlas (m_SelectedActor->GetAtlas ());
            newActor->SetAtlasRegionName (m_SelectedActor->GetAtlasRegionName ());

            newActor->PhysPoints = m_SelectedActor->PhysPoints;

            newActor->Initialize ();

            m_Editor->GetMainLoop ()->GetSceneManager ().GetActiveScene ()->AddActor (newActor);

            m_Rotation = newActor->Rotation;
            m_SelectedActor = newActor;
        }

        m_Editor->SetCursorMode (CursorMode::TileEditMode);
    }

    //--------------------------------------------------------------------------------------------------

    TileActor* EditorActorMode::AddTile (int mouseX, int mouseY)
    {
        if (!m_Atlas)
        {
            return nullptr;
        }

        TileActor* tile = new TileActor (&m_Editor->GetMainLoop ()->GetSceneManager ());
        Point regionSize = m_Atlas->GetRegion (m_SelectedAtlasRegion.Name).Bounds.GetSize ();
        Point point = m_Editor->CalculateCursorPoint (mouseX, mouseY);

        tile->ID = Entity::GetNextID ();
        tile->Tileset = m_Atlas->GetName ();
        tile->SetAtlas (m_Atlas);
        tile->SetAtlasRegionName (m_SelectedAtlasRegion.Name);
        tile->Name = tile->GetAtlasRegionName ();

        tile->Bounds = Rect (
            point.X - regionSize.Width * 0.5f, point.Y - regionSize.Height * 0.5f, regionSize.Width, regionSize.Height);
        tile->Rotation = m_Rotation;

        m_Editor->GetMainLoop ()->GetSceneManager ().GetActiveScene ()->AddTile (tile);
        m_Editor->GetMainLoop ()->GetSceneManager ().GetActiveScene ()->SortActors ();

        return tile;
    }

    //--------------------------------------------------------------------------------------------------

    void EditorActorMode::DrawTiles ()
    {
        if (!m_Atlas)
        {
            return;
        }

        std::vector<AtlasRegion>& regions = m_Atlas->GetRegions ();
        const Point windowSize = m_Editor->GetMainLoop ()->GetScreen ()->GetWindowSize ();
        float beginning = windowSize.Width * 0.5 - (TILES_COUNT - 1) * 0.5 * TILE_SIZE - TILE_SIZE * 0.5;
        float advance = 0;

        ALLEGRO_MOUSE_STATE state;
        al_get_mouse_state (&state);

        Point drawNamePoint = Point::MIN_POINT;
        std::string drawName = "";

        for (int i = 0; i < TILES_COUNT; ++i)
        {
            advance = beginning + i * TILE_SIZE;

            al_draw_rectangle (
                advance, windowSize.Height - TILE_SIZE, advance + TILE_SIZE, windowSize.Height - 1, COLOR_GREEN, 1);

            if (i < regions.size ())
            {
                Rect region = regions[i].Bounds;
                al_draw_scaled_bitmap (m_Atlas->GetImage (), region.GetPos ().X, region.GetPos ().Y,
                    region.GetSize ().Width, region.GetSize ().Height, advance + 1, windowSize.Height - TILE_SIZE + 1,
                    TILE_SIZE - 2, TILE_SIZE - 2, 0);

                float x = advance;
                float y = windowSize.Height - TILE_SIZE - 1;
                Rect r = Rect{{x, y}, {x + TILE_SIZE, y + TILE_SIZE - 2}};

                if (InsideRect (state.x, state.y, r))
                {
                    drawNamePoint = {state.x, state.y - 10};
                    drawName = regions[i].Name;
                }
            }
        }

        Font& font = m_Editor->GetMainLoop ()->GetScreen ()->GetFont ();
        font.DrawText (FONT_NAME_SMALL, al_map_rgb (255, 255, 0), drawNamePoint.X, drawNamePoint.Y, drawName,
            ALLEGRO_ALIGN_CENTER);
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
        m_Atlas = m_Editor->GetMainLoop ()->GetAtlasManager ().GetAtlas (newAtlasName);
    }

    //--------------------------------------------------------------------------------------------------
}
