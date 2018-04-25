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
        , m_CollisionEnabled (true)
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

    void Collidable::DrawPhysBody ()
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

            al_draw_polygon (out.data (), (int)PhysPoints[i].size (), 0, COLOR_GREEN, 1, 0);
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

    void Collidable::SetCollisionEnabled (bool enabled) { m_CollisionEnabled = enabled; }

    //--------------------------------------------------------------------------------------------------

    bool Collidable::IsCollisionEnabled () const { return m_CollisionEnabled; }

    //--------------------------------------------------------------------------------------------------

    void Collidable::BuildEdges ()
    {
        for (int i = 0; i < m_PhysPolygons.size (); ++i)
        {
            m_PhysPolygons[i].BuildEdges ();
        }
    }

    //--------------------------------------------------------------------------------------------------

    bool Collidable::IsCollidingWith (Collidable* other, Point velocity, Point&& offset)
    {
        if (m_CollisionEnabled && !PhysPoints.empty () && !other->PhysPoints.empty ())
        {
            for (int i = 0; i < GetPhysPolygonsCount (); ++i)
            {
                Polygon& myPolygon = GetPhysPolygon (i);

                if (!myPolygon.Points.empty ())
                {
                    for (int j = 0; j < other->GetPhysPolygonsCount (); ++j)
                    {
                        if (!other->GetPhysPolygon (j).Points.empty ())
                        {
                            PolygonCollisionResult r = m_PhysicsManager->PolygonCollision (
                                myPolygon, other->GetPhysPolygon (j), { velocity.X, velocity.Y });

                            bool found = false;

                            for (Collidable* saved : m_Collisions)
                            {
                                if (saved == other)
                                {
                                    found = true;
                                    break;
                                }
                            }

                            if (r.WillIntersect)
                            {
                                offset = r.MinimumTranslationVector;

                                if (!found)
                                {
                                    m_Collisions.push_back (other);

                                    CollisionEvent (other);
                                }

                                return true;
                            }
                            else
                            {
                                for (std::vector<Collidable*>::iterator it = m_Collisions.begin ();
                                     it != m_Collisions.end (); ++it)
                                {
                                    if (*it == other)
                                    {
                                        m_Collisions.erase (it);
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        return false;
    }

    //--------------------------------------------------------------------------------------------------
}
