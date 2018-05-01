// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __ENEMY_ACTOR_H__
#define __ENEMY_ACTOR_H__

#include "Actor.h"

namespace aga
{
    class EnemyActor : public Actor
    {
    public:
        static std::string TypeName;

    public:
        EnemyActor (SceneManager* sceneManager);

        virtual bool Initialize () override;

        virtual bool Update (float deltaTime);

        virtual std::string GetTypeName () override;

    private:
    };
}

#endif //   __ENEMY_ACTOR_H__
