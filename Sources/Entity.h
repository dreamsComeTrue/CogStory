// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __ENTITY_H__
#define __ENTITY_H__

#include "Rect.h"

namespace aga
{
    class Entity
    {
    public:
        int ID = 0;
        Rect Bounds;
    };
}

#endif //   __ENTITY_H__
