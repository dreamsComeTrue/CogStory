// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __ACTOR_FACTORY_H__
#define __ACTOR_FACTORY_H__

#include "Actor.h"

namespace aga
{
    const std::string ANIMATION_PLAYER = "ANIMATION_PLAYER";

    class ActorFactory
    {
    public:
        ActorFactory ();

        static void RegisterActorTypes ();
        static void RegisterAnimations ();
        static std::vector<std::string>& GetActorTypes ();

        static Actor* GetActor (SceneManager* sceneManager, const std::string& type);
        static Animation& GetAnimation (const std::string& name);

    private:
        static Animation RegisterAnimation_Player ();

    protected:
        static std::vector<std::string> s_ActorTypes;
        static std::map<std::string, Animation> s_Animations;
    };
}

#endif //   __ACTOR_FACTORY_H__
