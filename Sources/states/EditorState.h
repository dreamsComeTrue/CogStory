// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __EDITOR_STATE_H__
#define __EDITOR_STATE_H__

#include "editor/Editor.h"
#include "State.h"

namespace aga
{
    class StateManager;

    class EditorState : public State
    {
    public:
        EditorState (StateManager* stateManager);

        virtual ~EditorState ();
        bool Initialize ();
        bool Destroy ();

        void BeforeEnter ();
        void AfterLeave ();

        void ProcessEvent (ALLEGRO_EVENT* event, double deltaTime);
        void Update (double deltaTime);
        void Render (double deltaTime);

    private:
        StateManager* m_StateManager;
        Editor m_Editor;
    };
}

#endif //   __EDITOR_STATE_H__
