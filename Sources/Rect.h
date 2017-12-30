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
            SetTopLeft (topLeft);
            SetBottomRight (bottomRight);
        }

        Point GetTopLeft () { return Pos; }
        void SetTopLeft (Point p) { Pos = p; }

        Point GetBottomRight () { return Pos + Size; }
        void SetBottomRight (Point p) { Size = p - Pos; }

        Point GetPos () { return Pos; }
        void SetPos (Point p) { Pos = p; }

        Point GetSize () { return Size; }
        void SetSize (Point p) { Size = p; }

        Point GetCenter () { return Point{ Pos.X + Pos.Width * 0.5f, Pos.Y + Pos.Height * 0.5f }; }

        Point Pos;
        Point Size;

        bool operator== (const Rect& rhs) const { return Pos == rhs.Pos && Size == rhs.Size; }
    };

    static bool Intersect (Rect rectA, Rect rectB)
    {
        return rectA.GetTopLeft ().X < rectB.GetBottomRight ().X && rectB.GetTopLeft ().X < rectA.GetBottomRight ().X &&
               rectA.GetTopLeft ().Y < rectB.GetBottomRight ().Y && rectB.GetTopLeft ().Y < rectA.GetBottomRight ().Y;
    }

    static bool InsideRect (float x, float y, Rect& rect)
    {
        return (x >= rect.GetTopLeft ().X && y >= rect.GetTopLeft ().Y && x <= rect.GetBottomRight ().X &&
                y <= rect.GetBottomRight ().Y);
    }

    static bool InsideRect (const Point& point, Rect& rect) { return InsideRect (point.X, point.Y, rect); }
}

#endif //   __RECT_H__
