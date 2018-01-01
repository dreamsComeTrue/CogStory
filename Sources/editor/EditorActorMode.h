// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __EDITOR_ACTOR_MODE_H__
#define __EDITOR_ACTOR_MODE_H__

#include "Common.h"
#include "Scene.h"

namespace aga
{
    class Editor;
    class Actor;

    class EditorActorMode
    {
        friend class Editor;
        friend class EditorActorWindow;

    public:
        EditorActorMode (Editor* editor);
        virtual ~EditorActorMode ();

        bool AddOrUpdateActor (const std::string& oldName, const std::string& actorType);
        void RemoveActor (const std::string& name);
        void Clear ();

    private:
        Editor* m_Editor;
        Actor* m_Actor;
    };
}

#endif //   __EDITOR_ACTOR_MODE_H__
