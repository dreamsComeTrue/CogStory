// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.
#include "Player.h"
#include "ActorFactory.h"
#include "AudioSample.h"
#include "MainLoop.h"
#include "PhysicsManager.h"
#include "Resources.h"
#include "Scene.h"
#include "SceneManager.h"
#include "Screen.h"
#include "actors/components/AudioSampleComponent.h"
#include "actors/components/ParticleEmitterComponent.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    std::string Player::TypeName = "Player";

    //--------------------------------------------------------------------------------------------------
    //--------------------------------------------------------------------------------------------------

    Player::Player (SceneManager* sceneManager)
        : Actor (sceneManager)
        , m_PreventInput (false)
        , m_ActionHandling (false)
        , m_ActionHandler (nullptr)
    {
    }

    //--------------------------------------------------------------------------------------------------

    Player::~Player ()
    {
        if (!IsDestroyed ())
        {
            Destroy ();
        }
    }

    //--------------------------------------------------------------------------------------------------

    bool Player::Initialize ()
    {
        Actor::Initialize ();

        CreateParticleEmitters ();
        UpdateParticleEmitters ();

        SetCheckOverlap (true);

        Animable::Initialize ("player", "");
        Bounds.SetSize ({64, 64});

        SetAnimation (ActorFactory::GetAnimation (ANIMATION_PLAYER));
        SetCurrentAnimation (ANIM_IDLE_NAME);

        m_FootStepComponent
            = (AudioSampleComponent*)ActorFactory::GetActorComponent (this, AudioSampleComponent::TypeName);
        m_FootStepComponent->LoadSampleFromFile (
            "FOOT_STEP", GetResource (SOUND_FOOT_STEP).Dir + GetResource (SOUND_FOOT_STEP).Name);
        m_FootStepComponent->GetAudioSample ()->SetVolume (2.0f);

        m_Components.insert (std::make_pair ("FOOT_STEP_COMPONENT", m_FootStepComponent));

        PhysPoints.clear ();
        PhysPoints.push_back ({
            {23, 35}, //  Left arm
            {25, 25}, //  Left head
            {37, 25}, //  Right head
            {39, 35}, //  Right arm
            {39, 64}, //  Right leg
            {23, 64} //  Left leg
        });

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    bool Player::Destroy () { return Actor::Destroy (); }

    //--------------------------------------------------------------------------------------------------

    void Player::BeforeEnter ()
    {
        SetPhysOffset (Bounds.GetPos ().X, Bounds.GetPos ().Y);

        m_HeadParticleComponent->GetEmitter ()->Reset ();
        m_FootParticleComponent->GetEmitter ()->Reset ();
    }

    //--------------------------------------------------------------------------------------------------

    void Player::CreateParticleEmitters ()
    {
        m_HeadParticleComponent
            = (ParticleEmitterComponent*)ActorFactory::GetActorComponent (this, ParticleEmitterComponent::TypeName);

        m_HeadParticleComponent->CreateEmitter ("particles", "smog_particles", 3, 2.0f);
        m_HeadParticleComponent->GetEmitter ()->SetColorTransition (COLOR_WHITE, al_map_rgba (255, 255, 255, 0));
        float xSpread = 0.12f;
        m_HeadParticleComponent->GetEmitter ()->SetVelocityVariance (Point (-xSpread, 0.3f), Point (xSpread, 0.3f));
        float partLife = 2.0f;
        m_HeadParticleComponent->GetEmitter ()->SetParticleLifeVariance (partLife, partLife);
        m_HeadParticleComponent->GetEmitter ()->Initialize ();

        m_Components.insert (std::make_pair ("HEAD_PARTICLE_COMPONENT", m_HeadParticleComponent));

        m_FootParticleComponent
            = (ParticleEmitterComponent*)ActorFactory::GetActorComponent (this, ParticleEmitterComponent::TypeName);

        m_FootParticleComponent->CreateEmitter ("particles", "dust_particles", 2, 0.7f);
        m_FootParticleComponent->GetEmitter ()->SetColorTransition (COLOR_WHITE, al_map_rgba (255, 255, 255, 0));
        float velVar = 0.4f;
        m_FootParticleComponent->GetEmitter ()->SetVelocityVariance (Point (-velVar, -velVar), Point (velVar, velVar));
        partLife = 0.4f;
        m_FootParticleComponent->GetEmitter ()->SetParticleLifeVariance (partLife, partLife);
        m_FootParticleComponent->GetEmitter ()->Initialize ();

        m_Components.insert (std::make_pair ("FOOT_PARTICLE_COMPONENT", m_FootParticleComponent));
    }

    //--------------------------------------------------------------------------------------------------

    void Player::UpdateParticleEmitters ()
    {
        m_HeadParticleComponent->GetEmitter ()->SetPosition (
            Bounds.Pos.X + Bounds.GetHalfSize ().Width + 5, Bounds.Pos.Y - 5);
        m_FootParticleComponent->GetEmitter ()->SetPosition (
            Bounds.Pos.X + Bounds.GetHalfSize ().Width, Bounds.Pos.Y + Bounds.GetSize ().Height);
    }

    //--------------------------------------------------------------------------------------------------

    bool Player::ProcessEvent (ALLEGRO_EVENT* event, float deltaTime)
    {
        if (m_PreventInput)
        {
            return false;
        }

        if (event->type == ALLEGRO_EVENT_KEY_DOWN)
        {
            if (event->keyboard.keycode == ALLEGRO_KEY_SPACE || event->keyboard.keycode == ALLEGRO_KEY_ENTER)
            {
                m_ActionHandling = true;

                if (m_ActionHandler)
                {
                    asIScriptContext* ctx = m_SceneManager->GetMainLoop ()->GetScriptManager ().GetContext ();
                    ctx->Prepare (m_ActionHandler);
                    ctx->Execute ();
                    ctx->Unprepare ();
                    ctx->GetEngine ()->ReturnContext (ctx);
                }

                return true;
            }
        }
        else if (event->type == ALLEGRO_EVENT_KEY_UP)
        {
            if (event->keyboard.keycode == ALLEGRO_KEY_SPACE || event->keyboard.keycode == ALLEGRO_KEY_ENTER)
            {
                m_ActionHandling = false;
            }

            if (m_Animation.GetCurrentAnimationName () == ANIM_MOVE_DOWN_NAME)
            {
                SetCurrentAnimation (ANIM_IDLE_NAME);
            }
            else if (m_Animation.GetCurrentAnimationName () == ANIM_MOVE_UP_NAME)
            {
                SetCurrentAnimation (ANIM_STAND_UP_NAME);
            }
            else if (m_Animation.GetCurrentAnimationName () == ANIM_MOVE_LEFT_NAME)
            {
                SetCurrentAnimation (ANIM_STAND_LEFT_NAME);
            }
            else if (m_Animation.GetCurrentAnimationName () == ANIM_MOVE_RIGHT_NAME)
            {
                SetCurrentAnimation (ANIM_STAND_RIGHT_NAME);
            }

            return true;
        }

        return false;
    }

    //--------------------------------------------------------------------------------------------------

    void Player::HandleInput (float deltaTime)
    {
        if (m_PreventInput)
        {
            return;
        }

        ALLEGRO_KEYBOARD_STATE state;
        al_get_keyboard_state (&state);
        float dx = 0, dy = 0;

        //  Prevent diagonal moveing from left/right move animation resetting
        bool isMoveRight = al_key_down (&state, ALLEGRO_KEY_RIGHT) || al_key_down (&state, ALLEGRO_KEY_D);
        bool isMoveLeft = al_key_down (&state, ALLEGRO_KEY_LEFT) || al_key_down (&state, ALLEGRO_KEY_A);

        if (al_key_down (&state, ALLEGRO_KEY_DOWN) || al_key_down (&state, ALLEGRO_KEY_S))
        {
            dy = MOVE_SPEED * deltaTime;

            if (!isMoveRight && !isMoveLeft)
            {
                SetCurrentAnimation (ANIM_MOVE_DOWN_NAME);
            }
        }

        if (al_key_down (&state, ALLEGRO_KEY_UP) || al_key_down (&state, ALLEGRO_KEY_W))
        {
            dy = -MOVE_SPEED * deltaTime;

            if (!isMoveRight && !isMoveLeft)
            {
                SetCurrentAnimation (ANIM_MOVE_UP_NAME);
            }
        }

        if (isMoveRight)
        {
            dx = MOVE_SPEED * deltaTime;
            SetCurrentAnimation (ANIM_MOVE_RIGHT_NAME);
        }

        if (isMoveLeft)
        {
            dx = -MOVE_SPEED * deltaTime;
            SetCurrentAnimation (ANIM_MOVE_LEFT_NAME);
        }

        ProcessTriggerAreas (dx, dy);

        if (al_key_down (&state, ALLEGRO_KEY_LSHIFT))
        {
            float multiplier = 3;
            dx *= multiplier;
            dy *= multiplier;
        }

        std::vector<Entity*> entites = m_SceneManager->GetActiveScene ()->RecomputeVisibleEntities (true);
        for (Entity* ent : entites)
        {
            Collidable* collidable = (Actor*)ent;
            Point collisionDelta;

            if (IsCollidingWith (collidable, Point (dx, dy), std::move (collisionDelta)))
            {
                float positiveMoveBoundary = 1.5f;
                float negativeMoveBoundary = -0.5f;

                if (collisionDelta.X < negativeMoveBoundary || collisionDelta.X > positiveMoveBoundary)
                {
                    dx = dx + collisionDelta.X;
                }

                if (collisionDelta.Y < negativeMoveBoundary || collisionDelta.Y > positiveMoveBoundary)
                {
                    dy = dy + collisionDelta.Y;
                }
            }
        }

        if (!((AreSame (dx, 0) && AreSame (dy, 0))))
        {
            Move (dx, dy);
        }
    }

    //--------------------------------------------------------------------------------------------------

    bool Player::Update (float deltaTime)
    {
        Actor::Update (deltaTime);

        //  We can only emit where new position is different than last one
        m_FootParticleComponent->GetEmitter ()->SetCanEmit (!AreSame (m_OldPosition, Bounds.Pos));
        m_HeadParticleComponent->Update (deltaTime);
        m_FootParticleComponent->Update (deltaTime);

        m_OldPosition = Bounds.GetPos ();

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    void Player::Render (float deltaTime)
    {
        m_HeadParticleComponent->Render (deltaTime);
        m_FootParticleComponent->Render (deltaTime);

        return Actor::Render (deltaTime);
    }

    //--------------------------------------------------------------------------------------------------

    void Player::Move (float dx, float dy)
    {
        Actor::Move (dx, dy);

        static float sampleCounter = 0.0f;

        sampleCounter += m_SceneManager->GetMainLoop ()->GetScreen ()->GetDeltaTime ();

        if (sampleCounter > 0.28f)
        {
            m_FootStepComponent->GetAudioSample ()->Play ();

            sampleCounter = 0;
        }

        UpdateParticleEmitters ();
    }

    //--------------------------------------------------------------------------------------------------

    void Player::SetPosition (float x, float y)
    {
        Actor::SetPosition (x, y);

        UpdateParticleEmitters ();
        SetCurrentAnimation (ANIM_IDLE_NAME);
    }

    //--------------------------------------------------------------------------------------------------

    void Player::CollisionEvent (Collidable* other) {}

    //--------------------------------------------------------------------------------------------------
}
