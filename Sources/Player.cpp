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
        m_Image = m_Screen->LoadTexture (GetResourcePath (ResourceID::GFX_PLAYER));
        m_Size = { 64, 64 };

        InitializeAnimations ();

        Lifecycle::Initialize ();
    }

    //--------------------------------------------------------------------------------------------------

    bool Player::Destroy ()
    {
        if (m_Image != nullptr)
        {
            SDL_DestroyTexture (m_Image);
            m_Image = nullptr;
        }

        Lifecycle::Destroy ();
    }

    //--------------------------------------------------------------------------------------------------

    bool Player::Update (double deltaTime)
    {
        m_Animation.Update (deltaTime);
        UpdateScripts (deltaTime);

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    void Player::ProcessEvent (SDL_Event* event, double deltaTime)
    {
        if (event->type == SDL_KEYUP)
        {
            m_Animation.SetCurrentAnimation (ANIM_IDLE);
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Player::Render (double deltaTime)
    {
        AnimationFrames& frames = m_Animation.GetCurrentAnimation ();
        const Rect& frame = frames.GetFrame (m_Animation.GetCurrentFrame ());

        SDL_Rect srcRect = { frame.TopLeft.X, frame.TopLeft.Y, frame.BottomRight.Width, frame.BottomRight.Height };
        SDL_Rect dstRect = { m_Position.X, m_Position.Y, frame.BottomRight.Width, frame.BottomRight.Height };

        SDL_RenderCopyEx (m_Screen->GetRenderer (), m_Image, &srcRect, &dstRect, 0, NULL, SDL_FLIP_NONE);
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
        const Uint8* state = SDL_GetKeyboardState (NULL);
        double dx = 0, dy = 0;

        if (state[SDL_SCANCODE_DOWN] || state[SDL_SCANCODE_S])
        {
            m_Animation.SetCurrentAnimation (ANIM_IDLE);
            dy = MOVE_SPEED * deltaTime;
        }

        if (state[SDL_SCANCODE_UP] || state[SDL_SCANCODE_W])
        {
            m_Animation.SetCurrentAnimation (ANIM_MOVE_UP);
            dy = -MOVE_SPEED * deltaTime;
        }

        if (state[SDL_SCANCODE_RIGHT] || state[SDL_SCANCODE_D])
        {
            m_Animation.SetCurrentAnimation (ANIM_MOVE_RIGHT);
            dx = MOVE_SPEED * deltaTime;
        }

        if (state[SDL_SCANCODE_LEFT] || state[SDL_SCANCODE_A])
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
        m_OldPosition = m_Position;
        m_Position.X += dx;
        m_Position.Y += dy;

        if (MoveCallback != nullptr)
        {
            MoveCallback (dx, dy);
        }
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
