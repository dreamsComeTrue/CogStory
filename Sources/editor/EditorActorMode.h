// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __EDITOR_ACTOR_MODE_H__
#define __EDITOR_ACTOR_MODE_H__

#include "Common.h"
#include "Scene.h"

namespace aga
{
    class Editor;
    class EditorPhysMode;
    class Actor;

    class EditorActorMode
    {
        friend class Editor;
        friend class EditorActorWindow;
        friend class EditorPhysMode;

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

    private:
        Editor* m_Editor;
        Actor* m_Actor;
        float m_Rotation;
        Point m_TileSelectionOffset;
        Actor* m_ActorUnderCursor;
        Actor* m_SelectedActor;
    };
}

#endif //   __EDITOR_ACTOR_MODE_H__
