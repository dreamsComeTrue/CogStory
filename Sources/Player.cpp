// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.
#include "Player.h"
#include "ActorFactory.h"
#include "AudioSample.h"
#include "MainLoop.h"
#include "ParticleEmitter.h"
#include "Resources.h"
#include "Scene.h"
#include "Screen.h"
#include "SpeechFrameManager.h"
#include "actors/components/AudioSampleComponent.h"
#include "actors/components/ParticleEmitterComponent.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    std::string Player::TypeName = "Player";
    static int ACTION_ID = 0;

    //  Pixels Per Second
    const float MOVE_SPEED = 130.0;

    //--------------------------------------------------------------------------------------------------
    //--------------------------------------------------------------------------------------------------

    Player::Player (SceneManager* sceneManager)
        : Actor (sceneManager)
        , m_PreventInput (false)
        , m_ActionHandling (false)
        , m_ActionHandler (nullptr)
        , m_LastActionActor (nullptr)
    {
        SetCollidable (true);
        SetCollisionEnabled (true);
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

        SetAnimation (ActorFactory::GetAnimation ("PLAYER"));
        SetCurrentAnimation (ANIM_IDLE_NAME);

        m_FootStepComponent = static_cast<AudioSampleComponent*> (
            ActorFactory::GetActorComponent (this, AudioSampleComponent::TypeName));
        m_FootStepComponent->LoadSampleFromFile (
            "FOOT_STEP", GetResource (SOUND_FOOT_STEP).Dir + GetResource (SOUND_FOOT_STEP).Name);
        m_FootStepComponent->GetAudioSample ()->SetVolume (2.0f);

        m_Components.insert (std::make_pair ("FOOT_STEP_COMPONENT", m_FootStepComponent));

        PhysPoints.clear ();
        PhysPoints.push_back ({
            {23.f, 35.f}, //  Left arm
            {25.f, 25.f}, //  Left head
            {37.f, 25.f}, //  Right head
            {39.f, 35.f}, //  Right arm
            {39.f, 64.f}, //  Right leg
            {23.f, 64.f} //  Left leg
        });

        m_SceneManager->GetSpeechFrameManager ()->RegisterSpeechesFinishedHandler ([&]() {
            if (m_LastActionActor)
            {
                m_LastActionActor->ResumeUpdate ();
            }
        });

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    bool Player::Destroy () { return Actor::Destroy (); }

    //--------------------------------------------------------------------------------------------------

    void Player::BeforeEnter ()
    {
        SetPhysOffset (Bounds.GetPos ());

        m_HeadParticleComponent->GetEmitter ()->Reset ();
        m_FootParticleComponent->GetEmitter ()->Reset ();

        m_OverlapCallbacks.clear ();
    }

    //--------------------------------------------------------------------------------------------------

    void Player::CreateParticleEmitters ()
    {
        m_HeadParticleComponent = static_cast<ParticleEmitterComponent*> (
            ActorFactory::GetActorComponent (this, ParticleEmitterComponent::TypeName));

        m_HeadParticleComponent->CreateEmitter ("particles", "smog_particles", 3, 2.0f);
        m_HeadParticleComponent->GetEmitter ()->SetColorTransition (COLOR_WHITE, al_map_rgba (255, 255, 255, 0));
        float xSpread = 0.12f;
        m_HeadParticleComponent->GetEmitter ()->SetVelocityVariance (Point (-xSpread, 0.3f), Point (xSpread, 0.3f));
        float partLife = 2.0f;
        m_HeadParticleComponent->GetEmitter ()->SetParticleLifeVariance (partLife, partLife);
        m_HeadParticleComponent->GetEmitter ()->Initialize ();

        m_Components.insert (std::make_pair ("HEAD_PARTICLE_COMPONENT", m_HeadParticleComponent));

        m_FootParticleComponent = static_cast<ParticleEmitterComponent*> (
            ActorFactory::GetActorComponent (this, ParticleEmitterComponent::TypeName));

        m_FootParticleComponent->CreateEmitter ("particles", "dust_particles", 2, 1);
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

    bool Player::ProcessEvent (ALLEGRO_EVENT* event, float)
    {
        if (m_PreventInput)
        {
            return false;
        }

        if (event->type == ALLEGRO_EVENT_KEY_DOWN)
        {
            if (event->keyboard.keycode == ALLEGRO_KEY_SPACE || event->keyboard.keycode == ALLEGRO_KEY_ENTER)
            {
                HandleAction ();

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

    bool Player::IsAction () { return m_ActionHandling; }

    //--------------------------------------------------------------------------------------------------

    void Player::SetPreventInput (bool prevent) { m_PreventInput = prevent; }

    //--------------------------------------------------------------------------------------------------

    bool Player::IsPreventInput () const { return m_PreventInput; }

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

        auto updatePos = [&](Point point) {
            float positiveMoveBoundary = 1.5f;
            float negativeMoveBoundary = -0.5f;

            if (point.X < negativeMoveBoundary || point.X > positiveMoveBoundary)
            {
                dx = dx + point.X;
            }

            if (point.Y < negativeMoveBoundary || point.Y > positiveMoveBoundary)
            {
                dy = dy + point.Y;
            }
        };

        Point triggerAreaDelta;

        ProcessTriggerAreas (dx, dy, std::move (triggerAreaDelta));
        updatePos (triggerAreaDelta);

        if (al_key_down (&state, ALLEGRO_KEY_LSHIFT))
        {
            float multiplier = 3;
            dx *= multiplier;
            dy *= multiplier;
        }

        std::vector<Entity*> entites = m_SceneManager->GetActiveScene ()->RecomputeVisibleEntities (true);
        for (Entity* ent : entites)
        {
            Collidable* collidable = static_cast<Actor*> (ent);
            Point collisionDelta;

            if (IsCollidingWith (collidable, Point (dx, dy), std::move (collisionDelta)))
            {
                updatePos (collisionDelta);
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

        m_OldPosition = Bounds.GetPos ();

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    void Player::Render (float deltaTime) { return Actor::Render (deltaTime); }

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

    void Player::SetPosition (Point point) { Player::SetPosition (point.X, point.Y); }

    //--------------------------------------------------------------------------------------------------

    std::string Player::GetTypeName () { return TypeName; }

    //--------------------------------------------------------------------------------------------------

    void Player::SetCurrentAnimation (const std::string& name) { Animable::SetCurrentAnimation (name); }

    //--------------------------------------------------------------------------------------------------

    void Player::SetActionHandler (asIScriptFunction* func) { m_ActionHandler = func; }

    //--------------------------------------------------------------------------------------------------

    void Player::HandleAction ()
    {
        //  In case we're still moving...
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

        m_ActionHandling = true;

        if (m_ActionHandler)
        {
            asIScriptContext* ctx = m_SceneManager->GetMainLoop ()->GetScriptManager ().GetContext ();
            ctx->Prepare (m_ActionHandler);
            ctx->Execute ();
            ctx->Unprepare ();
            ctx->GetEngine ()->ReturnContext (ctx);
        }

        m_LastActionActor = nullptr;

        for (std::map<std::string, ActorAction>::iterator it = m_ActorActions.begin (); it != m_ActorActions.end ();
             ++it)
        {
            ActorAction& action = it->second;

            if (IsOverlaping (action.AnActor))
            {
                if (action.Func)
                {
                    asIScriptContext* ctx = m_SceneManager->GetMainLoop ()->GetScriptManager ().GetContext ();
                    ctx->Prepare (action.Func);
                    ctx->SetArgObject (0, action.AnActor);
                    ctx->Execute ();
                    ctx->Unprepare ();
                    ctx->GetEngine ()->ReturnContext (ctx);
                }

                if (action.SpeechID != "")
                {
                    action.AnActor->OrientTo (this);
                    action.AnActor->SuspendUpdate ();

                    m_SceneManager->GetSpeechFrameManager ()->AddSpeechFrame (action.SpeechID, true);
                }

                action.Handled = true;

                m_LastActionActor = action.AnActor;
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    Actor* Player::RegisterActorAction (
        const std::string& actionName, const std::string& actorName, asIScriptFunction* func)
    {
        if (m_ActorActions.find (actionName) == m_ActorActions.end ())
        {
            ActorAction action;
            action.AnActor = m_SceneManager->GetActor (actorName);
            action.Func = func;

            m_ActorActions.insert (std::make_pair (actionName, action));
        }

        return m_ActorActions[actionName].AnActor;
    }

    //--------------------------------------------------------------------------------------------------

    void Player::RegisterActionSpeech (const std::string& actorName, const std::string& speechID)
    {
        ActorAction action;
        action.AnActor = m_SceneManager->GetActor (actorName);
        action.SpeechID = speechID;

        m_ActorActions.insert (std::make_pair ("ACTION_" + std::to_string (ACTION_ID++), action));
    }

    //--------------------------------------------------------------------------------------------------

    SpeechFrame* Player::TalkTo (Actor* actor, const std::string& speechID)
    {
        actor->OrientTo (this);
        actor->SuspendUpdate ();

        SpeechFrame* frame = m_SceneManager->GetSpeechFrameManager ()->AddSpeechFrame (speechID, true);

        return frame;
    }

    //--------------------------------------------------------------------------------------------------

    Actor* Player::GetLastActionActor () { return m_LastActionActor; }

    //--------------------------------------------------------------------------------------------------
}
