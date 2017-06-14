// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __STATE_MANAGER_H__
#define __STATE_MANAGER_H__

#include "Common.h"

union ALLEGRO_EVENT;

namespace aga
{
    class Screen;
    class State;

    class StateManager : public Lifecycle
    {
    public:
        StateManager (Screen* screen);
        virtual ~StateManager ();
        bool Initialize ();
        bool Destroy ();

        void SetActiveState (State* state);
        void RegisterState (State* state);

        bool Update (double deltaTime);
        void ProcessEvent (ALLEGRO_EVENT* event, double deltaTime);
        void Render (double deltaTime);

        Screen* GetScreen ();

    private:
        std::vector<State*> m_States;
        State* m_ActiveState;
        Screen* m_Screen;
    };
}

#endif //   __STATE_MANAGER_H__
