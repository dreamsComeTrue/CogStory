// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "Player.h"
#include "Screen.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    //  Pixels Per Second
    const double MOVE_SPEED = 70.0;
    const std::string ANIM_IDLE = "ANIM_IDLE";
    const std::string ANIM_MOVE_UP = "ANIM_MOVE_UP";
    const std::string ANIM_MOVE_LEFT = "ANIM_MOVE_LEFT";
    const std::string ANIM_MOVE_RIGHT = "ANIM_MOVE_RIGHT";

    //--------------------------------------------------------------------------------------------------

    Player::Player (Screen* screen)
        : m_Screen (screen)
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
        m_Size = { al_get_bitmap_width (m_Image), al_get_bitmap_height (m_Image) };

        InitializeAnimations ();

        tween = tweeny::from (0).to (400).during (200).onStep ([this](int value) {  Move (value * m_Screen->GetDeltaTime(), 0); return tween.progress() >= 1; });

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

    bool Player::Update (double deltaTime)
    {
        HandleInput (deltaTime);

        if (tween.progress () < 1)
        {
            tween.step ((float)deltaTime);
        }

        m_Animation.Update (deltaTime);

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    void Player::ProcessEvent (ALLEGRO_EVENT* event, double deltaTime)
    {
        if (event->type == ALLEGRO_EVENT_KEY_UP)
        {
            m_Animation.SetCurrentAnimation (ANIM_IDLE);
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Player::Render (double deltaTime)
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

    void Player::HandleInput (double deltaTime)
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

        if (!AreSame (dx, 0) || !AreSame (dy, 0))
        {
            Move (dx, dy);
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Player::Move (double dx, double dy)
    {
        m_Position.X += dx;
        m_Position.Y += dy;
    }

    //--------------------------------------------------------------------------------------------------
}
