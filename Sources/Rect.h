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

        Rect (float x, float y, float width, float height)
        {
            SetTopLeft ({x, y});
            SetSize ({width, height});
        }

        Point GetTopLeft () { return Pos; }
        void SetTopLeft (Point p) { Pos = p; }

        Point GetBottomRight () { return Pos + Size; }
        void SetBottomRight (Point p) { Size = p - Pos; }

        Point GetPos () { return Pos; }
        void SetPos (Point p) { Pos = p; }
        void SetPos (float x, float y) { Pos.Set (x, y); }

        Point GetSize () { return Size; }
        void SetSize (Point p) { Size = p; }
        void SetSize (float x, float y) { Size.Set (x, y); }

        void Offset (Point p) { Offset (p.X, p.Y); }
        void Offset (float x, float y) { SetPos (Pos.X + x, Pos.Y + y); }

        Point GetCenter () { return Point{Pos.X + Size.Width * 0.5f, Pos.Y + Size.Height * 0.5f}; }
        Point GetHalfSize () { return Point{Size.Width * 0.5f, Size.Height * 0.5f}; }

        Point Pos;
        Point Size;

        bool operator== (const Rect& rhs) const { return Pos == rhs.Pos && Size == rhs.Size; }
    };

    static Rect OrientRect (float x1, float y1, float x2, float y2)
    {
        float minX = std::min (x1, x2);
        float minY = std::min (y1, y2);
        float maxX = std::max (x1, x2);
        float maxY = std::max (y1, y2);

        return {{minX, minY}, {maxX, maxY}};
    }

    static bool Intersect (Rect rectA, Rect rectB)
    {
        return rectA.GetTopLeft ().X < rectB.GetBottomRight ().X && rectB.GetTopLeft ().X < rectA.GetBottomRight ().X
            && rectA.GetTopLeft ().Y < rectB.GetBottomRight ().Y && rectB.GetTopLeft ().Y < rectA.GetBottomRight ().Y;
    }

    static bool InsideRect (float x, float y, Rect& rect)
    {
        return (x >= rect.GetTopLeft ().X && y >= rect.GetTopLeft ().Y && x <= rect.GetBottomRight ().X
            && y <= rect.GetBottomRight ().Y);
    }

    static bool InsideRect (const Point& point, Rect& rect) { return InsideRect (point.X, point.Y, rect); }
}

#endif //   __RECT_H__
