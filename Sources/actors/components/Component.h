// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __COMPONENT_H__
#define __COMPONENT_H__

#include "Lifecycle.h"
#include "Actor.h"

namespace aga
{
    class Component : public Lifecycle
    {
    public:
        Component (Actor* owner);

        virtual bool Update (float deltaTime) = 0;

        virtual bool Render (float deltaTime) = 0;

    protected:
        Actor* m_Actor;
    };
}

#endif //   __COMPONENT_H__

