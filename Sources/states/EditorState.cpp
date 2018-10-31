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

        ALLEGRO_DISPLAY* display = m_MainLoop->GetScreen ()->GetDisplay ();

        m_LastWindowSize.X = al_get_display_width (display);
        m_LastWindowSize.Y = al_get_display_height (display);

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
        m_MainLoop->GetAudioManager ().SetEnabled (false);
        m_MainLoop->GetAudioManager ().ClearLastPlayedStreams ();

        m_MainLoop->GetScreen ()->SetWindowSize (m_LastWindowSize);
        m_MainLoop->GetScreen ()->CenterOnScreen ();

        al_show_mouse_cursor (m_MainLoop->GetScreen ()->GetDisplay ());

        m_Editor->BeforeEnter ();

        SceneManager& sceneManager = m_MainLoop->GetSceneManager ();
        Player* player = sceneManager.GetPlayer ();

        if (m_LastEditedScene != nullptr && m_LastEditedScene != sceneManager.GetActiveScene ())
        {
            sceneManager.SetActiveScene (m_LastEditedScene);
            player->TemplateBounds.Pos = player->GetPosition ();
        }

        sceneManager.GetActiveScene ()->ResetAllActorsPositions ();

        //  Reset camera to player
        player->BeforeEnter ();
        player->Move (0, 0.00001);
        player->SetPosition (player->TemplateBounds.Pos);

        if (m_Editor->GetCursorMode () == EditPhysBodyMode)
        {
            m_Editor->SwitchCursorMode ();
        }
    }

    //--------------------------------------------------------------------------------------------------

    void EditorState::AfterLeave ()
    {
        m_LastEditedScene = m_MainLoop->GetSceneManager ().GetActiveScene ();

        ALLEGRO_DISPLAY* display = m_MainLoop->GetScreen ()->GetDisplay ();

        m_LastWindowSize.X = al_get_display_width (display);
        m_LastWindowSize.Y = al_get_display_height (display);
    }

    //--------------------------------------------------------------------------------------------------

    bool EditorState::ProcessEvent (ALLEGRO_EVENT* event, float deltaTime)
    {
        m_Editor->ProcessEvent (event, deltaTime);

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    void EditorState::Update (float deltaTime) { m_Editor->Update (deltaTime); }

    //--------------------------------------------------------------------------------------------------

    void EditorState::Render (float deltaTime) { m_Editor->Render (deltaTime); }

    //--------------------------------------------------------------------------------------------------
}
