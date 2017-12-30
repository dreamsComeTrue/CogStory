// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __ACTOR_FACTORY_H__
#define __ACTOR_FACTORY_H__

#include "Actor.h"

namespace aga
{
    class ActorFactory
    {
    public:
        ActorFactory ();

        static void RegisterActorTypes ();
        static std::vector<std::string>& GetActorTypes ();

        static Actor* GetActor (SceneManager* sceneManager, const std::string& type);

    protected:
        static std::vector<std::string> s_ActorTypes;
    };
}

#endif //   __ACTOR_FACTORY_H__
