// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "EnemyActor.h"
#include "MainLoop.h"
#include "SceneManager.h"

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
        Animable::Initialize ("menu_ui", "cog");

        Bounds.SetSize ({ 64, 64 });
        SetCheckOverlap (true);

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    bool EnemyActor::Update (float deltaTime)
    {
        Actor::Update (deltaTime);

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    std::string EnemyActor::GetTypeName () { return TypeName; }

    //--------------------------------------------------------------------------------------------------
}
