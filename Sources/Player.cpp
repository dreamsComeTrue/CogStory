// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.
#include "Player.h"
#include "AudioSample.h"
#include "MainLoop.h"
#include "PhysicsManager.h"
#include "Scene.h"
#include "SceneManager.h"
#include "Screen.h"
#include "actors/NPCActor.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    std::string Player::TypeName = "Player";

    //--------------------------------------------------------------------------------------------------
    //--------------------------------------------------------------------------------------------------

    Player::Player (SceneManager* sceneManager)
        : Actor (sceneManager)
        , m_WalkParticleEmitter (2, 0.7f, GetResourcePath (GFX_DUST_PARTICLES))
        , m_HeadParticleEmitter (3, 2.0f, GetResourcePath (GFX_SMOG_PARTICLES))
        , m_FollowCamera (true)
        , m_PreventInput (false)
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

    AudioSample* sample;

    bool Player::Initialize ()
    {
        Actor::Initialize ();

        UpdateParticleEmitters ();

        m_WalkParticleEmitter.SetColorTransition (COLOR_WHITE, al_map_rgba (255, 255, 255, 0));
        float velVar = 0.4f;
        m_WalkParticleEmitter.SetVelocityVariance (Point (-velVar, -velVar), Point (velVar, velVar));
        float partLife = 0.4f;
        m_WalkParticleEmitter.SetParticleLifeVariance (partLife, partLife);
        m_WalkParticleEmitter.Initialize ();

        m_HeadParticleEmitter.SetColorTransition (COLOR_WHITE, al_map_rgba (255, 255, 255, 0));
        float xSpread = 0.12f;
        m_HeadParticleEmitter.SetVelocityVariance (Point (-xSpread, 0.4f), Point (xSpread, 0.4f));
        partLife = 1.0f;
        m_HeadParticleEmitter.SetParticleLifeVariance (partLife, partLife);
        m_HeadParticleEmitter.Initialize ();

        m_Image = al_load_bitmap (GetResourcePath (ResourceID::GFX_PLAYER).c_str ());
        Bounds.SetSize ({ 64, 64 });

        InitializeAnimations ();

        sample = m_SceneManager->GetMainLoop ()->GetAudioManager ().LoadSampleFromFile (
            "FOOT_STEP", GetResourcePath (SOUND_FOOT_STEP));
        sample->SetVolume (1.0f);

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    bool Player::Destroy ()
    {
        m_WalkParticleEmitter.Destroy ();
        m_HeadParticleEmitter.Destroy ();

        return Actor::Destroy ();
    }

    //--------------------------------------------------------------------------------------------------

    void Player::BeforeEnter ()
    {
        PhysPoints.clear ();
        PhysPoints.push_back ({ { 20, 30 }, { 25, 20 }, { 39, 20 }, { 44, 30 }, { 44, 64 }, { 20, 64 } });
        SetPhysOffset (Bounds.GetPos ().X - Bounds.GetHalfSize ().Width,
                       Bounds.GetPos ().Y - Bounds.GetHalfSize ().Height);
    }

    //--------------------------------------------------------------------------------------------------

    void Player::InitializeAnimations ()
    {
        Point cellSize (64, 64);
        float animSpeed = 80;

        AnimationFrames idleFrames (3, cellSize);
        idleFrames.AddFrame (0, 0, 0);
        idleFrames.AddFrame (1, 0, 1);
        idleFrames.AddFrame (2, 0, 2);
        idleFrames.SetPlaySpeed (500);
        m_Animation.AddAnimationFrames (ANIM_IDLE_NAME, idleFrames);

        AnimationFrames moveDownFrames (6, cellSize);
        moveDownFrames.AddFrame (0, 1, 0);
        moveDownFrames.AddFrame (1, 1, 1);
        moveDownFrames.AddFrame (2, 1, 2);
        moveDownFrames.AddFrame (3, 1, 3);
        moveDownFrames.AddFrame (4, 1, 4);
        moveDownFrames.AddFrame (5, 1, 5);
        moveDownFrames.SetPlaySpeed (animSpeed);
        m_Animation.AddAnimationFrames (ANIM_MOVE_DOWN_NAME, moveDownFrames);

        AnimationFrames moveLeftFrames (6, cellSize);
        moveLeftFrames.AddFrame (0, 2, 0);
        moveLeftFrames.AddFrame (1, 2, 1);
        moveLeftFrames.AddFrame (2, 2, 2);
        moveLeftFrames.AddFrame (3, 2, 3);
        moveLeftFrames.AddFrame (4, 2, 4);
        moveLeftFrames.AddFrame (5, 2, 5);
        moveLeftFrames.SetPlaySpeed (animSpeed + 30);
        m_Animation.AddAnimationFrames (ANIM_MOVE_LEFT_NAME, moveLeftFrames);

        AnimationFrames moveRightFrames (6, cellSize);
        moveRightFrames.AddFrame (0, 3, 0);
        moveRightFrames.AddFrame (1, 3, 1);
        moveRightFrames.AddFrame (2, 3, 2);
        moveRightFrames.AddFrame (3, 3, 3);
        moveRightFrames.AddFrame (4, 3, 4);
        moveRightFrames.AddFrame (5, 3, 5);
        moveRightFrames.SetPlaySpeed (animSpeed + 30);
        m_Animation.AddAnimationFrames (ANIM_MOVE_RIGHT_NAME, moveRightFrames);

        AnimationFrames moveUpFrames (6, cellSize);
        moveUpFrames.AddFrame (0, 4, 0);
        moveUpFrames.AddFrame (1, 4, 1);
        moveUpFrames.AddFrame (2, 4, 2);
        moveUpFrames.AddFrame (3, 4, 3);
        moveUpFrames.AddFrame (4, 4, 4);
        moveUpFrames.AddFrame (5, 4, 5);
        moveUpFrames.SetPlaySpeed (animSpeed);
        m_Animation.AddAnimationFrames (ANIM_MOVE_UP_NAME, moveUpFrames);

        SetCurrentAnimation (ANIM_IDLE_NAME);
    }

    //--------------------------------------------------------------------------------------------------

    void Player::UpdateParticleEmitters ()
    {
        m_WalkParticleEmitter.SetPosition (Bounds.Pos.X, Bounds.Pos.Y + Bounds.GetHalfSize ().Height);
        m_HeadParticleEmitter.SetPosition (Bounds.Pos.X + 5, Bounds.Pos.Y - Bounds.GetHalfSize ().Height - 5);
    }

    //--------------------------------------------------------------------------------------------------

    void Player::ProcessEvent (ALLEGRO_EVENT* event, float deltaTime)
    {
        if (event->type == ALLEGRO_EVENT_KEY_UP)
        {
            SetCurrentAnimation (ANIM_IDLE_NAME);
        }
    }

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

        ProcessTriggerAreas (dx, dy);

        std::vector<Entity*> entites = m_SceneManager->GetActiveScene ()->GetVisibleEntities ();
        for (Entity* ent : entites)
        {
            Collidable* collidable = (Actor*)ent;
            Point collisionDelta;

            if (IsCollidingWith (collidable, Point (dx, dy), std::move (collisionDelta)))
            {
                dx = dx + collisionDelta.X;
                dy = dy + collisionDelta.Y;
            }
        }

        if (!((AreSame (dx, 0) && AreSame (dy, 0))))
        {
            if (al_key_down (&state, ALLEGRO_KEY_LSHIFT))
            {
                float multiplier = 3;
                Move (dx * multiplier, dy * multiplier);
            }
            else
            {
                Move (dx, dy);
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    bool Player::Update (float deltaTime)
    {
        Actor::Update (deltaTime);

        //  We can only emit where new position is different than last one
        m_WalkParticleEmitter.SetCanEmit (!AreSame (m_OldPosition, Bounds.Pos));
        m_WalkParticleEmitter.Update (deltaTime);

        m_HeadParticleEmitter.Update (deltaTime);

        m_OldPosition = Bounds.GetPos ();

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    void Player::Render (float deltaTime)
    {
        m_WalkParticleEmitter.Render (deltaTime);
        m_HeadParticleEmitter.Render (deltaTime);

        return Actor::Render (deltaTime);
    }

    //--------------------------------------------------------------------------------------------------

    float sampleCounter = 0;

    void Player::Move (float dx, float dy)
    {
        float playerSpeed = 0.8f;

        dx *= playerSpeed;
        dy *= playerSpeed;

        sampleCounter += m_SceneManager->GetMainLoop ()->GetScreen ()->GetDeltaTime ();

        if (sampleCounter > 0.3f)
        {
            sample->Play ();
            sampleCounter = 0;
        }

        Bounds.SetPos (Bounds.GetPos () + Point (dx, dy));

        if (!(std::abs (dx) < 0.1 && std::abs (dy) < 0.1f))
        {
            ChooseAnimation (ToPositiveAngle (RadiansToDegrees (std::atan2 (dy, dx))));
        }

        if (MoveCallback != nullptr)
        {
            MoveCallback (dx, dy);
        }

        AddPhysOffset ({ dx, dy });

        UpdateParticleEmitters ();
    }

    //--------------------------------------------------------------------------------------------------

    void Player::SetPosition (float x, float y)
    {
        Bounds.SetPos ({ x, y });

        if (MoveCallback != nullptr && m_FollowCamera)
        {
            MoveCallback (Bounds.GetPos ().X - m_OldPosition.X, Bounds.GetPos ().Y - m_OldPosition.Y);
        }

        SetPhysOffset (Bounds.GetPos ().X - Bounds.GetHalfSize ().Width,
                       Bounds.GetPos ().Y - Bounds.GetHalfSize ().Height);

        UpdateParticleEmitters ();
    }

    //--------------------------------------------------------------------------------------------------

    void Player::SetPosition (Point pos) { Actor::SetPosition (pos); }

    //--------------------------------------------------------------------------------------------------

    void Player::SetFollowCamera (bool follow) { m_FollowCamera = follow; }

    //--------------------------------------------------------------------------------------------------

    std::string Player::GetTypeName () { return TypeName; }

    //--------------------------------------------------------------------------------------------------

    void Player::CollisionEvent (Collidable* other)
    {
        Entity* entity = dynamic_cast<Entity*> (other);

        if (entity->GetTypeName () == NPCActor::TypeName)
        {
            m_SceneManager->GetSpeechFrameManager ().AddSpeechFrame ("GREET_1", true);
        }
    }

    //--------------------------------------------------------------------------------------------------
}
