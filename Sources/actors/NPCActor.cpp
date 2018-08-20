// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "NPCActor.h"
#include "ActorFactory.h"
#include "Player.h"
#include "SceneManager.h"
#include "components/MovementComponent.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    std::string NPCActor::TypeName = "NPC";

    //--------------------------------------------------------------------------------------------------
    //--------------------------------------------------------------------------------------------------

    NPCActor::NPCActor (SceneManager* sceneManager)
        : Actor (sceneManager)
        , m_MovementComponent (new MovementComponent (this))
    {
        m_Components.insert (std::make_pair ("MOVEMENT_COMPONENT", m_MovementComponent));
    }

    //--------------------------------------------------------------------------------------------------

    bool NPCActor::Initialize ()
    {
        Actor::Initialize ();

        SetCheckOverlap (true);

        SetAnimation (ActorFactory::GetAnimation (ANIMATION_NPC_1));
        SetCurrentAnimation (ANIM_STAND_LEFT_NAME);

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    bool NPCActor::Update (float deltaTime)
    {
        Actor::Update (deltaTime);

        m_MovementComponent->Update (deltaTime);

        return true;
    }

    void NPCActor::SetPosition (float x, float y)
    {
        Actor::SetPosition (x, y);

        m_MovementComponent->SetStartPos ({x, y});
    }

    //--------------------------------------------------------------------------------------------------

    void NPCActor::BeginOverlap (Entity* entity) {}

    //--------------------------------------------------------------------------------------------------

    void NPCActor::CollisionEvent (Collidable* other) {}

    //--------------------------------------------------------------------------------------------------
}
