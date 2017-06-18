// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __RECT_H__
#define __RECT_H__

#include "Point.h"

namespace aga
{
    struct Rect
    {
        Point TopLeft;
        Point BottomRight;
    };

    static bool InsideRect (double x, double y, const Rect& rect)
    {
        return (x >= rect.TopLeft.X && y >= rect.TopLeft.Y && x <= rect.BottomRight.X
            && y <= rect.BottomRight.Y);
    }
}

#endif //   __RECT_H__
