// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "NPCActor.h"
#include "ActorFactory.h"
#include "Player.h"
#include "SceneManager.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    std::string NPCActor::TypeName = "NPC";

    //--------------------------------------------------------------------------------------------------
    //--------------------------------------------------------------------------------------------------

    NPCActor::NPCActor (SceneManager* sceneManager)
        : Actor (sceneManager)
    {
    }

    //--------------------------------------------------------------------------------------------------

    bool NPCActor::Initialize ()
    {
        Actor::Initialize ();

        SetAnimation (ActorFactory::GetAnimation ("NPC_1"));
        SetCurrentAnimation (ANIM_STAND_LEFT_NAME);

        return true;
    }

    bool NPCActor::Update (float deltaTime) { return Actor::Update (deltaTime); }

    //--------------------------------------------------------------------------------------------------
}
