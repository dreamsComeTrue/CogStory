// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "Collidable.h"
#include "MainLoop.h"
#include "PhysicsManager.h"
#include "Screen.h"
#include "addons/triangulator/Triangulator.h"

namespace aga
{
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

    void Collidable::SetPhysOffset (float offsetX, float offsetY)
    {
        m_Offset.Set (offsetX, offsetY);
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
                float xPoint = m_Offset.X + PhysPoints[i][j].X;
                float yPoint = m_Offset.Y + PhysPoints[i][j].Y;

                out.push_back (xPoint);
                out.push_back (yPoint);
            }

            al_draw_polygon (out.data (), (int)PhysPoints[i].size (), 0, COLOR_GREEN, 2, 0);
        }
    }

    //--------------------------------------------------------------------------------------------------

    Polygon& Collidable::GetPhysPolygon (int index) { return m_PhysPolygons[index]; }

    //--------------------------------------------------------------------------------------------------

    size_t Collidable::GetPhysPolygonsCount () const { return m_PhysPolygons.size (); }

    //--------------------------------------------------------------------------------------------------

    void Collidable::UpdatePhysPolygon ()
    {
        m_PhysPolygons.clear ();

        int counter = 0;
        for (int i = 0; i < PhysPoints.size (); ++i)
        {
            std::vector<std::vector<Point>> result;
            std::vector<Point> pointsCopy = PhysPoints[i];

            int validate = m_PhysicsManager->GetTriangulator ().Validate (pointsCopy);

            if (validate == 2)
            {
                std::reverse (pointsCopy.begin (), pointsCopy.end ());
            }

            if (m_PhysicsManager->GetTriangulator ().Validate (pointsCopy) == 0)
            {
                m_PhysicsManager->GetTriangulator ().ProcessVertices (&pointsCopy, result);

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
