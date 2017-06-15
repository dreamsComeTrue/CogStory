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
        , m_Editor (stateManager->GetScreen ())
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
        m_Editor.Destroy ();

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
        m_Editor.ProcessEvent (event, deltaTime);
    }

    //--------------------------------------------------------------------------------------------------

    void EditorState::Update (double deltaTime)
    {
        m_Editor.Update (deltaTime);
    }

    //--------------------------------------------------------------------------------------------------

    void EditorState::Render (double deltaTime)
    {
        m_Editor.Render (deltaTime);
    }

    //--------------------------------------------------------------------------------------------------
}
