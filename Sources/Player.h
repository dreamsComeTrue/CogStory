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

    class AudioSampleComponent;
    class ParticleEmitterComponent;

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

        void SetPreventInput (bool prevent = false) { m_PreventInput = prevent; }
        bool IsPreventInput () const { return m_PreventInput; }
        void HandleInput (float deltaTime);
        void ProcessEvent (ALLEGRO_EVENT* event, float deltaTime);

        bool Update (float deltaTime) override;
        void Render (float deltaTime) override;

        virtual void Move (float dx, float dy) override;
        virtual void SetPosition (float x, float y) override;
        virtual void SetPosition (Point point) override { Player::SetPosition (point.X, point.Y); }

        virtual std::string GetTypeName () override { return TypeName; }

        //  Override for ScriptManager
        void SetCurrentAnimation (const std::string& name) { Animable::SetCurrentAnimation (name); }

    private:
        void CreateParticleEmitters ();
        void UpdateParticleEmitters ();

        void CollisionEvent (Collidable* other) override;

    private:
        bool m_PreventInput;

        AudioSampleComponent* m_FootStepComponent;
        ParticleEmitterComponent* m_HeadParticleComponent;
        ParticleEmitterComponent* m_FootParticleComponent;
    };
}

#endif //   __PLAYER_H__
