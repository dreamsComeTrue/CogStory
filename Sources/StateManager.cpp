// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "StateManager.h"
#include "Common.h"
#include "MainLoop.h"
#include "Screen.h"
#include "State.h"

#include <algorithm>

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    StateManager::StateManager (MainLoop* mainLoop)
        : m_ActiveState (nullptr)
        , m_MainLoop (mainLoop)
    {
    }

    //--------------------------------------------------------------------------------------------------

    StateManager::~StateManager ()
    {
        if (!IsDestroyed ())
        {
            Destroy ();
        }

        Lifecycle::Destroy ();
    }

    //--------------------------------------------------------------------------------------------------

    bool StateManager::Initialize () { Lifecycle::Initialize (); }

    //--------------------------------------------------------------------------------------------------

    bool StateManager::Destroy () { Lifecycle::Destroy (); }

    //--------------------------------------------------------------------------------------------------

    void StateManager::SetActiveState (State* state)
    {
        if (m_ActiveState != nullptr)
        {
            m_ActiveState->AfterLeave ();
        }

        m_ActiveState = state;
        m_ActiveState->BeforeEnter ();
    }

    //--------------------------------------------------------------------------------------------------

    void StateManager::SetActiveState (const std::string& name)
    {
        std::map<std::string, State*>::iterator it = m_States.find (name);

        if (it != m_States.end ())
        {
            SetActiveState ((*it).second);
        }
    }

    //--------------------------------------------------------------------------------------------------

    void StateManager::RegisterState (const std::string& name, State* state)
    {
        std::map<std::string, State*>::iterator it = m_States.find (name);

        if (it == m_States.end ())
        {
            m_States.insert (std::make_pair (name, state));
        }
    }

    //--------------------------------------------------------------------------------------------------

    bool StateManager::Update (double deltaTime)
    {
        if (m_ActiveState != nullptr)
        {
            m_ActiveState->Update (deltaTime);
        }

        if (!m_MainLoop->GetScreen ()->Update (deltaTime))
        {
            return false;
        }

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    bool isEditor = false;

    void StateManager::ProcessEvent (ALLEGRO_EVENT* event, double deltaTime)
    {
        if (m_ActiveState != nullptr)
        {
            m_ActiveState->ProcessEvent (event, deltaTime);
        }

        if (event->type == ALLEGRO_EVENT_KEY_DOWN)
        {
            ALLEGRO_KEYBOARD_STATE state;
            al_get_keyboard_state (&state);

            if (al_key_down (&state, ALLEGRO_KEY_ENTER))
            {
                isEditor = !isEditor;

                if (isEditor)
                {
                    SetActiveState ("EDITOR");
                }
                else
                {
                    SetActiveState ("GAMEPLAY");
                }
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    void StateManager::Render (double deltaTime)
    {
        if (m_ActiveState != nullptr)
        {
            m_ActiveState->Render (deltaTime);
        }
    }

    //--------------------------------------------------------------------------------------------------

    MainLoop* StateManager::GetMainLoop () { return m_MainLoop; }

    //--------------------------------------------------------------------------------------------------
}
