// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "Camera.h"
#include "Screen.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    ALLEGRO_TRANSFORM IdentityTransform;

    Camera::Camera (Screen* screen)
      : m_Screen (screen)
      , m_Scale (1.0, 1.0)
    {
        const Point& size = screen->GetScreenSize ();
        SetOffset (size.Width * 0.5, size.Height * 0.5);

        al_identity_transform (&IdentityTransform);
    }

    //--------------------------------------------------------------------------------------------------

    Camera::~Camera () {}

    //--------------------------------------------------------------------------------------------------

    void Camera::Update (double deltaTime) { al_use_transform (&m_Transform); }

    //--------------------------------------------------------------------------------------------------

    void Camera::SetOffset (float dx, float dy)
    {
        m_Position.X += dx;
        m_Position.Y += dy;

        al_identity_transform (&m_Transform);
        al_translate_transform (&m_Transform, dx, dy);
    }

    //--------------------------------------------------------------------------------------------------

    void Camera::Move (float dx, float dy)
    {
        m_Position.X += dx;
        m_Position.Y += dy;

        al_translate_transform (&m_Transform, dx, dy);
    }

    //--------------------------------------------------------------------------------------------------

    void Camera::Scale (float dx, float dy, float mousePosX, float mousePosY)
    {
        if (mousePosX >= 0 && mousePosY >= 0)
        {
            al_translate_transform (&m_Transform, -mousePosX, -mousePosY);
        }

        m_Scale.X *= dx;
        m_Scale.Y *= dy;

        al_scale_transform (&m_Transform, dx, dy);

        if (mousePosX >= 0 && mousePosY >= 0)
        {
            al_translate_transform (&m_Transform, mousePosX, mousePosY);
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Camera::Reset ()
    {
        m_Position.X = 0;
        m_Position.Y = 0;

        al_use_transform (&IdentityTransform);
    }

    //--------------------------------------------------------------------------------------------------

    void Camera::ClearTransformations ()
    {
        m_Position.X = 0;
        m_Position.Y = 0;

        al_identity_transform (&m_Transform);
        Reset ();
    }

    //--------------------------------------------------------------------------------------------------

    Point Camera::GetTranslate () { return { -m_Transform.m[3][0], -m_Transform.m[3][1] }; }

    //--------------------------------------------------------------------------------------------------

    Point Camera::GetScale () { return m_Scale; }

    //--------------------------------------------------------------------------------------------------
}
