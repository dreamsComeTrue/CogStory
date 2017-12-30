// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __NPC_ACTOR_H__
#define __NPC_ACTOR_H__

#include "Actor.h"

namespace aga
{
    class NPCActor : public Actor
    {
    public:
        NPCActor (SceneManager* sceneManager);

        static std::string GetTypeName ();
    };
}

#endif //   __NPC_ACTOR_H__
