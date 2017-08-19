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
        void SetOffset (float dx, float dy);
        void Move (float dx, float dy);
        void Scale (float dx, float dy, float mousePosX = -1, float mousePosY = -1);
        void Reset ();
        void ClearTransformations ();

        Point GetTranslate ();
        Point GetScale ();

    private:
        ALLEGRO_TRANSFORM m_Transform;
        Point m_Position;
        Point m_Scale;
        Screen* m_Screen;
    };
}

#endif //   __CAMERA_H__
