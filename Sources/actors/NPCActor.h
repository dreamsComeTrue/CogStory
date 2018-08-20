// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

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

        virtual bool Initialize () override;

        virtual bool Update (float deltaTime);

        virtual std::string GetTypeName () override { return TypeName; }

        virtual void SetPosition (float x, float y) override;

    private:
        virtual void BeginOverlap (Entity* entity) override;
        virtual void CollisionEvent (Collidable* other) override;

    private:
        class MovementComponent* m_MovementComponent;
    };
}

#endif //   __NPC_ACTOR_H__
