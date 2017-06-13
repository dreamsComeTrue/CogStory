// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "StateManager.h"
#include "Common.h"
#include "Screen.h"
#include "State.h"

#include <algorithm>

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    StateManager::StateManager (Screen* screen)
        : m_Screen (screen)
        , m_ActiveState (nullptr)
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

    bool StateManager::Initialize ()
    {
        Lifecycle::Initialize ();
    }

    //--------------------------------------------------------------------------------------------------

    bool StateManager::Destroy ()
    {
        Lifecycle::Destroy ();
    }

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

    void StateManager::RegisterState (State* state)
    {
        if (std::find (m_States.begin (), m_States.end (), state) == m_States.end ())
        {
            m_States.push_back (state);
        }
    }

    //--------------------------------------------------------------------------------------------------

    bool StateManager::Update (double deltaTime)
    {
        if (m_ActiveState != nullptr)
        {
            m_ActiveState->Update (deltaTime);
        }

        if (!m_Screen->Update (deltaTime))
        {
            return false;
        }

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    void StateManager::ProcessEvent (ALLEGRO_EVENT* event, double deltaTime)
    {
        if (m_ActiveState != nullptr)
        {
            m_ActiveState->ProcessEvent (event, deltaTime);
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
}
