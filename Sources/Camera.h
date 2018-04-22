// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __CAMERA_H__
#define __CAMERA_H__

#include "Common.h"

namespace aga
{
    class SceneManager;
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

        void SetTranslate (float dx, float dy);
        Point GetTranslate ();

        void Scale (float dx, float dy, float mousePosX = -1, float mousePosY = -1);
        Point GetScale ();

        void SetCenter (float x, float y);
        Point GetCenter ();

        void SetFollowActor (Actor* actor);
        void TweenToPoint (Point endPoint, float timeMs = 1000, bool centerScreen = true);
        void TweenToPoint (Point endPoint, asIScriptFunction* finishFunc, float timeMs = 1000,
                           bool centerScreen = true);

    private:
        ALLEGRO_TRANSFORM m_Transform;
        SceneManager* m_SceneManager;

        Actor* m_CameraFollowActor;
        TweenData* m_TweenToPoint;
    };
}

#endif //   __CAMERA_H__
