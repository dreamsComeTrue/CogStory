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

        void Update (double deltaTime);
        void SetTranslate (float dx, float dy);
        Point& GetTranslate ();
        void Move (float dx, float dy);
        void Scale (float dx, float dy, float mousePosX = -1, float mousePosY = -1);
        void UseIdentityTransform ();
        void ClearTransformations ();

        Point GetScale ();

    private:
        glm::mat3 m_Transform;
        Point m_Translate;
        Screen* m_Screen;
    };
}

#endif //   __CAMERA_H__
