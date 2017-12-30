// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __TRANSFORMABLE_H__
#define __TRANSFORMABLE_H__

#include "Rect.h"

namespace aga
{
    class Transformable
    {
    public:
        Rect Bounds;
        float Rotation = 0;
    };
}

#endif //   __TRANSFORMABLE_H__