// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __MOVEMENT_COMPONENT_H__
#define __MOVEMENT_COMPONENT_H__

#include "Component.h"

namespace aga
{
    class MovementComponent : public Component
    {
    public:
        MovementComponent (Actor* owner);

        virtual bool Update (float deltaTime) override;

        virtual bool Render (float deltaTime) override;
    private:
    };
}

#endif //   __MOVEMENT_COMPONENT_H__

