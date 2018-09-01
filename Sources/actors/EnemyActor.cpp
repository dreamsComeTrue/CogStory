// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "EnemyActor.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    std::string EnemyActor::TypeName = "Enemy";

    //--------------------------------------------------------------------------------------------------
    //--------------------------------------------------------------------------------------------------

    EnemyActor::EnemyActor (SceneManager* sceneManager)
        : Actor (sceneManager)
    {
    }

    //--------------------------------------------------------------------------------------------------

    bool EnemyActor::Initialize ()
    {
        Actor::Initialize ();

        SetCheckOverlap (true);

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    bool EnemyActor::Update (float deltaTime) { return Actor::Update (deltaTime); }

    //--------------------------------------------------------------------------------------------------
}
