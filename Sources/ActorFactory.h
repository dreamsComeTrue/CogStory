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
        static void RegisterActorComponents ();
        static void RegisterAnimations ();
        static std::vector<std::string>& GetActorTypes () { return s_ActorTypes; }
        static std::vector<std::string>& GetActorComponents () { return s_ActorComponents; }

        static Actor* GetActor (SceneManager* sceneManager, const std::string& type);
        static class Component* GetActorComponent (Actor* actor, const std::string& type);
        static Animation& GetAnimation (const std::string& name);
        static Animation& GetDummyAnimation () { return s_DummyAnimation; }

        static std::map<std::string, Animation>& GetAnimations () { return s_Animations; }

    private:
        static Animation LoadAnimationFromFile (const std::string& path);

    protected:
        static std::vector<std::string> s_ActorTypes;
        static std::vector<std::string> s_ActorComponents;
        static std::map<std::string, Animation> s_Animations;
        static Animation s_DummyAnimation;
    };
}

#endif //   __ACTOR_FACTORY_H__
