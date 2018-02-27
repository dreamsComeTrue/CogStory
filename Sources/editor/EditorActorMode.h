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

        bool AddOrUpdateActor (const std::string& name, const std::string& actorType, Point pos, float rotation);
        void RemoveActor (const std::string& name);
        void Clear ();

        void ChangeRotation (bool clockwise);
        void ChangeZOrder (bool clockwise);
        bool MoveSelectedActor ();
        Actor* GetActorUnderCursor (int mouseX, int mouseY, Rect&& outRect);

        TileActor* AddTile (int mouseX, int mouseY);
        void RemoveSelectedTile ();
        void CopySelectedTile ();
        void DrawTiles ();

        void ChangeAtlas (const std::string& newAtlasName);
        bool ChooseTile (int mouseX, int mouseY);

        void ResetSettings ();

        void InitializeUI ();

        AtlasRegion& GetSelectedAtlasRegion () { return m_SelectedAtlasRegion; }

        Actor* GetSelectedActor () { return m_SelectedActor; }
        void SetSelectedActor (Actor* actor) { m_SelectedActor = actor; }

        Actor* GetActorUnderCursor () { return m_ActorUnderCursor; }

        bool IsDrawTiles () const { return m_IsDrawTiles; }
        void SetDrawTiles (bool draw) { m_IsDrawTiles = draw; }

        float GetRotation () const { return m_Rotation; }
        void SetRotation (float rotation) { m_Rotation = rotation; }

        Actor* GetActor () { return m_Actor; }
        void SetActor (Actor* actor) { m_Actor = actor; }

    private:
        Atlas* m_Atlas;
        AtlasRegion m_SelectedAtlasRegion;

        Editor* m_Editor;
        Actor* m_Actor;
        float m_Rotation;
        Point m_TileSelectionOffset;
        Actor* m_ActorUnderCursor;
        Actor* m_SelectedActor;

        bool m_IsDrawTiles;
        TileActor* m_TileUnderCursor;
    };
}

#endif //   __EDITOR_ACTOR_MODE_H__
