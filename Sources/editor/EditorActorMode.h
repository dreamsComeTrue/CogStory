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
        bool MoveSelectedActor ();
        Actor* GetActorUnderCursor (int mouseX, int mouseY, bool selecting, Rect&& outRect);

        void RemoveSelectedActor ();
        void CopySelectedActor ();
        void DrawTiles ();

        void AddActor (int mouseX, int mouseY);

        Atlas* GetAtlas () { return m_Atlas; }
        void ChangeAtlas (const std::string& newAtlasName);
        bool ChooseTile (int mouseX, int mouseY);

        void ResetSettings ();

        AtlasRegion& GetSelectedAtlasRegion () { return m_SelectedAtlasRegion; }

        std::vector<Actor*> GetSelectedActors () { return m_SelectedActors; }
        void ClearSelectedActors () { m_SelectedActors.clear (); }
        void SetSelectedActor (Actor* actor)
        {
            ClearSelectedActors ();
            AddActorToSelection (actor);
        }
        void AddActorToSelection (Actor* actor) { m_SelectedActors.push_back (actor); }

        Actor* GetActorUnderCursor () { return m_ActorUnderCursor; }
        void SetActorUnderCursor (Actor* actor) { m_ActorUnderCursor = actor; }

        bool IsDrawTiles () const { return m_IsDrawTiles; }
        void SetDrawTiles (bool draw) { m_IsDrawTiles = draw; }

        float GetRotation () const { return m_Rotation; }
        void SetRotation (float rotation) { m_Rotation = rotation; }

        Actor* GetActor () { return m_Actor; }
        void SetActor (Actor* actor) { m_Actor = actor; }

        void ScrollNextTile (int offset);
        void ScrollPrevTile (int offset);

        void RenderSpriteSheet ();

    private:
        bool ChooseTileFromSpriteSheet (int mouseX, int mouseY);

    private:
        Atlas* m_Atlas;
        AtlasRegion m_SelectedAtlasRegion;

        Editor* m_Editor;
        Actor* m_Actor;
        float m_Rotation;
        Point m_TileSelectionOffset;
        Actor* m_ActorUnderCursor;
        std::vector<Actor*> m_SelectedActors;

        bool m_IsDrawTiles;
        TileActor* m_TileUnderCursor;

        int m_CurrentTileBegin;
    };
}

#endif //   __EDITOR_ACTOR_MODE_H__
