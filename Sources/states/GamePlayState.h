// Copyright 2017-2019 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __GAMEPLAY_STATE_H__
#define __GAMEPLAY_STATE_H__

#include "State.h"

namespace aga
{
    const std::string GAMEPLAY_STATE_NAME = "GAMEPLAY_STATE";
    extern const Point GAME_WINDOW_SIZE;
    extern const float GAME_WINDOW_SCALE;

    class MainLoop;
    class Script;

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
        void ResizeWindow ();

    private:
        bool m_AudioWasEnabled;
        Script* m_MasterScript;
    };
}

#endif //   __GAMEPLAY_STATE_H__
