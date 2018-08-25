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
        , m_ActorUnderCursor (nullptr)
        , m_Rotation (0)
        , m_Atlas (nullptr)
        , m_CurrentTileBegin (0)
        , m_PrimarySelectedActor (nullptr)
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

        SetSelectedActor (actor);

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

    bool EditorActorMode::MoveSelectedActors ()
    {
        ALLEGRO_MOUSE_STATE state;
        al_get_mouse_state (&state);

        if (state.buttons == 1)
        {
            Point movePoint
                = m_Editor->CalculateWorldPoint (state.x + m_TileSelectionOffset.X, state.y + m_TileSelectionOffset.Y);
            Point deltaPoint = {0, 0};

            if (m_PrimarySelectedActor)
            {
                deltaPoint = movePoint - m_PrimarySelectedActor->Bounds.Pos;

                m_PrimarySelectedActor->Bounds.SetPos (movePoint);
            }

            for (Actor* actor : m_SelectedActors)
            {
                if (m_SelectedActors.size () == 1)
                {
                    actor->Bounds.SetPos (movePoint);
                }
                else
                {
                    if (actor != m_PrimarySelectedActor)
                    {
                        actor->Bounds.Offset (deltaPoint);
                    }
                }

                actor->TemplateBounds = actor->Bounds;

                actor->SetPhysOffset (actor->Bounds.GetPos () + actor->Bounds.GetHalfSize ());

                QuadTreeNode& quadTree = m_Editor->GetMainLoop ()->GetSceneManager ().GetActiveScene ()->GetQuadTree ();
                quadTree.Remove (actor);
                quadTree.UpdateStructures ();
                quadTree.Insert (actor);
                quadTree.UpdateStructures ();
            }
        }

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    Actor* EditorActorMode::GetActorUnderCursor (int mouseX, int mouseY, bool selecting, Rect&& outRect)
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

                    if (selecting)
                    {
                        m_TileSelectionOffset = {r.GetTopLeft ().X - mouseX, r.GetTopLeft ().Y - mouseY};
                    }
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

        for (Actor* actor : m_SelectedActors)
        {
            actor->Rotation = m_Rotation;
        }
    }

    //--------------------------------------------------------------------------------------------------

    void EditorActorMode::ChangeZOrder (bool clockwise)
    {
        std::vector<int> selectedIDs;

        for (Actor* actor : m_SelectedActors)
        {
            actor->ZOrder += clockwise ? -1 : 1;
            selectedIDs.push_back (actor->ID);
        }

        Scene* activeScene = m_Editor->GetMainLoop ()->GetSceneManager ().GetActiveScene ();

        activeScene->SortActors ();
        activeScene->RecomputeVisibleEntities (true);

        std::vector<Actor*>& actors = activeScene->GetActors ();
        m_SelectedActors.clear ();

        for (Actor* actor : actors)
        {
            for (int id : selectedIDs)
            {
                if (actor->ID == id)
                {
                    AddActorToSelection ((actor));
                    break;
                }
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    void EditorActorMode::AddActor (int mouseX, int mouseY)
    {
        if (!m_Atlas)
        {
            return;
        }

        TileActor* tile = new TileActor (&m_Editor->GetMainLoop ()->GetSceneManager ());
        Point regionSize = m_Atlas->GetRegion (m_SelectedAtlasRegion.Name).Bounds.GetSize ();
        Point point = m_Editor->CalculateWorldPoint (mouseX, mouseY);

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

        SetSelectedActor (tile);

        m_Rotation = tile->Rotation;

        //  Orient mouse cursor in middle of tile regarding camera scaling
        m_TileSelectionOffset
            = -tile->Bounds.GetHalfSize () * m_Editor->GetMainLoop ()->GetSceneManager ().GetCamera ().GetScale ();

        if (!tile->PhysPoints.empty ())
        {
            m_Editor->GetEditorPhysMode ().SetPhysPoly (&tile->PhysPoints[0]);
        }
    }

    //--------------------------------------------------------------------------------------------------

    bool EditorActorMode::ChooseTile (int mouseX, int mouseY)
    {
        if (!m_Atlas)
        {
            return false;
        }

        if (m_Editor->GetCursorMode () == CursorMode::EditSpriteSheetMode && ChooseTileFromSpriteSheet (mouseX, mouseY))
        {
            return true;
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
                    m_SelectedAtlasRegion = regions[m_CurrentTileBegin + i];
                    return true;
                }
            }
        }

        return false;
    }

    //--------------------------------------------------------------------------------------------------

    bool EditorActorMode::ChooseTileFromSpriteSheet (int mouseX, int mouseY)
    {
        std::vector<AtlasRegion>& regions = m_Atlas->GetRegions ();
        Point mouseCursor = m_Editor->CalculateWorldPoint (mouseX, mouseY);

        for (AtlasRegion& region : regions)
        {
            if (InsideRect (mouseCursor.X, mouseCursor.Y, region.Bounds))
            {
                m_SelectedAtlasRegion = region;

                return true;
            }
        }

        return false;
    }

    //--------------------------------------------------------------------------------------------------

    void EditorActorMode::RenderSpriteSheet ()
    {
        ALLEGRO_BITMAP* image = m_Atlas->GetImage ();
        int imageWidth = al_get_bitmap_width (image);
        int imageHeight = al_get_bitmap_height (image);

        al_draw_scaled_bitmap (image, 0, 0, imageWidth, imageHeight, 0, 0, imageWidth, imageHeight, 0);

        std::vector<AtlasRegion>& regions = m_Atlas->GetRegions ();

        ALLEGRO_MOUSE_STATE state;
        al_get_mouse_state (&state);

        Point mouseCursor = m_Editor->CalculateWorldPoint (state.x, state.y);

        for (AtlasRegion& region : regions)
        {
            Rect bounds = region.Bounds;

            if (InsideRect (mouseCursor.X, mouseCursor.Y, bounds))
            {
                ALLEGRO_COLOR selectColor = COLOR_YELLOW;
                selectColor.a = 0.5f;

                int blendOp, blendSrc, blendDst;
                al_get_blender (&blendOp, &blendSrc, &blendDst);
                al_set_blender (ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA);

                al_draw_rectangle (bounds.GetTopLeft ().X, bounds.GetTopLeft ().Y, bounds.GetBottomRight ().X,
                    bounds.GetBottomRight ().Y, COLOR_YELLOW, 1);
                al_draw_filled_rectangle (bounds.GetTopLeft ().X, bounds.GetTopLeft ().Y, bounds.GetBottomRight ().X,
                    bounds.GetBottomRight ().Y, selectColor);

                al_set_blender (blendOp, blendSrc, blendDst);
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    void EditorActorMode::RemoveSelectedActors ()
    {
        for (Actor* actor : m_SelectedActors)
        {
            m_Editor->GetMainLoop ()->GetSceneManager ().GetActiveScene ()->RemoveActor (actor);
        }

        m_SelectedActors.clear ();
        m_Editor->SetCursorMode (CursorMode::ActorSelectMode);
    }

    //--------------------------------------------------------------------------------------------------

    void EditorActorMode::CopySelectedActor ()
    {
        for (Actor* selectedActor : m_SelectedActors)
        {
            m_SelectedAtlasRegion = m_Atlas->GetRegion (selectedActor->Name);

            ALLEGRO_MOUSE_STATE state;
            al_get_mouse_state (&state);

            Point point = m_Editor->CalculateWorldPoint (state.x, state.y);
            Point regionSize = selectedActor->Bounds.GetSize ();

            Actor* newActor
                = ActorFactory::GetActor (&m_Editor->GetMainLoop ()->GetSceneManager (), selectedActor->GetTypeName ());
            newActor->Name = selectedActor->GetAtlasRegionName ();
            newActor->Bounds = Rect (point.X - regionSize.Width * 0.5f, point.Y - regionSize.Height * 0.5f,
                regionSize.Width, regionSize.Height);
            newActor->TemplateBounds = newActor->Bounds;
            newActor->Rotation = selectedActor->Rotation;
            newActor->ZOrder = selectedActor->ZOrder;
            newActor->SetAtlas (selectedActor->GetAtlas ());
            newActor->SetAtlasRegionName (selectedActor->GetAtlasRegionName ());
            newActor->PhysPoints = selectedActor->PhysPoints;
            newActor->SetPhysOffset (newActor->Bounds.GetPos () + newActor->Bounds.GetHalfSize ());

            newActor->Initialize ();

            m_Editor->GetMainLoop ()->GetSceneManager ().GetActiveScene ()->AddActor (newActor);

            m_Rotation = newActor->Rotation;
            SetSelectedActor (newActor);
        }
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
                Rect region = regions[m_CurrentTileBegin + i].Bounds;
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
        m_SelectedActors.clear ();
        m_TileUnderCursor = nullptr;
    }

    //--------------------------------------------------------------------------------------------------

    void EditorActorMode::ScrollNextTile (int offset)
    {
        m_CurrentTileBegin += offset;

        std::vector<AtlasRegion>& regions = m_Atlas->GetRegions ();

        if (m_CurrentTileBegin >= regions.size () - 1)
        {
            m_CurrentTileBegin = regions.size () - 1;
        }
    }

    //--------------------------------------------------------------------------------------------------

    void EditorActorMode::ScrollPrevTile (int offset)
    {
        m_CurrentTileBegin -= offset;

        if (m_CurrentTileBegin < 0)
        {
            m_CurrentTileBegin = 0;
        }
    }

    //--------------------------------------------------------------------------------------------------

    void EditorActorMode::ChangeAtlas (const std::string& newAtlasName)
    {
        m_Atlas = m_Editor->GetMainLoop ()->GetAtlasManager ().GetAtlas (newAtlasName);
        m_CurrentTileBegin = 0;
    }

    //--------------------------------------------------------------------------------------------------

    void EditorActorMode::SetSelectedActor (Actor* actor)
    {
        ClearSelectedActors ();
        AddActorToSelection (actor);
    }

    //--------------------------------------------------------------------------------------------------

    void EditorActorMode::ClearSelectedActors ()
    {
        m_SelectedActors.clear ();
        m_PrimarySelectedActor = nullptr;
    }

    //--------------------------------------------------------------------------------------------------

    bool EditorActorMode::IsActorSelected (Actor* actor)
    {
        for (std::vector<Actor*>::iterator it = m_SelectedActors.begin (); it != m_SelectedActors.end (); ++it)
        {
            if (*it == actor)
            {
                return true;
            }
        }

        return false;
    }

    //--------------------------------------------------------------------------------------------------

    void EditorActorMode::AddActorToSelection (Actor* actor)
    {
        if (!IsActorSelected (actor))
        {
            m_SelectedActors.push_back (actor);
        }

        m_PrimarySelectedActor = actor;
    }

    //--------------------------------------------------------------------------------------------------

    void EditorActorMode::RemoveActorFromSelection (Actor* actor)
    {
        for (std::vector<Actor*>::iterator it = m_SelectedActors.begin (); it != m_SelectedActors.end (); ++it)
        {
            if (*it == actor)
            {
                m_SelectedActors.erase (it);
                break;
            }
        }

        m_PrimarySelectedActor = nullptr;
    }

    //--------------------------------------------------------------------------------------------------
}
