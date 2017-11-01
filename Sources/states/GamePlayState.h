// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __GAMEPLAY_STATE_H__
#define __GAMEPLAY_STATE_H__

#include "State.h"

namespace aga
{
    extern std::string GAMEPLAY_STATE_NAME;

    class MainLoop;
    class Scene;

    class GamePlayState : public State
    {
    public:
        GamePlayState (MainLoop* mainLoop);

        virtual ~GamePlayState ();
        bool Initialize ();
        bool Destroy ();

        void BeforeEnter ();
        void AfterLeave ();

        void ProcessEvent (ALLEGRO_EVENT* event, double deltaTime);
        void Update (double deltaTime);
        void Render (double deltaTime);

    private:
        Scene* m_CurrentScene;
    };
}

#endif //   __GAMEPLAY_STATE_H__
