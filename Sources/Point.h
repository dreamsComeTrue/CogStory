// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __POINT_H__
#define __POINT_H__

#include <cmath>

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
            : X ((float)x)
            , Y ((float)y)
        {
        }

        Point (float x, float y)
            : X (x)
            , Y (y)
        {
        }

        union {
            struct
            {
                float X, Y;
            };
            struct
            {
                float Width, Height;
            };
        };

        float Magnitude () { return (float)std::sqrt (X * X + Y * Y); }

        void Normalize ()
        {
            float magnitude = Magnitude ();
            X = X / magnitude;
            Y = Y / magnitude;
        }

        void Set (float x, float y)
        {
            X = x;
            Y = y;
        }

        float DotProduct (const Point& vector) { return X * vector.X + Y * vector.Y; }

        bool operator== (const Point& rhs) const { return X == rhs.X && Y == rhs.Y; }

        bool operator!= (const Point& rhs) const { return X != rhs.X || Y != rhs.Y; }

        Point operator+ (const Point& rhs) { return Point (X + rhs.X, Y + rhs.Y); }

        Point operator* (const Point& rhs) { return Point (X * rhs.X, Y * rhs.Y); }

        Point operator- (const Point& rhs) { return Point (X - rhs.X, Y - rhs.Y); }

        Point operator- () { return { -X, -Y }; }

        Point operator* (float b) { return { X * b, Y * b }; }

        void operator*= (float p)
        {
            X *= p;
            Y *= p;
        }

        void operator+= (const Point& p)
        {
            X += p.X;
            Y += p.Y;
        }

        void operator-= (const Point& p)
        {
            X -= p.X;
            Y -= p.Y;
        }

        static Point MIN_POINT;
        static Point ZERO_POINT;
    };
}

#endif //   __POINT_H__
