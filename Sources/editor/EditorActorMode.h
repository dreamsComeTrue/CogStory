// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __EDITOR_ACTOR_MODE_H__
#define __EDITOR_ACTOR_MODE_H__

#include "Atlas.h"
#include "Common.h"
#include "Scene.h"

namespace aga
{
    extern const int TILES_COUNT;
    extern const int TILE_SIZE;

    class Editor;
    class EditorPhysMode;
    class Actor;
    class Atlas;

    class EditorActorMode
    {
    public:
        EditorActorMode (Editor* editor);
        virtual ~EditorActorMode ();

        Actor* AddOrUpdateActor (
            int id, const std::string& name, const std::string& actorType, Point pos, float rotation, int zOrder);
        void RemoveActor (const std::string& name);
        void Clear ();

        void ChangeRotation (bool clockwise);
        void ChangeZOrder (bool clockwise);
        bool MoveSelectedActors (float moveX, float moveY);
        Actor* GetActorUnderCursor (int mouseX, int mouseY, bool selecting, Rect&& outRect);

        void RemoveSelectedActors ();
        void CopySelectedActor ();
        void DrawTiles ();

        void AddActor (int mouseX, int mouseY);

        Atlas* GetAtlas () { return m_Atlas; }
        void ChangeAtlas (const std::string& newAtlasName);
        bool ChooseTile (int mouseX, int mouseY);
        bool ChooseTilesFromSpriteSheet (int mouseX, int mouseY);

        void ResetSettings ();

        AtlasRegion& GetSelectedAtlasRegion () { return m_SelectedAtlasRegion; }

        std::vector<Actor*> GetSelectedActors () { return m_SelectedActors; }
        void ClearSelectedActors ();
        void SetSelectedActor (Actor* actor);
        bool IsActorSelected (Actor* actor);
        void AddActorToSelection (Actor* actor);
        void RemoveActorFromSelection (Actor* actor);

        Actor* GetActorUnderCursor () { return m_ActorUnderCursor; }
        void SetActorUnderCursor (Actor* actor) { m_ActorUnderCursor = actor; }

        bool IsDrawTiles () const { return m_IsDrawTiles; }
        void SetDrawTiles (bool draw) { m_IsDrawTiles = draw; }

        float GetRotation () const { return m_Rotation; }
        void SetRotation (float rotation) { m_Rotation = rotation; }

        Actor* GetActor () { return m_Actor; }
        void SetActor (Actor* actor) { m_Actor = actor; }

        void SetPrimarySelectedActor (Actor* actor) { m_PrimarySelectedActor = actor; }

        void ScrollNextTile (int offset);
        void ScrollPrevTile (int offset);

        void RenderSpriteSheet ();

        bool IsSpriteSheetChoosen () { return m_SpriteSheetChoosen; }
        void SetSpriteSheetChoosen (bool choosen) { m_SpriteSheetChoosen = choosen; }

    private:
        Atlas* m_Atlas;
        AtlasRegion m_SelectedAtlasRegion;

        Editor* m_Editor;
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
