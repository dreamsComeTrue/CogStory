// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.
#include "ActorFactory.h"
#include "actors/NPCActor.h"
#include "actors/TileActor.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    std::vector<std::string> ActorFactory::s_ActorTypes;

    //--------------------------------------------------------------------------------------------------

    ActorFactory::ActorFactory () {}

    //--------------------------------------------------------------------------------------------------

    void ActorFactory::RegisterActorTypes ()
    {
        s_ActorTypes.push_back (NPCActor::TypeName);
        s_ActorTypes.push_back (TileActor::TypeName);
    }

    //--------------------------------------------------------------------------------------------------

    std::vector<std::string>& ActorFactory::GetActorTypes () { return s_ActorTypes; }

    //--------------------------------------------------------------------------------------------------

    Actor* ActorFactory::GetActor (SceneManager* sceneManager, const std::string& type)
    {
        Actor* newActor = nullptr;

        if (type == NPCActor::TypeName)
        {
            newActor = new NPCActor (sceneManager);
        }
        else if (type == TileActor::TypeName)
        {
            newActor = new TileActor (sceneManager);
        }

        if (newActor)
        {
            newActor->Initialize ();
        }

        return newActor;
    }

    //--------------------------------------------------------------------------------------------------
}
