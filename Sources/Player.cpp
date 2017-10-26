// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.
#include "Player.h"
#include "MainLoop.h"
#include "PhysicsManager.h"
#include "Scene.h"
#include "SceneManager.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    //  Pixels Per Second
    const double MOVE_SPEED = 90.0;
    const std::string ANIM_IDLE = "ANIM_IDLE";
    const std::string ANIM_MOVE_UP = "ANIM_MOVE_UP";
    const std::string ANIM_MOVE_LEFT = "ANIM_MOVE_LEFT";
    const std::string ANIM_MOVE_RIGHT = "ANIM_MOVE_RIGHT";

    //--------------------------------------------------------------------------------------------------

    Player::Player (SceneManager* sceneManager)
      : m_SceneManager (sceneManager)
      , m_Image (nullptr)
    {
    }

    //--------------------------------------------------------------------------------------------------

    Player::~Player ()
    {
        if (!IsDestroyed ())
        {
            Destroy ();
        }

        Lifecycle::Destroy ();
    }

    //--------------------------------------------------------------------------------------------------

    bool Player::Initialize ()
    {
        m_Image = al_load_bitmap (GetResourcePath (ResourceID::GFX_PLAYER).c_str ());
        m_Size = { 64, 64 };

        InitializeAnimations ();

        Lifecycle::Initialize ();
    }

    //--------------------------------------------------------------------------------------------------

    bool Player::Destroy ()
    {
        if (m_Image != nullptr)
        {
            al_destroy_bitmap (m_Image);
            m_Image = nullptr;
        }

        Lifecycle::Destroy ();
    }

    //--------------------------------------------------------------------------------------------------

    void Player::CreatePhysics (Scene* currentScene)
    {
        PhysPoints = { { 10, 0 }, { 50, 0 }, { 50, 70 }, { 10, 70 } };
        SetPhysOffset (m_Position);
    }

    //--------------------------------------------------------------------------------------------------

    void Player::DestroyPhysics (Scene* currentScene) {}

    //--------------------------------------------------------------------------------------------------

    bool Player::Update (float deltaTime)
    {
        m_Animation.Update (deltaTime);
        UpdateScripts (deltaTime);

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    void Player::ProcessEvent (ALLEGRO_EVENT* event, float deltaTime)
    {
        if (event->type == ALLEGRO_EVENT_KEY_UP)
        {
            m_Animation.SetCurrentAnimation (ANIM_IDLE);
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Player::Render (float deltaTime)
    {
        AnimationFrames& frames = m_Animation.GetCurrentAnimation ();
        const Rect& frame = frames.GetFrame (m_Animation.GetCurrentFrame ());
        int width = frame.BottomRight.Width;
        int height = frame.BottomRight.Height;

        al_draw_scaled_bitmap (m_Image, frame.TopLeft.X, frame.TopLeft.Y, width, height, m_Position.X, m_Position.Y, width, height, 0);
    }

    //--------------------------------------------------------------------------------------------------

    void Player::InitializeAnimations ()
    {
        AnimationFrames idleFrames (3);
        idleFrames.AddFrame (0, Rect{ Point (0, 0), Point (64, 64) });
        idleFrames.AddFrame (1, Rect{ Point (64, 0), Point (64, 64) });
        idleFrames.AddFrame (2, Rect{ Point (128, 0), Point (64, 64) });
        idleFrames.SetPlaySpeed (500);
        m_Animation.AddAnimationFrames (ANIM_IDLE, idleFrames);

        AnimationFrames moveLeftFrames (1);
        moveLeftFrames.AddFrame (0, Rect{ Point (0, 64), Point (64, 64) });
        moveLeftFrames.SetPlaySpeed (500);
        m_Animation.AddAnimationFrames (ANIM_MOVE_LEFT, moveLeftFrames);

        AnimationFrames moveRightFrames (1);
        moveRightFrames.AddFrame (0, Rect{ Point (0, 128), Point (64, 64) });
        moveRightFrames.SetPlaySpeed (500);
        m_Animation.AddAnimationFrames (ANIM_MOVE_RIGHT, moveRightFrames);

        AnimationFrames moveUpFrames (1);
        moveUpFrames.AddFrame (0, Rect{ Point (0, 64 * 3), Point (64, 64) });
        moveUpFrames.SetPlaySpeed (500);
        m_Animation.AddAnimationFrames (ANIM_MOVE_UP, moveUpFrames);

        m_Animation.SetCurrentAnimation (ANIM_IDLE);
    }

    //--------------------------------------------------------------------------------------------------

    void Player::HandleInput (float deltaTime)
    {
        ALLEGRO_KEYBOARD_STATE state;
        al_get_keyboard_state (&state);
        double dx = 0, dy = 0;

        if (al_key_down (&state, ALLEGRO_KEY_DOWN) || al_key_down (&state, ALLEGRO_KEY_S))
        {
            m_Animation.SetCurrentAnimation (ANIM_IDLE);
            dy = MOVE_SPEED * deltaTime;
        }

        if (al_key_down (&state, ALLEGRO_KEY_UP) || al_key_down (&state, ALLEGRO_KEY_W))
        {
            m_Animation.SetCurrentAnimation (ANIM_MOVE_UP);
            dy = -MOVE_SPEED * deltaTime;
        }

        if (al_key_down (&state, ALLEGRO_KEY_RIGHT) || al_key_down (&state, ALLEGRO_KEY_D))
        {
            m_Animation.SetCurrentAnimation (ANIM_MOVE_RIGHT);
            dx = MOVE_SPEED * deltaTime;
        }

        if (al_key_down (&state, ALLEGRO_KEY_LEFT) || al_key_down (&state, ALLEGRO_KEY_A))
        {
            m_Animation.SetCurrentAnimation (ANIM_MOVE_LEFT);
            dx = -MOVE_SPEED * deltaTime;
        }

        for (Tile* tile : m_SceneManager->GetActiveScene ()->GetTiles ())
        {
            if (!tile->PhysPoints.empty ())
            {
                PolygonCollisionResult r = m_SceneManager->GetMainLoop ()->GetPhysicsManager ().PolygonCollision (
                  GetPhysPolygon (), tile->GetPhysPolygon (), { dx, dy });

                if (r.WillIntersect)
                {
                    dx = dx + r.MinimumTranslationVector.X;
                    dy = dy + r.MinimumTranslationVector.Y;
                    break;
                }
            }
        }

        if (!AreSame (dx, 0) || !AreSame (dy, 0))
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

    void Player::Move (double dx, double dy)
    {
        m_OldPosition = m_Position;
        m_Position.X += dx;
        m_Position.Y += dy;

        if (MoveCallback != nullptr)
        {
            MoveCallback (dx, dy);
        }

        AddPhysOffset ({ dx, dy });
    }

    //--------------------------------------------------------------------------------------------------

    void Player::SetPosition (const Point& pos) { SetPosition (pos.X, pos.Y); }

    //--------------------------------------------------------------------------------------------------

    void Player::SetPosition (double x, double y)
    {
        m_OldPosition = m_Position;
        m_Position.X = x;
        m_Position.Y = y;

        if (MoveCallback != nullptr)
        {
            MoveCallback (m_Position.X - m_OldPosition.X, m_Position.Y - m_OldPosition.Y);
        }
    }

    //--------------------------------------------------------------------------------------------------

    Point Player::GetPosition () { return m_Position; }

    //--------------------------------------------------------------------------------------------------

    Point Player::GetSize () { return m_Size; }

    //--------------------------------------------------------------------------------------------------
}
