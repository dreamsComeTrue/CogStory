// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __RECT_H__
#define __RECT_H__

#include "Point.h"

namespace aga
{
    struct Rect
    {
        Rect () {}

        Rect (Point topLeft, Point bottomRight)
        {
            Dim.TopLeft = topLeft;
            Dim.BottomRight = bottomRight;
        }

        union {
            struct
            {
                Point TopLeft;
                Point BottomRight;
            } Dim;
            struct
            {
                Point Pos;
                Point Size;
            } Transform;
        };

        //        Rect () {}

        //        Rect (double x, double y, double width, double height)
        //        {
        //            TopLeft.X = x;
        //            TopLeft.Y = y;
        //            BottomRight.Width = width;
        //            BottomRight.Height = height;
        //        }
        bool operator== (const Rect& rhs) const { return Dim.TopLeft == rhs.Dim.TopLeft && Dim.BottomRight == rhs.Dim.BottomRight; }
    };

    static bool InsideRect (double x, double y, const Rect& rect)
    {
        return (x >= rect.Dim.TopLeft.X && y >= rect.Dim.TopLeft.Y && x <= rect.Dim.BottomRight.X && y <= rect.Dim.BottomRight.Y);
    }

    static bool InsideRect (const Point& point, const Rect& rect) { return InsideRect (point.X, point.Y, rect); }
}

#endif //   __RECT_H__
