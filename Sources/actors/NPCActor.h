// Copyright 2017-2019 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __NPC_ACTOR_H__
#define __NPC_ACTOR_H__

#include "Actor.h"

namespace aga
{
    class NPCActor : public Actor
    {
    public:
        static std::string TypeName;

    public:
        NPCActor (SceneManager* sceneManager);
        NPCActor (const NPCActor& rhs);
        virtual NPCActor* Clone () const override;

        virtual bool Initialize () override;

        virtual bool Update (float deltaTime) override;

        virtual std::string GetTypeName () override { return TypeName; }
    };
}

#endif //   __NPC_ACTOR_H__
