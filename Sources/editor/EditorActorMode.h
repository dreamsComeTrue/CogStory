// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __EDITOR_ACTOR_MODE_H__
#define __EDITOR_ACTOR_MODE_H__

#include "Atlas.h"
#include "Common.h"

namespace aga
{
    extern const int TILES_COUNT;
    extern const int TILE_SIZE;

    class Editor;
    class EditorPhysMode;
    class Actor;
    class Atlas;
    class TileActor;

    class EditorActorMode
    {
    public:
        EditorActorMode (Editor* editor);
        virtual ~EditorActorMode ();

        Actor* AddOrUpdateActor (int id, const std::string& name, const std::string& actorType, int blueprintID,
            Point pos, float rotation, int zOrder, float focusHeight);
        void RemoveActor (int id);
        void Clear ();

        void ChangeRotation (bool clockwise);
        void ChangeZOrder (bool clockwise);
        bool MoveSelectedActors (float moveX, float moveY);
        Actor* GetActorUnderCursor (int mouseX, int mouseY, bool selecting, Rect&& outRect);

        void RemoveSelectedActors ();
        void CopySelectedActors (bool changeSelection, bool linkWithParent);
        void DrawTiles ();

        void AddActor (int mouseX, int mouseY);

        Atlas* GetAtlas () { return m_Atlas; }
        void ChangeAtlas (const std::string& newAtlasName);
        bool ChooseTile (int mouseX, int mouseY);
        bool ChooseTilesFromSpriteSheet ();

        void ResetSettings ();

        AtlasRegion* GetSelectedAtlasRegion ();

        std::vector<Actor*> GetSelectedActors ();
        void ClearSelectedActors ();
        void SetSelectedActor (Actor* actor);
        bool IsActorSelected (Actor* actor);
        void AddActorToSelection (Actor* actor);
        void RemoveActorFromSelection (Actor* actor);

        Actor* GetActorUnderCursor ();
        void SetActorUnderCursor (Actor* actor);

        bool IsDrawTiles () const;
        void SetDrawTiles (bool draw);

        float GetRotation () const;
        void SetRotation (float rotation);

        Actor* GetActor ();
        void SetActor (Actor* actor);

        void SetPrimarySelectedActor (Actor* actor);

        void ScrollNextTile (int offset);
        void ScrollPrevTile (int offset);

        void RenderSpriteSheet ();

        bool IsSpriteSheetChoosen ();
        void SetSpriteSheetChoosen (bool choosen);

    private:
        Editor* m_Editor;

        Atlas* m_Atlas;
        std::vector<AtlasRegion> m_SelectedAtlasRegions;

        Actor* m_Actor;
        float m_Rotation;
        Point m_TileSelectionOffset;
        Actor* m_ActorUnderCursor;
        std::vector<Actor*> m_SelectedActors;

        Actor* m_PrimarySelectedActor;

        bool m_IsDrawTiles;
        TileActor* m_TileUnderCursor;

        int m_CurrentTileBegin;

        bool m_SpriteSheetChoosen;
    };
}

#endif //   __EDITOR_ACTOR_MODE_H__
