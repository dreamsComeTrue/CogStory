// Copyright 2017-2019 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __POINT_H__
#define __POINT_H__

#include <cmath>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

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

        std::string ToString ()
        {
            std::stringstream streamX;
            streamX << std::fixed << std::setprecision (2) << X;

            std::stringstream streamY;
            streamY << std::fixed << std::setprecision (2) << Y;

            return "(X: " + streamX.str () + ", Y:" + streamY.str () + ")";
        }

        static Point MIN_POINT;
        static Point MAX_POINT;
        static Point ZERO_POINT;
    };

    //--------------------------------------------------------------------------------------------------

    inline float Dot (const Point& a, const Point& b) { return (a.X * b.X) + (a.Y * b.Y); }

    //--------------------------------------------------------------------------------------------------

    inline float PerpDot (const Point& a, const Point& b) { return (a.Y * b.X) - (a.X * b.Y); }

    //--------------------------------------------------------------------------------------------------

    inline bool LineCollision (const Point& A1, const Point& A2, const Point& B1, const Point& B2, Point& intersection)
    {
        Point a (A2 - A1);
        Point b (B2 - B1);

        float f = PerpDot (a, b);

        if (!f) // lines are parallel
        {
            return false;
        }

        Point c (B2 - A2);
        float aa = PerpDot (a, c);
        float bb = PerpDot (b, c);

        if (f < 0)
        {
            if (aa > 0)
                return false;
            if (bb > 0)
                return false;
            if (aa < f)
                return false;
            if (bb < f)
                return false;
        }
        else
        {
            if (aa < 0)
                return false;
            if (bb < 0)
                return false;
            if (aa > f)
                return false;
            if (bb > f)
                return false;
        }

        float out = 1.0f - (aa / f);
        intersection = ((B2 - B1) * out) + B1;

        return true;
    }

    //--------------------------------------------------------------------------------------------------
}

#endif //   __POINT_H__
