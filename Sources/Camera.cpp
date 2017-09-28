// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "Camera.h"
#include "Screen.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    //  ALLEGRO_TRANSFORM IdentityTransform;

    Camera::Camera (Screen* screen)
      : m_Screen (screen)
    {
        const Point& size = screen->GetScreenSize ();
        SetOffset (size.Width * 0.5, size.Height * 0.5);

        //        al_identity_transform (&IdentityTransform);
    }

    //--------------------------------------------------------------------------------------------------

    Camera::~Camera () {}

    //--------------------------------------------------------------------------------------------------

    void Camera::Update (double deltaTime)
    {
        //        al_use_transform (&m_Transform);
    }

    //--------------------------------------------------------------------------------------------------

    void Camera::SetOffset (float dx, float dy)
    {
        //    al_identity_transform (&m_Transform);
        //     al_translate_transform (&m_Transform, dx, dy);
    }

    //--------------------------------------------------------------------------------------------------

    void Camera::Move (float dx, float dy)
    {
        // al_translate_transform (&m_Transform, dx, dy);
    }

    //--------------------------------------------------------------------------------------------------

    void Camera::Scale (float dx, float dy, float mousePosX, float mousePosY)
    {
        if (mousePosX >= 0 && mousePosY >= 0)
        {
            //          al_translate_transform (&m_Transform, -mousePosX, -mousePosY);
        }

        //      al_scale_transform (&m_Transform, dx, dy);

        if (mousePosX >= 0 && mousePosY >= 0)
        {
            //       al_translate_transform (&m_Transform, mousePosX, mousePosY);
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Camera::UseIdentityTransform ()
    {
        // al_use_transform (&IdentityTransform);
    }

    //--------------------------------------------------------------------------------------------------

    void Camera::ClearTransformations ()
    {
        //        al_identity_transform (&m_Transform);
        UseIdentityTransform ();
    }

    //--------------------------------------------------------------------------------------------------

    Point Camera::GetTranslate ()
    {
        return { 0, 0 }; // -m_Transform.m[3][0], -m_Transform.m[3][1] };
    }

    //--------------------------------------------------------------------------------------------------

    Point Camera::GetScale ()
    {
        return { 1, 1 }; // m_Transform.m[0][0], m_Transform.m[1][1] };
    }

    //--------------------------------------------------------------------------------------------------
}
