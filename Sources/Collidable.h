// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __COLLIDABLE_H__
#define __COLLIDABLE_H__

#include "Common.h"

namespace aga
{
    class Collidable
    {
    public:
        void AddPhysOffset (const Point& offset)
        {
            m_Offset.X += offset.X;
            m_Offset.Y += offset.Y;

            UpdatePhysPolygon ();
        }

        void SetPhysOffset (const Point& offset)
        {
            m_Offset = offset;

            UpdatePhysPolygon ();
        }

        void DrawPhysVertices ()
        {
            std::vector<float> out;

            for (int i = 0; i < m_PhysPolygon.Points.size (); ++i)
            {
                out.push_back (m_Offset.X + PhysPoints[i].X);
                out.push_back (m_Offset.Y + PhysPoints[i].Y);
            }

            al_draw_polygon (out.data (), PhysPoints.size (), 0, COLOR_GREEN, 2, 0);
        }

        void BuildEdges () { m_PhysPolygon.BuildEdges (); }

        Polygon& GetPhysPolygon () { return m_PhysPolygon; }

        void UpdatePhysPolygon ()
        {
            m_PhysPolygon.Points = PhysPoints;
            m_PhysPolygon.Offset (m_Offset);
            m_PhysPolygon.BuildEdges ();
        }

    public:
        std::vector<Point> PhysPoints;

    private:
        Polygon m_PhysPolygon;
        Point m_Offset;
    };
}

#endif //   __COLLIDABLE_H__
