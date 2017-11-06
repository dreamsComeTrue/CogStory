// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "Camera.h"
#include "Screen.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    ALLEGRO_TRANSFORM IdentityTransform;

    Camera::Camera (Screen* screen)
        : m_Screen (screen)
    {
        const Point& size = screen->GetWindowSize ();
        SetTranslate (size.Width * 0.5, size.Height * 0.5);

        al_identity_transform (&IdentityTransform);
    }

    //--------------------------------------------------------------------------------------------------

    Camera::~Camera () {}

    //--------------------------------------------------------------------------------------------------

    void Camera::Update (float deltaTime) { al_use_transform (&m_Transform); }

    //--------------------------------------------------------------------------------------------------

    void Camera::SetTranslate (float dx, float dy)
    {
        m_Transform.m[3][0] = dx;
        m_Transform.m[3][1] = dy;
    }

    //--------------------------------------------------------------------------------------------------

    void Camera::Move (float dx, float dy) { al_translate_transform (&m_Transform, dx, dy); }

    //--------------------------------------------------------------------------------------------------

    void Camera::Scale (float dx, float dy, float mousePosX, float mousePosY)
    {
        if (mousePosX >= 0 && mousePosY >= 0)
        {
            al_translate_transform (&m_Transform, -mousePosX, -mousePosY);
        }

        al_scale_transform (&m_Transform, dx, dy);

        if (mousePosX >= 0 && mousePosY >= 0)
        {
            al_translate_transform (&m_Transform, mousePosX, mousePosY);
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Camera::UseIdentityTransform () { al_use_transform (&IdentityTransform); }

    //--------------------------------------------------------------------------------------------------

    void Camera::ClearTransformations ()
    {
        al_identity_transform (&m_Transform);
        UseIdentityTransform ();
    }

    //--------------------------------------------------------------------------------------------------

    Point Camera::GetTranslate () { return { -m_Transform.m[3][0], -m_Transform.m[3][1] }; }

    //--------------------------------------------------------------------------------------------------

    Point Camera::GetScale () { return { m_Transform.m[0][0], m_Transform.m[1][1] }; }

    //--------------------------------------------------------------------------------------------------
}
