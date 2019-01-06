// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "EditorActorMode.h"
#include "ActorFactory.h"
#include "Editor.h"
#include "MainLoop.h"
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
        , m_Atlas (nullptr)
        , m_Rotation (0)
        , m_ActorUnderCursor (nullptr)
        , m_PrimarySelectedActor (nullptr)
        , m_CurrentTileBegin (0)
        , m_SpriteSheetChoosen (false)
    {
    }

    //--------------------------------------------------------------------------------------------------

    EditorActorMode::~EditorActorMode () {}

    //--------------------------------------------------------------------------------------------------

    Actor* EditorActorMode::AddOrUpdateActor (int id, const std::string& name, const std::string& actorType,
        int blueprintID, Point pos, float rotation, int zOrder, float focusHeight)
    {
        Scene* activeScene = m_Editor->GetMainLoop ()->GetSceneManager ().GetActiveScene ();
        Actor* actor = activeScene->GetActor (id);

        if (actor && actor->GetTypeName () == actorType)
        {
            actor->Name = name;
            actor->BlueprintID = blueprintID;
            actor->Bounds.Pos = pos;
            actor->TemplateBounds.Pos = pos;
            actor->Rotation = rotation;
            actor->ZOrder = zOrder;
            actor->SetFocusHeight (focusHeight);
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
            actor->BlueprintID = blueprintID;
            actor->Bounds.Pos = pos;
            actor->TemplateBounds.Pos = pos;
            actor->Rotation = rotation;
            actor->ZOrder = zOrder;
            actor->SetFocusHeight (focusHeight);

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

    void EditorActorMode::RemoveActor (int id)
    {
        Scene* activeScene = m_Editor->GetMainLoop ()->GetSceneManager ().GetActiveScene ();

        activeScene->RemoveActor (id);
        activeScene->SortActors ();
    }

    //--------------------------------------------------------------------------------------------------

    void EditorActorMode::Clear () {}

    //--------------------------------------------------------------------------------------------------

    bool EditorActorMode::MoveSelectedActors (float moveX, float moveY)
    {
        Point movePoint = m_Editor->CalculateWorldPoint (
            static_cast<int> (moveX + m_TileSelectionOffset.X), static_cast<int> (moveY + m_TileSelectionOffset.Y));
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

            std::vector<Entity*> children = actor->GetBluprintChildren ();

            for (Entity* child : children)
            {
                child->Rotation = actor->Rotation;
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    void EditorActorMode::ChangeZOrder (bool clockwise)
    {
        Scene* activeScene = m_Editor->GetMainLoop ()->GetSceneManager ().GetActiveScene ();

        std::vector<int> selectedIDs;

        for (Actor* actor : m_SelectedActors)
        {
            actor->ZOrder += clockwise ? -1 : 1;
            selectedIDs.push_back (actor->ID);

            std::vector<Entity*> children = actor->GetBluprintChildren ();

            for (Entity* child : children)
            {
                child->ZOrder = actor->ZOrder;
            }
        }

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
                    m_SelectedActors.push_back (actor);
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

        ClearSelectedActors ();

        Point primaryPos;

        for (AtlasRegion& region : m_SelectedAtlasRegions)
        {
            Actor* tile = new TileActor (&m_Editor->GetMainLoop ()->GetSceneManager ());
            Point regionSize = m_Atlas->GetRegion (region.Name).Bounds.GetSize ();
            Point point = m_Editor->CalculateWorldPoint (mouseX, mouseY);

            tile->SetAtlas (m_Atlas);
            tile->SetAtlasName (m_Atlas->GetName ());
            tile->SetAtlasRegionName (region.Name);
            tile->Name = tile->GetAtlasRegionName ();
            tile->Rotation = m_Rotation;

            Point newPos;

            if (!m_PrimarySelectedActor)
            {
                m_PrimarySelectedActor = tile;
                primaryPos = region.Bounds.Pos;
                newPos = point;
            }
            else
            {
                newPos = m_PrimarySelectedActor->Bounds.Pos + (region.Bounds.Pos - primaryPos);
            }

            tile->Bounds = Rect (newPos.X, newPos.Y, regionSize.Width, regionSize.Height);

            tile->Initialize ();

            m_Editor->GetMainLoop ()->GetSceneManager ().GetActiveScene ()->AddActor (tile);
            m_Editor->GetMainLoop ()->GetSceneManager ().GetActiveScene ()->SortActors ();

            m_SelectedActors.push_back (tile);

            m_Rotation = tile->Rotation;

            //  Orient mouse cursor in middle of tile regarding camera scaling
            m_TileSelectionOffset
                = -tile->Bounds.GetHalfSize () * m_Editor->GetMainLoop ()->GetSceneManager ().GetCamera ().GetScale ();

            if (!tile->PhysPoints.empty ())
            {
                m_Editor->GetEditorPhysMode ().SetPhysPoly (&tile->PhysPoints[0]);
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

        m_SelectedAtlasRegions.clear ();

        const Point screenSize = m_Editor->GetMainLoop ()->GetScreen ()->GetWindowSize ();
        float beginning = screenSize.Width * 0.5f - (TILES_COUNT - 1) * 0.5f * TILE_SIZE - TILE_SIZE * 0.5f;
        float advance = 0;
        std::vector<AtlasRegion> regions = m_Atlas->GetRegions ();

        for (size_t i = 0; i < TILES_COUNT; ++i)
        {
            if (i < regions.size ())
            {
                advance = beginning + i * TILE_SIZE;
                float x = advance;
                float y = screenSize.Height - TILE_SIZE - 1;
                Rect r = Rect {{x, y}, {x + TILE_SIZE, y + TILE_SIZE - 2}};

                if (InsideRect (mouseX, mouseY, r))
                {
                    m_SelectedAtlasRegions.push_back (regions[m_CurrentTileBegin + i]);
                    return true;
                }
            }
        }

        return false;
    }

    //--------------------------------------------------------------------------------------------------

    bool EditorActorMode::ChooseTilesFromSpriteSheet ()
    {
        std::vector<AtlasRegion>& regions = m_Atlas->GetRegions ();

        m_SelectedAtlasRegions.clear ();

        Rect r = m_Editor->GetSelectionRect ();
        Rect selectionRect
            = OrientRect (r.GetTopLeft ().X, r.GetTopLeft ().Y, r.GetBottomRight ().X, r.GetBottomRight ().Y);

        for (AtlasRegion& region : regions)
        {
            if (Intersect (selectionRect, region.Bounds))
            {
                m_SelectedAtlasRegions.push_back (region);
                m_SpriteSheetChoosen = true;
            }
        }

        return !m_SelectedAtlasRegions.empty ();
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
        Rect r = m_Editor->GetSelectionRect ();
        Rect selectionRect
            = OrientRect (r.GetTopLeft ().X, r.GetTopLeft ().Y, r.GetBottomRight ().X, r.GetBottomRight ().Y);

        for (AtlasRegion& region : regions)
        {
            Rect bounds = region.Bounds;

            if (InsideRect (mouseCursor.X, mouseCursor.Y, bounds) || Intersect (selectionRect, bounds))
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

    void EditorActorMode::CopySelectedActors (bool changeSelection, bool linkWithParent)
    {
        ALLEGRO_MOUSE_STATE state;
        al_get_mouse_state (&state);

        Point deltaPoint = {0, 0};

        if (m_PrimarySelectedActor)
        {
            Point mousePoint = m_Editor->CalculateWorldPoint (state.x, state.y);
            deltaPoint = mousePoint - m_PrimarySelectedActor->Bounds.Pos;
        }

        m_SelectedAtlasRegions.clear ();

        std::vector<Actor*> actorsToSelect;

        for (Actor* selectedActor : m_SelectedActors)
        {
            m_SelectedAtlasRegions.push_back (m_Atlas->GetRegion (selectedActor->Name));

            Point regionSize = selectedActor->Bounds.GetSize ();

            Actor* newActor
                = ActorFactory::GetActor (&m_Editor->GetMainLoop ()->GetSceneManager (), selectedActor->GetTypeName ());
            newActor->Name = selectedActor->GetAtlasRegionName ();

            if (linkWithParent)
            {
                newActor->BlueprintID = selectedActor->ID;
            }

            newActor->Bounds.Pos = selectedActor->Bounds.Pos + deltaPoint;
            newActor->Bounds.Size = regionSize;
            newActor->TemplateBounds = newActor->Bounds;
            newActor->Rotation = selectedActor->Rotation;
            newActor->ZOrder = selectedActor->ZOrder;
            newActor->SetFocusHeight (selectedActor->GetFocusHeight ());
            newActor->SetAtlas (selectedActor->GetAtlas ());
            newActor->SetAtlasRegionName (selectedActor->GetAtlasRegionName ());
            newActor->PhysPoints = selectedActor->PhysPoints;
            newActor->SetPhysOffset (newActor->Bounds.GetPos () + newActor->Bounds.GetHalfSize ());

            newActor->Initialize ();

            m_Editor->GetMainLoop ()->GetSceneManager ().GetActiveScene ()->AddActor (newActor);

            m_Rotation = newActor->Rotation;
            actorsToSelect.push_back (newActor);
        }

        if (changeSelection)
        {
            ClearSelectedActors ();
            
            for (Actor* actor : actorsToSelect) 
            {
                AddActorToSelection (actor);
            }
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
        float beginning = windowSize.Width * 0.5f - (TILES_COUNT - 1) * 0.5f * TILE_SIZE - TILE_SIZE * 0.5f;
        float advance = 0;

        ALLEGRO_MOUSE_STATE state;
        al_get_mouse_state (&state);

        Point drawNamePoint = Point::MIN_POINT;
        std::string drawName = "";

        for (size_t i = 0; i < TILES_COUNT; ++i)
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
                Rect r = Rect {{x, y}, {x + TILE_SIZE, y + TILE_SIZE - 2}};

                if (InsideRect (state.x, state.y, r))
                {
                    drawNamePoint = {state.x, state.y - 10};
                    drawName = regions[i].Name;
                }
            }
        }

        Font& font = m_Editor->GetMainLoop ()->GetScreen ()->GetFont ();
        font.DrawText (FONT_NAME_SMALL, drawName, al_map_rgb (255, 255, 0), drawNamePoint.X, drawNamePoint.Y, 1.0f,
            ALLEGRO_ALIGN_CENTER);
    }

    //--------------------------------------------------------------------------------------------------

    void EditorActorMode::ResetSettings ()
    {
        m_SelectedActors.clear ();
        m_TileUnderCursor = nullptr;
    }

    //--------------------------------------------------------------------------------------------------

    AtlasRegion* EditorActorMode::GetSelectedAtlasRegion ()
    {
        if (!m_SelectedAtlasRegions.empty ())
        {
            return &m_SelectedAtlasRegions[0];
        }
        else
        {
            return nullptr;
        }
    }

    //--------------------------------------------------------------------------------------------------

    std::vector<Actor*> EditorActorMode::GetSelectedActors () { return m_SelectedActors; }

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

    Actor* EditorActorMode::GetActorUnderCursor () { return m_ActorUnderCursor; }

    //--------------------------------------------------------------------------------------------------

    void EditorActorMode::SetActorUnderCursor (Actor* actor) { m_ActorUnderCursor = actor; }

    //--------------------------------------------------------------------------------------------------

    bool EditorActorMode::IsDrawTiles () const { return m_IsDrawTiles; }

    //--------------------------------------------------------------------------------------------------

    void EditorActorMode::SetDrawTiles (bool draw) { m_IsDrawTiles = draw; }

    //--------------------------------------------------------------------------------------------------

    float EditorActorMode::GetRotation () const { return m_Rotation; }

    //--------------------------------------------------------------------------------------------------

    void EditorActorMode::SetRotation (float rotation) { m_Rotation = rotation; }

    //--------------------------------------------------------------------------------------------------

    Actor* EditorActorMode::GetActor () { return m_Actor; }

    //--------------------------------------------------------------------------------------------------

    void EditorActorMode::SetActor (Actor* actor) { m_Actor = actor; }

    //--------------------------------------------------------------------------------------------------

    void EditorActorMode::SetPrimarySelectedActor (Actor* actor) { m_PrimarySelectedActor = actor; }

    //--------------------------------------------------------------------------------------------------

    bool EditorActorMode::IsSpriteSheetChoosen () { return m_SpriteSheetChoosen; }

    //--------------------------------------------------------------------------------------------------

    void EditorActorMode::SetSpriteSheetChoosen (bool choosen) { m_SpriteSheetChoosen = choosen; }

    //--------------------------------------------------------------------------------------------------
}
