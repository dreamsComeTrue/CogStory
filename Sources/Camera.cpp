// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "Camera.h"
#include "Actor.h"
#include "MainLoop.h"
#include "SceneManager.h"
#include "Screen.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    const int CAMERA_TWEEN_ID = 99998;

    //--------------------------------------------------------------------------------------------------

    ALLEGRO_TRANSFORM IdentityTransform;

    Camera::Camera (SceneManager* sceneManager)
        : m_SceneManager (sceneManager)
        , m_CameraFollowActor (nullptr)
        , m_TweenToPoint (nullptr)
    {
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

    void Camera::SetCenter (float x, float y)
    {
        const Point winSize = m_SceneManager->GetMainLoop ()->GetScreen ()->GetWindowSize ();

        SetTranslate (x + winSize.Width * 0.5f, y + winSize.Height * 0.5f);
    }

    //--------------------------------------------------------------------------------------------------

    Point Camera::GetCenter ()
    {
        Point trans = GetTranslate ();
        const Point winSize = m_SceneManager->GetMainLoop ()->GetScreen ()->GetWindowSize ();

        return { trans.X + winSize.Width * 0.5f, trans.Y + winSize.Height * 0.5f };
    }

    //--------------------------------------------------------------------------------------------------

    void Camera::SetFollowActor (Actor* actor)
    {
        if (m_CameraFollowActor)
        {
            m_CameraFollowActor->MoveCallback = nullptr;
        }

        m_CameraFollowActor = actor;

        if (m_CameraFollowActor)
        {
            m_CameraFollowActor->MoveCallback = [&](float dx, float dy) {
                Point scale = GetScale ();
                Point screenSize = m_SceneManager->GetMainLoop ()->GetScreen ()->GetWindowSize ();
                Point actorSize = m_CameraFollowActor->GetSize ();
                Point actorPos = m_CameraFollowActor->GetPosition ();

                SetTranslate (screenSize.Width * 0.5 - actorPos.X * scale.X - actorSize.Width,
                              screenSize.Height * 0.5 - actorPos.Y * scale.Y - actorSize.Height);
            };
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Camera::TweenToPoint (Point point, float timeMs)
    {
        if (m_CameraFollowActor)
        {
            m_CameraFollowActor->MoveCallback = nullptr;
            m_CameraFollowActor = nullptr;
        }

        auto tweenFunc = [&](float x, float y) {
            SetTranslate (x, y);

            printf ("%f, %f\n", x, y);

            return false;
        };

        Point startPoint = GetTranslate ();
        startPoint.X = -startPoint.X;
        startPoint.Y = -startPoint.Y;

        m_TweenToPoint = &m_SceneManager->GetMainLoop ()->GetTweenManager ().AddTween (CAMERA_TWEEN_ID, startPoint,
                                                                                       point, timeMs, tweenFunc);
    }

    //--------------------------------------------------------------------------------------------------
}
