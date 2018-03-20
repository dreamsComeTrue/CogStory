// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "EditorState.h"
#include "Camera.h"
#include "Common.h"
#include "MainLoop.h"
#include "Scene.h"
#include "SceneManager.h"
#include "Screen.h"
#include "StateManager.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    EditorState::EditorState (MainLoop* mainLoop)
        : State (mainLoop, EDITOR_STATE_NAME)
        , m_LastEditedScene (nullptr)
    {
    }

    //--------------------------------------------------------------------------------------------------

    EditorState::~EditorState ()
    {
        if (!IsDestroyed ())
        {
            Destroy ();
        }
    }

    //--------------------------------------------------------------------------------------------------

    bool EditorState::Initialize ()
    {
        Lifecycle::Initialize ();

        m_Editor = new Editor (m_MainLoop);
        m_Editor->Initialize ();

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    bool EditorState::Destroy ()
    {
        SAFE_DELETE (m_Editor);

        return Lifecycle::Destroy ();
    }

    //--------------------------------------------------------------------------------------------------

    void EditorState::BeforeEnter ()
    {
        m_Editor->OnResetScale ();
        m_Editor->OnResetTranslate ();
        m_Editor->SetDrawUITiles (true);

        if (m_LastEditedScene != nullptr && m_LastEditedScene != m_MainLoop->GetSceneManager ().GetActiveScene ())
        {
            m_MainLoop->GetSceneManager ().SetActiveScene (m_LastEditedScene);
            m_MainLoop->GetSceneManager ().GetPlayer ().TemplateBounds.Pos
                = m_MainLoop->GetSceneManager ().GetPlayer ().GetPosition ();
        }

        m_Editor->GetMainLoop ()->GetSceneManager ().GetActiveScene ()->ResetAllActorsPositions ();
        m_Editor->GetMainLoop ()->GetSceneManager ().GetPlayer ().ResetParticleEmitters ();

        if (m_Editor->GetCursorMode () == EditPhysBodyMode)
        {
            m_Editor->SwitchCursorMode ();
        }
    }

    //--------------------------------------------------------------------------------------------------

    void EditorState::AfterLeave () { m_LastEditedScene = m_MainLoop->GetSceneManager ().GetActiveScene (); }

    //--------------------------------------------------------------------------------------------------

    void EditorState::ProcessEvent (ALLEGRO_EVENT* event, float deltaTime)
    {
        m_Editor->ProcessEvent (event, deltaTime);
    }

    //--------------------------------------------------------------------------------------------------

    void EditorState::Update (float deltaTime) { m_Editor->Update (deltaTime); }

    //--------------------------------------------------------------------------------------------------

    void EditorState::Render (float deltaTime) { m_Editor->Render (deltaTime); }

    //--------------------------------------------------------------------------------------------------
}
