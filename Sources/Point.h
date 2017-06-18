// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __POINT_H__
#define __POINT_H__

namespace aga
{
    struct Point
    {
        Point ()
            : X (0.0)
            , Y (0.0)
        {
        }

        Point (int x, int y)
            : X (x)
            , Y (y)
        {
        }

        Point (double x, double y)
            : X (x)
            , Y (y)
        {
        }

        union {
            struct
            {
                double X, Y;
            };
            struct
            {
                double Width, Height;
            };
        };
    };
}

#endif //   __POINT_H__
