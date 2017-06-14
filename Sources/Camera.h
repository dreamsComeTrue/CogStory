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

    private:
        ALLEGRO_TRANSFORM m_Transform;
        Point m_Position;
        Screen* m_Screen;
    };
}

#endif //   __CAMERA_H__
