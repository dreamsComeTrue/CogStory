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

    std::string StateManager::GetActiveStateName ()
    {
        if (m_ActiveState != nullptr)
        {
            return m_ActiveState->GetName ();
        }

        return "";
    }

    //--------------------------------------------------------------------------------------------------

    void StateManager::RegisterState (State* state)
    {
        std::map<std::string, State*>::iterator it = m_States.find (state->GetName ());

        if (it == m_States.end ())
        {
            m_States.insert (std::make_pair (state->GetName (), state));
        }
    }

    //--------------------------------------------------------------------------------------------------

    bool StateManager::Update (float deltaTime)
    {
        if (m_ActiveState != nullptr)
        {
            m_ActiveState->Update (deltaTime);
        }

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    void StateManager::ProcessEvent (ALLEGRO_EVENT* event, float deltaTime)
    {
        std::string before;
        std::string after;

        if (m_ActiveState != nullptr)
        {
            before = GetActiveStateName ();
            m_ActiveState->ProcessEvent (event, deltaTime);
            after = GetActiveStateName ();
        }

        if (before == after)
        {
            if (event->type == ALLEGRO_EVENT_KEY_UP)
            {
                switch (event->keyboard.keycode)
                {
                    case ALLEGRO_KEY_F1:
                    {
                        if (GetActiveStateName () == "GAMEPLAY_STATE")
                        {
                            SetActiveState ("EDITOR_STATE");
                        }
                        else
                        {
                            SetActiveState ("GAMEPLAY_STATE");
                        }
                    }
                }
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    void StateManager::Render (float deltaTime)
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
