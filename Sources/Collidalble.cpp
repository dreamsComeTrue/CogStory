// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "Collidable.h"
#include "PhysicsManager.h"
#include "addons/triangulator/Triangulator.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    int GetQuadrant (const Point& p)
    {
        int result = 4; // origin

        if (p.X > 0 && p.Y > 0)
        {
            return 1;
        }
        else if (p.X < 0 && p.Y > 0)
        {
            return 2;
        }
        else if (p.X < 0 && p.Y < 0)
        {
            return 3;
        }
        // else 4th quadrant
        return result;
    }

    //--------------------------------------------------------------------------------------------------

    double GetClockwiseAngle (const Point& p)
    {
        double angle = 0.0;
        int quadrant = GetQuadrant (p);

        /*add the appropriate pi/2 value based on the quadrant. (one of 0, pi/2, pi, 3pi/2)*/
        switch (quadrant)
        {
            case 1:
                angle = std::atan2 (p.X, p.Y) * 180 / M_PI;
                break;
            case 2:
                angle = std::atan2 (p.Y, p.X) * 180 / M_PI;
                angle += M_PI / 2;
                break;
            case 3:
                angle = std::atan2 (p.X, p.Y) * 180 / M_PI;
                angle += M_PI;
                break;
            case 4:
                angle = std::atan2 (p.Y, p.X) * 180 / M_PI;
                angle += 3 * M_PI / 2;
                break;
        }

        return angle;
    }

    //--------------------------------------------------------------------------------------------------

    bool ComparePoints (const Point& a, const Point& b) { return (GetClockwiseAngle (a) < GetClockwiseAngle (b)); }

    //--------------------------------------------------------------------------------------------------
    //--------------------------------------------------------------------------------------------------

    Collidable::Collidable (PhysicsManager* physicsManager)
      : m_PhysicsManager (physicsManager)
    {
    }

    //--------------------------------------------------------------------------------------------------

    void Collidable::AddPhysOffset (const Point& offset)
    {
        m_Offset.X += offset.X;
        m_Offset.Y += offset.Y;

        UpdatePhysPolygon ();
    }

    //--------------------------------------------------------------------------------------------------

    void Collidable::SetPhysOffset (const Point& offset)
    {
        m_Offset = offset;

        UpdatePhysPolygon ();
    }

    //--------------------------------------------------------------------------------------------------

    void Collidable::DrawPhysVertices ()
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

    //--------------------------------------------------------------------------------------------------

    Polygon& Collidable::GetPhysPolygon (int index) { return m_PhysPolygons[index]; }

    //--------------------------------------------------------------------------------------------------

    int Collidable::GetPhysPolygonsCount () const { return m_PhysPolygons.size (); }

    //--------------------------------------------------------------------------------------------------

    void Collidable::UpdatePhysPolygon ()
    {
        m_PhysPolygons.clear ();

        int counter = 0;
        for (int i = 0; i < PhysPoints.size (); ++i)
        {
            std::vector<std::vector<Point>> result;

            int minX = 10000000, maxX = -100000000;
            int minY = 10000000, maxY = -100000000;

            for (int a = 0; a < PhysPoints[i].size (); ++a)
            {
                Point& point = PhysPoints[i][a];

                if (point.X < minX)
                {
                    minX = point.X;
                }
                if (point.X > maxX)
                {
                    maxX = point.X;
                }
                if (point.Y < minY)
                {
                    minY = point.Y;
                }
                if (point.Y < maxY)
                {
                    maxY = point.Y;
                }
            }

            int halfX = (maxX - minX) / 2;
            int halfY = (maxY - minY) / 2;

            for (int a = 0; a < PhysPoints[i].size (); ++a)
            {
                PhysPoints[i][a].X -= halfX;
                PhysPoints[i][a].Y -= halfY;
            }

            //   std::sort (PhysPoints[i].begin (), PhysPoints[i].end (), ComparePoints);

            for (int a = 0; a < PhysPoints[i].size (); ++a)
            {
                PhysPoints[i][a].X += halfX;
                PhysPoints[i][a].Y += halfY;
            }

            if (m_PhysicsManager->GetTriangulator ().Validate (PhysPoints[i]) == 0)
            {
                m_PhysicsManager->GetTriangulator ().ProcessVertices (&PhysPoints[i], result);

                for (int j = 0; j < result.size (); ++j)
                {
                    m_PhysPolygons.push_back (Polygon ());
                    m_PhysPolygons[counter].Points = result[j];
                    m_PhysPolygons[counter].Offset (m_Offset);
                    m_PhysPolygons[counter].BuildEdges ();

                    ++counter;
                }
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Collidable::BuildEdges ()
    {
        for (int i = 0; i < m_PhysPolygons.size (); ++i)
        {
            m_PhysPolygons[i].BuildEdges ();
        }
    }

    //--------------------------------------------------------------------------------------------------
}
