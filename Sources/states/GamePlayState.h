// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __GAMEPLAY_STATE_H__
#define __GAMEPLAY_STATE_H__

#include "State.h"

struct ALLEGRO_BITMAP;

namespace aga
{
    class StateManager;
    class Player;

    class GamePlayState : public State
    {
    public:
        GamePlayState (StateManager* stateManager);

        virtual ~GamePlayState ();
        bool Initialize ();
        bool Destroy ();

        void ProcessEvent (ALLEGRO_EVENT* event);
        void Update ();
        void Render ();

    private:
        StateManager* m_StateManager;
        Player* m_Player;
    };
}

#endif //   __GAMEPLAY_STATE_H__
