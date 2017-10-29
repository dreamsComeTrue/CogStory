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
            for (int i = 0; i < PhysPoints.size (); ++i)
            {
                std::vector<float> out;

                for (int j = 0; j < PhysPoints[i].size (); ++j)
                {
                    out.push_back (m_Offset.X + PhysPoints[i][j].X);
                    out.push_back (m_Offset.Y + PhysPoints[i][j].Y);
                }

                al_draw_polygon (out.data (), PhysPoints[i].size (), 0, COLOR_GREEN, 2, 0);
            }
        }

        Polygon& GetPhysPolygon (int index) { return m_PhysPolygons[index]; }

        int GetPhysPolygonsCount () const { return m_PhysPolygons.size (); }

        void UpdatePhysPolygon ()
        {
            m_PhysPolygons.clear ();

            for (int i = 0; i < PhysPoints.size (); ++i)
            {
                m_PhysPolygons.push_back (Polygon ());

                m_PhysPolygons[i].Points = PhysPoints[i];
                m_PhysPolygons[i].Offset (m_Offset);
                m_PhysPolygons[i].BuildEdges ();
            }
        }

    private:
        void BuildEdges ()
        {
            for (int i = 0; i < m_PhysPolygons.size (); ++i)
            {
                m_PhysPolygons[i].BuildEdges ();
            }
        }

    public:
        std::vector<std::vector<Point>> PhysPoints;

    private:
        std::vector<Polygon> m_PhysPolygons;
        Point m_Offset;
    };
}

#endif //   __COLLIDABLE_H__
