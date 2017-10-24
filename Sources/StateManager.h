// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __STATE_MANAGER_H__
#define __STATE_MANAGER_H__

#include "Common.h"

namespace aga
{
    class Screen;
    class State;
    class MainLoop;

    class StateManager : public Lifecycle
    {
    public:
        StateManager (MainLoop* mainLoop);
        virtual ~StateManager ();
        bool Initialize ();
        bool Destroy ();

        void SetActiveState (State* state);
        void SetActiveState (const std::string& name);
        std::string GetActiveStateName ();
        void RegisterState (State* state);

        bool Update (float deltaTime);
        void ProcessEvent (ALLEGRO_EVENT* event, float deltaTime);
        void Render (float deltaTime);

        MainLoop* GetMainLoop ();

    private:
        std::map<std::string, State*> m_States;
        State* m_ActiveState;

        MainLoop* m_MainLoop;
    };
}

#endif //   __STATE_MANAGER_H__
