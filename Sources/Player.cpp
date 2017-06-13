// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "Player.h"
#include "Common.h"
#include "Screen.h"

namespace aga
{
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

    bool Player::Update ()
    {
        return true;
    }

    //--------------------------------------------------------------------------------------------------

    void Player::ProcessEvent (ALLEGRO_EVENT* event)
    {
    }

    //--------------------------------------------------------------------------------------------------

    void Player::Render ()
    {
        AnimationFrames& frames = m_Animation.GetAnimation ("IDLE");
        const Rect& frame = frames.GetFrame (0);
        int width = frame.BottomRight.Width;
        int height = frame.BottomRight.Height;

        al_draw_scaled_bitmap (m_Image, frame.TopLeft.X, frame.TopLeft.Y, width, height, 0, 0, width, height, 0);
    }

    //--------------------------------------------------------------------------------------------------

    void Player::InitializeAnimations ()
    {
        AnimationFrames idleFrames (3);
        idleFrames.AddFrame (0, Rect{ Point (0, 0), Point (64, 64) });

        m_Animation.AddAnimationFrames ("IDLE", idleFrames);
    }

    //--------------------------------------------------------------------------------------------------
}
