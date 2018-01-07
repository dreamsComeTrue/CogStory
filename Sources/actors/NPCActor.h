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

        bool Initialize () override;

        virtual bool Update (float deltaTime);
        virtual void BeginOverlap (Entity* entity) override;
        virtual void EndOverlap (Entity* entity) override;

        virtual std::string GetTypeName () override;

    private:
        int m_Direction; // 0-4 => stop, up, right, down, left
        float m_StepsCounter;
    };
}

#endif //   __NPC_ACTOR_H__
