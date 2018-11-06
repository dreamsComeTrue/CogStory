// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "Camera.h"
#include "Actor.h"
#include "MainLoop.h"
#include "SceneManager.h"
#include "Screen.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    static int CAMERA_TWEEN_ID = 100000;

    //--------------------------------------------------------------------------------------------------

    ALLEGRO_TRANSFORM IdentityTransform;

    Camera::Camera (SceneManager* sceneManager)
        : m_SceneManager (sceneManager)
        , m_CameraFollowActor (nullptr)
        , m_TweenToPoint (nullptr)
        , m_FollowingEnabledXAxis (false)
        , m_FollowingEnabledYAxis (false)
        , m_SavedFollowPoint (Point::ZERO_POINT)
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

    void Camera::SetTranslate (Point point) { SetTranslate (point.X, point.Y); }

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

    Point Camera::GetTranslate () { return {-m_Transform.m[3][0], -m_Transform.m[3][1]}; }

    //--------------------------------------------------------------------------------------------------

    Point Camera::GetScale () { return {m_Transform.m[0][0], m_Transform.m[1][1]}; }

    //--------------------------------------------------------------------------------------------------

    ALLEGRO_TRANSFORM& Camera::GetCurrentTransform () { return m_Transform; }

    //--------------------------------------------------------------------------------------------------

    void Camera::SetCurrentTransform (ALLEGRO_TRANSFORM& transform) { m_Transform = transform; }

    //--------------------------------------------------------------------------------------------------

    void Camera::SetCenter (float x, float y)
    {
        const Point winSize = m_SceneManager->GetMainLoop ()->GetScreen ()->GetWindowSize ();

        SetTranslate (-x * 2 + winSize.Width * 0.5f, -y * 2 + winSize.Height * 0.5f);
    }

    //--------------------------------------------------------------------------------------------------

    void Camera::SetCenter (Point point) { SetCenter (point.X, point.Y); }

    //--------------------------------------------------------------------------------------------------

    Point Camera::GetCenter ()
    {
        Point trans = GetTranslate ();
        Point scale = GetScale ();
        const Point winSize = m_SceneManager->GetMainLoop ()->GetScreen ()->GetWindowSize ();

        return {
            (trans.X + winSize.Width * 0.5f) * (1.0f / scale.X), (trans.Y + winSize.Height * 0.5f) * (1.0f / scale.Y)};
    }

    //--------------------------------------------------------------------------------------------------

    void Camera::SetFollowActor (Actor* actor, Point followOffset)
    {
        if (m_CameraFollowActor)
        {
            m_CameraFollowActor->MoveCallback = nullptr;
        }

        m_CameraFollowActor = actor;
        m_FollowOffset = followOffset;
        m_FollowingEnabledXAxis = true;
        m_FollowingEnabledYAxis = true;

        if (m_CameraFollowActor)
        {
            m_CameraFollowActor->MoveCallback = [&](float dx, float dy) {
                Point scale = GetScale ();
                Point screenSize = m_SceneManager->GetMainLoop ()->GetScreen ()->GetWindowSize ();
                Point actorHalfSize = m_CameraFollowActor->Bounds.GetHalfSize ();
                Point pos = m_CameraFollowActor->GetPosition ();

                if (m_FollowingEnabledXAxis)
                {
                    m_SavedFollowPoint.X
                        = screenSize.Width * 0.5 - (pos.X + actorHalfSize.Width) * scale.X - m_FollowOffset.X;
                }

                if (m_FollowingEnabledYAxis)
                {
                    m_SavedFollowPoint.Y
                        = screenSize.Height * 0.5 - (pos.Y + actorHalfSize.Height) * scale.Y - m_FollowOffset.Y;
                }

                SetTranslate (m_SavedFollowPoint);
            };

            //  In case of first new frame rendered - update camera with new actor as a target
            m_CameraFollowActor->MoveCallback (0, 0);
        }
    }

    //--------------------------------------------------------------------------------------------------

    Actor* Camera::GetFollowActor () { return m_CameraFollowActor; }

    //--------------------------------------------------------------------------------------------------

    void Camera::SetFollowOffset (Point offset) { m_FollowOffset = offset; }

    //--------------------------------------------------------------------------------------------------

    bool Camera::IsFollowingXAxis () { return m_FollowingEnabledXAxis; }

    //--------------------------------------------------------------------------------------------------

    bool Camera::IsFollowingYAxis () { return m_FollowingEnabledYAxis; }

    //--------------------------------------------------------------------------------------------------

    void Camera::SetSavedFollowPoint (Point p) { m_SavedFollowPoint = p; }

    //--------------------------------------------------------------------------------------------------

    Point Camera::GetSavedFollowPoint () const { return m_SavedFollowPoint; }

    //--------------------------------------------------------------------------------------------------

    void Camera::SetFollowingXAxis (bool enabled)
    {
        m_FollowingEnabledXAxis = enabled;

        m_CameraFollowActor->MoveCallback (0, 0);
    }

    //--------------------------------------------------------------------------------------------------

    void Camera::SetFollowingYAxis (bool enabled)
    {
        m_FollowingEnabledYAxis = enabled;

        m_CameraFollowActor->MoveCallback (0, 0);
    }

    //--------------------------------------------------------------------------------------------------

    void Camera::TweenToPoint (Point endPoint, float timeMs, bool centerScreen)
    {
        TweenToPoint (endPoint, timeMs, centerScreen, nullptr);
    }

    //--------------------------------------------------------------------------------------------------

    void Camera::TweenToPoint (Point endPoint, float timeMs, bool centerScreen, asIScriptFunction* finishFunc)
    {
        if (m_CameraFollowActor)
        {
            m_CameraFollowActor->MoveCallback = nullptr;
            m_CameraFollowActor = nullptr;
        }

        auto tweenFunc = [&](float x, float y) {
            SetTranslate (x, y);

            return false;
        };

        Point startPoint = -GetTranslate ();

        if (centerScreen)
        {
            const Point winSize = m_SceneManager->GetMainLoop ()->GetScreen ()->GetWindowSize ();

            //  Weird multiplication, but works :)
            endPoint.X = -endPoint.X * 2;
            endPoint.Y = -endPoint.Y * 2;

            endPoint.X += winSize.Width * 0.5f;
            endPoint.Y += winSize.Height * 0.5f;
        }

        if (finishFunc)
        {
            m_TweenToPoint = &m_SceneManager->GetMainLoop ()->GetTweenManager ().AddTween (
                CAMERA_TWEEN_ID++, startPoint, endPoint, timeMs, tweenFunc, finishFunc);
        }
        else
        {
            m_TweenToPoint = &m_SceneManager->GetMainLoop ()->GetTweenManager ().AddTween (
                CAMERA_TWEEN_ID++, startPoint, endPoint, timeMs, tweenFunc);
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Camera::ShakeFunction (float progress)
    {
        if (!m_ShakeComputed)
        {
            if (progress < 0.8)
            {
                float newX = RandInRange (-m_ShakeRangePixels, m_ShakeRangePixels);
                float newY = RandInRange (-m_ShakeRangePixels, m_ShakeRangePixels);

                m_ShakeComputedPos = GetTranslate () + Point (newX, newY);
            }
            else
            {
                m_ShakeComputedPos = -m_ShakeStartPos;
            }

            m_ShakeComputed = true;
            m_ShakePercentage = 0.f;
        }

        Point newPos = Lerp (GetTranslate (), m_ShakeComputedPos, m_ShakePercentage);
        m_ShakePercentage += m_ShakeOscilatingTime;

        if (m_ShakePercentage >= 1.0f || AreSame (newPos, m_ShakeComputedPos))
        {
            m_ShakePercentage = 1.0f;
            m_ShakeComputed = false;
        }

        SetTranslate (-newPos);
    }

    //--------------------------------------------------------------------------------------------------

    void Camera::Shake (float timeMs, float oscilatingTime, float rangePixels)
    {
        m_ShakeRangePixels = rangePixels;
        m_ShakeStartPos = -GetTranslate ();
        m_ShakeComputed = false;
        m_ShakePercentage = 0.f;
        m_ShakeOscilatingTime = oscilatingTime;

        std::function<bool(float)> tweenFunc = [&](float progress) {
            if (progress < 1.0f)
            {
                ShakeFunction (progress);
            }
            else
            {
                SetTranslate (m_ShakeStartPos);
            }

            return false;
        };

        &m_SceneManager->GetMainLoop ()->GetTweenManager ().AddTween (CAMERA_TWEEN_ID++, 0.f, 1.0f, timeMs, tweenFunc);
    }

    //--------------------------------------------------------------------------------------------------
}
