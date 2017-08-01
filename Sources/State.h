// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __STATE_H__
#define __STATE_H__

#include "Lifecycle.h"

#include <string>

union ALLEGRO_EVENT;

namespace aga
{
    class MainLoop;

    class State : public Lifecycle
    {
    public:
        State (MainLoop* mainLoop, const std::string& name)
            : m_MainLoop (mainLoop)
            , m_Name (name)
        {
        }

        virtual void BeforeEnter () = 0;
        virtual void AfterLeave () = 0;

        virtual void ProcessEvent (ALLEGRO_EVENT* event, double deltaTime) = 0;
        virtual void Update (double deltaTime) = 0;
        virtual void Render (double deltaTime) = 0;

    protected:
        MainLoop* m_MainLoop;
        std::string m_Name;
    };
}

#endif //   __STATE_H__
