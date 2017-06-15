// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __GAMEPLAY_STATE_H__
#define __GAMEPLAY_STATE_H__

#include "SceneManager.h"
#include "State.h"

namespace aga
{
    class StateManager;

    class GamePlayState : public State
    {
    public:
        GamePlayState (StateManager* stateManager);

        virtual ~GamePlayState ();
        bool Initialize ();
        bool Destroy ();

        void BeforeEnter ();
        void AfterLeave ();

        void ProcessEvent (ALLEGRO_EVENT* event, double deltaTime);
        void Update (double deltaTime);
        void Render (double deltaTime);

    private:
        StateManager* m_StateManager;
        SceneManager m_SceneManager;
    };
}

#endif //   __GAMEPLAY_STATE_H__
