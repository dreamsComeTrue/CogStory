// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __CAMERA_H__
#define __CAMERA_H__

#include "Common.h"

namespace aga
{
    class Screen;

    class Camera
    {
    public:
        Camera (Screen* screen);
        virtual ~Camera ();

        void Update (float deltaTime);
        void SetTranslate (float dx, float dy);
        Point GetTranslate ();
        void Move (float dx, float dy);
        void Scale (float dx, float dy, float mousePosX = -1, float mousePosY = -1);
        void UseIdentityTransform ();
        void ClearTransformations ();

        Point GetScale ();

    private:
        ALLEGRO_TRANSFORM m_Transform;
        Screen* m_Screen;
    };
}

#endif //   __CAMERA_H__
