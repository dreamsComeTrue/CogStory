// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "PhysicsManager.h"
#include "MainLoop.h"

namespace aga
{
    //---------------------------------------------------------------------------

    PhysicsManager::PhysicsManager (MainLoop* mainLoop)
        : m_MainLoop (mainLoop)
    {
    }

    //--------------------------------------------------------------------------------------------------

    PhysicsManager::~PhysicsManager ()
    {
        if (!IsDestroyed ())
        {
            Destroy ();
        }
    }

    //--------------------------------------------------------------------------------------------------

    bool PhysicsManager::Initialize ()
    {
        Lifecycle::Initialize ();

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    bool PhysicsManager::Destroy () { return Lifecycle::Destroy (); }

    //--------------------------------------------------------------------------------------------------

    PolygonCollisionResult PhysicsManager::PolygonCollision (Polygon& polygonA, Polygon& polygonB, Point velocity)
    {
        PolygonCollisionResult result;
        result.Intersect = true;
        result.WillIntersect = true;

        size_t edgeCountA = polygonA.Edges.size ();
        size_t edgeCountB = polygonB.Edges.size ();
        float minIntervalDistance = std::numeric_limits<float>::max ();
        Point translationAxis;
        Point edge;

        // Loop through all the edges of both polygons
        for (int edgeIndex = 0; edgeIndex < edgeCountA + edgeCountB; edgeIndex++)
        {
            if (edgeIndex < edgeCountA)
            {
                edge = polygonA.Edges[edgeIndex];
            }
            else
            {
                edge = polygonB.Edges[edgeIndex - edgeCountA];
            }

            // ===== 1. Find if the polygons are currently intersecting =====

            // Find the axis perpendicular to the current edge
            Point axis = { -edge.Y, edge.X };
            axis.Normalize ();

            // Find the projection of the polygon on the current axis
            Point mA = ProjectPolygon (axis, polygonA);
            Point mB = ProjectPolygon (axis, polygonB);

            // Check if the polygon projections are currentlty intersecting
            if (IntervalDistance (mA.X, mA.Y, mB.X, mB.Y) > 0)
            {
                result.Intersect = false;
            }

            // ===== 2. Now find if the polygons *will* intersect =====

            // Project the velocity on the current axis
            float velocityProjection = axis.DotProduct (velocity);

            // Get the projection of polygon A during the movement
            if (velocityProjection < 0)
            {
                mA.X += velocityProjection;
            }
            else
            {
                mA.Y += velocityProjection;
            }

            // Do the same test as above for the new projection
            float intervalDistance = IntervalDistance (mA.X, mA.Y, mB.X, mB.Y);
            if (intervalDistance > 0)
            {
                result.WillIntersect = false;
            }

            // If the polygons are not intersecting and won't intersect, exit the loop
            if (!result.Intersect && !result.WillIntersect)
            {
                break;
            }

            // Check if the current interval distance is the minimum one. If so store
            // the interval distance and the current distance.
            // This will be used to calculate the minimum translation vector
            intervalDistance = std::abs (intervalDistance);

            if (intervalDistance < minIntervalDistance)
            {
                minIntervalDistance = intervalDistance;
                translationAxis = axis;

                Point d = polygonA.GetCenter () - polygonB.GetCenter ();

                if (d.DotProduct (translationAxis) < 0)
                {
                    translationAxis = -translationAxis;
                }
            }
        }

        // The minimum translation vector can be used to push the polygons appart.
        // First moves the polygons by their velocity
        // then move polygonA by MinimumTranslationVector.
        if (result.WillIntersect)
        {
            result.MinimumTranslationVector = translationAxis * minIntervalDistance;
        }

        return result;
    }

    //--------------------------------------------------------------------------------------------------

    float PhysicsManager::IntervalDistance (float minA, float maxA, float minB, float maxB)
    {
        if (minA < minB)
        {
            return minB - maxA;
        }
        else
        {
            return minA - maxB;
        }
    }

    //--------------------------------------------------------------------------------------------------

    Point PhysicsManager::ProjectPolygon (Point& axis, Polygon& polygon)
    {
        // To project a point on an axis use the dot product
        float d = axis.DotProduct (polygon.Points[0]);
        float min = d;
        float max = d;

        for (int i = 0; i < polygon.Points.size (); i++)
        {
            d = polygon.Points[i].DotProduct (axis);

            if (d < min)
            {
                min = d;
            }
            else if (d > max)
            {
                max = d;
            }
        }

        return { min, max };
    }

    //--------------------------------------------------------------------------------------------------

    Triangulator& PhysicsManager::GetTriangulator () { return m_Triangulator; }

    //--------------------------------------------------------------------------------------------------

    MainLoop* PhysicsManager::GetMainLoop () { return m_MainLoop; }

    //--------------------------------------------------------------------------------------------------
}
