// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __POINT_H__
#define __POINT_H__

#include <cmath>

namespace aga
{
    auto Compare = [](float a, float b, float epsilon = 1.0E-8f) -> bool { return std::fabs (a - b) < epsilon; };

    struct Point
    {
    public:
        Point ()
            : X (0.0)
            , Y (0.0)
        {
        }

        Point (int x, int y)
            : X (static_cast<float> (x))
            , Y (static_cast<float> (y))
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

        float Magnitude () const { return static_cast<float> (std::sqrt (X * X + Y * Y)); }

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

        bool operator== (const Point& rhs) const { return Compare (X, rhs.X) && Compare (Y, rhs.Y); }

        bool operator!= (const Point& rhs) const { return !Compare (X, rhs.X) || !Compare (Y, rhs.Y); }

        Point operator+ (const Point& rhs) const { return Point (X + rhs.X, Y + rhs.Y); }

        Point operator* (const Point& rhs) const { return Point (X * rhs.X, Y * rhs.Y); }

        Point operator/ (const Point& rhs) const { return Point (X / rhs.X, Y / rhs.Y); }

        Point operator- (const Point& rhs) const { return Point (X - rhs.X, Y - rhs.Y); }

        bool operator< (const Point& rhs) const { return X < rhs.X || Y < rhs.Y; }

        bool operator> (const Point& rhs) const { return X > rhs.X || Y > rhs.Y; }

        Point operator- () const { return {-X, -Y}; }

        Point operator* (float b) const { return {X * b, Y * b}; }

        Point operator/ (float b) const { return {X / b, Y / b}; }

        friend float Distance (const Point& lhs, const Point& rhs)
        {
            float xDiff = lhs.X - rhs.X;
            float yDiff = lhs.Y - rhs.Y;

            return std::sqrt (xDiff * xDiff + yDiff * yDiff);
        }

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

        void operator*= (const Point& p)
        {
            X *= p.X;
            Y *= p.Y;
        }

        void operator/= (const Point& p)
        {
            X /= p.X;
            Y /= p.Y;
        }

        static Point MIN_POINT;
        static Point MAX_POINT;
        static Point ZERO_POINT;
    };
}

#endif //   __POINT_H__
