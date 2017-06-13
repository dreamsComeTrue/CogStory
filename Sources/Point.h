// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __POINT_H__
#define __POINT_H__

namespace aga
{
    struct Point
    {
        Point () :X(0), Y(0) {}

        Point (int x, int y)
            : X (x)
            , Y (y)
        {
        }

        union {
            struct
            {
                int X, Y;
            };
            struct
            {
                int Width, Height;
            };
        };
    };
}

#endif //   __POINT_H__
