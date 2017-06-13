// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __STATE_H__
#define __STATE_H__

#include "Lifecycle.h"

#include <string>

union ALLEGRO_EVENT;

namespace aga
{
    class State : public Lifecycle
    {
    public:
        State (const std::string& name)
            : m_Name (name)
        {
        }

        virtual void ProcessEvent (ALLEGRO_EVENT* event) = 0;
        virtual void Update () = 0;
        virtual void Render () = 0;

    private:
        std::string m_Name;
    };
}

#endif //   __STATE_H__
