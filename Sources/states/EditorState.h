// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __EDITOR_STATE_H__
#define __EDITOR_STATE_H__

#include "State.h"
#include "editor/Editor.h"

namespace aga
{
    extern std::string EDITOR_STATE_NAME;

    class EditorState : public State
    {
    public:
        EditorState (MainLoop* mainLoop);

        virtual ~EditorState ();
        bool Initialize ();
        bool Destroy ();

        void BeforeEnter ();
        void AfterLeave ();

        void ProcessEvent (ALLEGRO_EVENT* event, double deltaTime);
        void Update (double deltaTime);
        void Render (double deltaTime);

    private:
        Editor* m_Editor;
    };
}

#endif //   __EDITOR_STATE_H__
