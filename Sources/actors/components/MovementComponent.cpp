// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "MovementComponent.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    MovementComponent::MovementComponent (Actor* owner) :
        Component (owner)
    {
        
    }

    //--------------------------------------------------------------------------------------------------

    bool MovementComponent::Update (float deltaTime)
    {
        m_Actor->Move (4.f * deltaTime, 4.f * deltaTime);
        
        return true;
    }

    //--------------------------------------------------------------------------------------------------

    bool MovementComponent::Render (float deltaTime)
    {
        return true;
    }

    //--------------------------------------------------------------------------------------------------
}


