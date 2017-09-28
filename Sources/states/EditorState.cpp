// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "EditorState.h"
#include "Camera.h"
#include "Common.h"
#include "MainLoop.h"
#include "SceneManager.h"
#include "Screen.h"
#include "StateManager.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    std::string EDITOR_STATE_NAME = "EDITOR_STATE";

    EditorState::EditorState (MainLoop* mainLoop)
      : State (mainLoop, EDITOR_STATE_NAME)
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
        m_Editor = new Editor (m_MainLoop);
        m_Editor->Initialize ();

        Lifecycle::Initialize ();
    }

    //--------------------------------------------------------------------------------------------------

    bool EditorState::Destroy ()
    {
        m_Editor->Destroy ();

        Lifecycle::Destroy ();
    }

    //--------------------------------------------------------------------------------------------------

    void EditorState::BeforeEnter ()
    {
        m_MainLoop->GetScreen ()->SetBackgroundColor (SDL_Color{ 50, 60, 100, 0 });
        m_MainLoop->GetSceneManager ()->GetCamera ().UseIdentityTransform ();
    }

    //--------------------------------------------------------------------------------------------------

    void EditorState::AfterLeave () { m_MainLoop->GetSceneManager ()->GetCamera ().ClearTransformations (); }

    //--------------------------------------------------------------------------------------------------

    void EditorState::ProcessEvent (SDL_Event* event, double deltaTime) { m_Editor->ProcessEvent (event, deltaTime); }

    //--------------------------------------------------------------------------------------------------

    void EditorState::Update (double deltaTime) { m_Editor->Update (deltaTime); }

    //--------------------------------------------------------------------------------------------------

    void EditorState::Render (double deltaTime) { m_Editor->Render (deltaTime); }

    //--------------------------------------------------------------------------------------------------
}
