// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "Camera.h"
#include "Screen.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    Camera::Camera (Screen* screen)
        : m_Screen (screen)
    {
        const Point& size = screen->GetScreenSize ();
        SetOffset (size.Width * 0.5, size.Height * 0.5);
    }

    //--------------------------------------------------------------------------------------------------

    Camera::~Camera ()
    {
    }

    //--------------------------------------------------------------------------------------------------

    void Camera::Update (double deltaTime)
    {
        al_use_transform (&m_Transform);
    }

    //--------------------------------------------------------------------------------------------------

    void Camera::SetOffset (float dx, float dy)
    {
        al_identity_transform (&m_Transform);
        al_translate_transform (&m_Transform, dx, dy);
    }

    //--------------------------------------------------------------------------------------------------

    void Camera::Move (float dx, float dy)
    {
        al_translate_transform (&m_Transform, dx, dy);
    }

    //--------------------------------------------------------------------------------------------------
}
