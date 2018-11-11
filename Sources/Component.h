// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __COMPONENT_H__
#define __COMPONENT_H__

#include "Actor.h"
#include "Lifecycle.h"

namespace aga
{
    class Component : public Lifecycle
    {
    public:
        Component (Actor* owner);
        virtual ~Component () {}

        virtual bool Update (float deltaTime) = 0;

        virtual bool Render (float deltaTime) = 0;

        void SetEnabled (bool enabled) { m_IsEnabled = enabled; }
        bool IsEnabled () const { return m_IsEnabled; }

        virtual std::string GetTypeName () = 0;

    protected:
        Actor* m_Actor;
        bool m_IsEnabled;
    };
}

#endif //   __COMPONENT_H__
