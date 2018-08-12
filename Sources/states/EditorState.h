// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __EDITOR_STATE_H__
#define __EDITOR_STATE_H__

#include "State.h"
#include "editor/Editor.h"

namespace aga
{
    const std::string EDITOR_STATE_NAME = "EDITOR_STATE";

    class EditorState : public State
    {
    public:
        EditorState (MainLoop* mainLoop);

        virtual ~EditorState ();
        bool Initialize ();
        bool Destroy ();

        void BeforeEnter ();
        void AfterLeave ();

        bool ProcessEvent (ALLEGRO_EVENT* event, float deltaTime);
        void Update (float deltaTime);
        void Render (float deltaTime);

    private:
        Editor* m_Editor;
        Scene* m_LastEditedScene;
    };
}

#endif //   __EDITOR_STATE_H__
