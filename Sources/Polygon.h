// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __POLYGON_H__
#define __POLYGON_H__

#include "Point.h"

#include <vector>

namespace aga
{
    struct Polygon
    {
        std::vector<Point> Points;
        std::vector<Point> Edges;

        void BuildEdges ()
        {
            Point p1;
            Point p2;
            Edges.clear ();

            for (size_t i = 0; i < Points.size (); i++)
            {
                p1 = Points[i];

                if (i + 1 >= Points.size ())
                {
                    p2 = Points[0];
                }
                else
                {
                    p2 = Points[i + 1];
                }

                Edges.push_back (p2 - p1);
            }
        }

        Point GetCenter ()
        {
            float totalX = 0;
            float totalY = 0;

            for (size_t i = 0; i < Points.size (); i++)
            {
                totalX += Points[i].X;
                totalY += Points[i].Y;
            }

            return Point (totalX / (float)Points.size (), totalY / (float)Points.size ());
        }

        void Offset (Point v) { Offset (v.X, v.Y); }

        void Offset (float x, float y)
        {
            for (size_t i = 0; i < Points.size (); i++)
            {
                Point& p = Points[i];
                p.X += x;
                p.Y += y;
            }
        }
    };
}

#endif //   __POLYGON_H__
