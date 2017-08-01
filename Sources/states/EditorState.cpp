// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "EditorState.h"
#include "Camera.h"
#include "Common.h"
#include "SceneManager.h"
#include "MainLoop.h"
#include "Screen.h"
#include "StateManager.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    EditorState::EditorState (MainLoop* mainLoop)
        : State (mainLoop, "EDITOR")
        , m_Editor (mainLoop->GetScreen ())
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
        m_Editor.Initialize ();

        Lifecycle::Initialize ();
    }

    //--------------------------------------------------------------------------------------------------

    bool EditorState::Destroy ()
    {
        m_Editor.Destroy ();

        Lifecycle::Destroy ();
    }

    //--------------------------------------------------------------------------------------------------

    void EditorState::BeforeEnter () { m_MainLoop->GetSceneManager ()->GetCamera ().Reset(); }

    //--------------------------------------------------------------------------------------------------

    void EditorState::AfterLeave () {}

    //--------------------------------------------------------------------------------------------------

    void EditorState::ProcessEvent (ALLEGRO_EVENT* event, double deltaTime)
    {
        m_Editor.ProcessEvent (event, deltaTime);
    }

    //--------------------------------------------------------------------------------------------------

    void EditorState::Update (double deltaTime) { m_Editor.Update (deltaTime); }

    //--------------------------------------------------------------------------------------------------

    void EditorState::Render (double deltaTime) { m_Editor.Render (deltaTime); }

    //--------------------------------------------------------------------------------------------------
}
