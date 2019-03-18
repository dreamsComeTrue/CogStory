// Copyright 2017-2019 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __CAMERA_H__
#define __CAMERA_H__

#include "Common.h"

namespace aga
{
    class SceneManager;
    class Entity;
    class Actor;    
    struct TweenData;

    class Camera
    {
    public:
        Camera (SceneManager* sceneManager);
        virtual ~Camera ();

        void Update (float deltaTime);
        void Move (float dx, float dy);
        void UseIdentityTransform ();
        void ClearTransformations ();

        void SetTranslate (Point point);
        void SetTranslate (float dx, float dy);
        Point GetTranslate ();

        ALLEGRO_TRANSFORM GetCurrentTransform ();
        void SetCurrentTransform (ALLEGRO_TRANSFORM transform);

        void Scale (float dx, float dy, float mousePosX = -1, float mousePosY = -1);
        Point GetScale ();

        void SetCenter (float x, float y);
        void SetCenter (Point point);
        Point GetCenter ();

        void SetFollowActor (Actor* actor, Point followOffset = {0.f, 0.f});
        Actor* GetFollowActor ();

        void SetFollowOffset (Point offset);

        void SetFollowingXAxis (bool enabled);
        bool IsFollowingXAxis ();
        void SetFollowingYAxis (bool enabled);
        bool IsFollowingYAxis ();
        void SetSavedFollowPoint (Point p);
        Point GetSavedFollowPoint () const;

        void TweenToPoint (Point endPoint, float timeMs = 1000, bool centerScreen = true);
        void TweenToPoint (
            Point endPoint, float timeMs = 1000, bool centerScreen = true, asIScriptFunction* finishFunc = nullptr);

        void Shake (float timeMs = 500, float oscilatingTime = 0.2f, float rangePixels = 5.f);

        Rect GetRenderBounds (Entity* entity, bool drawOOBBox = false);
        Rect GetRenderBounds (Rect b);
        
    private:
        void ShakeFunction (float progress);

    private:
        ALLEGRO_TRANSFORM m_Transform;
        SceneManager* m_SceneManager;

        Actor* m_CameraFollowActor;
        Point m_FollowOffset;

        TweenData* m_TweenToPoint;

        float m_ShakeRangePixels;
        float m_ShakePercentage;
        float m_ShakeOscilatingTime;
        Point m_ShakeStartPos;
        Point m_ShakeComputedPos;
        bool m_ShakeComputed;

        Point m_SavedFollowPoint;
        bool m_FollowingEnabledXAxis;
        bool m_FollowingEnabledYAxis;
    };
}

#endif //   __CAMERA_H__
