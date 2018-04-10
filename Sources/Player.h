// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "Actor.h"
#include "ParticleEmitter.h"

#define PLAYER_Z_ORDER 10

namespace aga
{
    class SceneManager;
    class Scene;

    class Player : public Actor
    {
    public:
        static std::string TypeName;

    public:
        Player (SceneManager* sceneManager);
        virtual ~Player ();
        bool Initialize () override;
        bool Destroy () override;

        void BeforeEnter () override;

        void SetPreventInput (bool prevent = false);
        bool IsPreventInput () const;
        void HandleInput (float deltaTime);
        void ProcessEvent (ALLEGRO_EVENT* event, float deltaTime);

        bool Update (float deltaTime) override;
        void Render (float deltaTime) override;

        void Move (float dx, float dy) override;
        void SetPosition (float x, float y) override;
        void SetPosition (Point pos) override;

        void SetFollowCamera (bool follow);

        virtual std::string GetTypeName () override;

        void ResetParticleEmitters ();

    private:
        void InitializeAnimations ();
        void CreateParticleEmitters ();
        void UpdateParticleEmitters ();

        void CollisionEvent (Collidable* other) override;

    private:
        bool m_FollowCamera;
        bool m_PreventInput;
        ParticleEmitter* m_HeadParticleEmitter;
        ParticleEmitter* m_WalkParticleEmitter;
    };
}

#endif //   __PLAYER_H__
