// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "EditorState.h"
#include "Common.h"
#include "Screen.h"
#include "StateManager.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    EditorState::EditorState (StateManager* stateManager)
        : State ("EDITOR")
        , m_StateManager (stateManager)
    {
    }

    //--------------------------------------------------------------------------------------------------

    EditorState::~EditorState ()
    {
        if (!IsDestroyed ())
        {
            Destroy ();
        }

        Lifecycle::Destroy ();
    }

    //--------------------------------------------------------------------------------------------------

    bool EditorState::Initialize ()
    {
        Lifecycle::Initialize ();
    }

    //--------------------------------------------------------------------------------------------------

    bool EditorState::Destroy ()
    {
        Lifecycle::Destroy ();
    }

    //--------------------------------------------------------------------------------------------------

    void EditorState::BeforeEnter ()
    {
    }

    //--------------------------------------------------------------------------------------------------

    void EditorState::AfterLeave ()
    {
    }

    //--------------------------------------------------------------------------------------------------

    void EditorState::ProcessEvent (ALLEGRO_EVENT* event, double deltaTime)
    {
        if (event->type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN)
        {
        }
    }

    //--------------------------------------------------------------------------------------------------

    void EditorState::Update (double deltaTime)
    {
    }

    //--------------------------------------------------------------------------------------------------

    void EditorState::Render (double deltaTime)
    {
    }

    //--------------------------------------------------------------------------------------------------
}
