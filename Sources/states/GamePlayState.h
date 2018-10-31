// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __GAMEPLAY_STATE_H__
#define __GAMEPLAY_STATE_H__

#include "State.h"

namespace aga
{
    const std::string GAMEPLAY_STATE_NAME = "GAMEPLAY_STATE";

    class MainLoop;

    class GamePlayState : public State
    {
    public:
        GamePlayState (MainLoop* mainLoop);

        virtual ~GamePlayState ();
        bool Initialize ();
        bool Destroy ();

        void BeforeEnter ();
        void AfterLeave ();

        bool ProcessEvent (ALLEGRO_EVENT* event, float deltaTime);
        void Update (float deltaTime);
        void Render (float deltaTime);

    private:
        void ResizeWindow ();

    private:
        bool m_AudioWasEnabled;
    };
}

#endif //   __GAMEPLAY_STATE_H__
