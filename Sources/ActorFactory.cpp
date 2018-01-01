// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.
#include "ActorFactory.h"
#include "actors/NPCActor.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    std::vector<std::string> ActorFactory::s_ActorTypes;

    //--------------------------------------------------------------------------------------------------

    ActorFactory::ActorFactory () {}

    //--------------------------------------------------------------------------------------------------

    void ActorFactory::RegisterActorTypes () { s_ActorTypes.push_back (NPCActor::GetTypeName ()); }

    //--------------------------------------------------------------------------------------------------

    std::vector<std::string>& ActorFactory::GetActorTypes () { return s_ActorTypes; }

    //--------------------------------------------------------------------------------------------------

    Actor* ActorFactory::GetActor (SceneManager* sceneManager, const std::string& type)
    {
        Actor* newActor = nullptr;

        if (type == NPCActor::GetTypeName ())
        {
            newActor = new NPCActor (sceneManager);
        }

        if (newActor)
        {
            newActor->Initialize ();
        }

        return newActor;
    }

    //--------------------------------------------------------------------------------------------------
}
