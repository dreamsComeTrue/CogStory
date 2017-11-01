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

        float Magnitude () { return (float)std::sqrt (X * X + Y * Y); }

        void Normalize ()
        {
            float magnitude = Magnitude ();
            X = X / magnitude;
            Y = Y / magnitude;
        }

        float DotProduct (const Point& vector) { return X * vector.X + Y * vector.Y; }

        bool operator== (const Point& rhs) const { return X == rhs.X && Y == rhs.Y; }

        Point operator+ (const Point& rhs) { return Point (X + rhs.X, Y + rhs.Y); }

        Point operator- (const Point& rhs) { return Point (X - rhs.X, Y - rhs.Y); }

        Point operator- () { return { -X, -Y }; }

        Point operator* (float b) { return { X * b, Y * b }; }
    };
}

#endif //   __POINT_H__
