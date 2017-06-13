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
        m_ActiveState = state;
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

    bool StateManager::Update ()
    {
        if (m_ActiveState != nullptr)
        {
            m_ActiveState->Update ();
        }

        if (!m_Screen->Update ())
        {
            return false;
        }

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    void StateManager::ProcessEvent (ALLEGRO_EVENT* event)
    {
        if (m_ActiveState != nullptr)
        {
            m_ActiveState->ProcessEvent (event);
        }
    }

    //--------------------------------------------------------------------------------------------------

    void StateManager::Render ()
    {
        if (m_ActiveState != nullptr)
        {
            m_ActiveState->Render ();
        }
    }

    //--------------------------------------------------------------------------------------------------
}
