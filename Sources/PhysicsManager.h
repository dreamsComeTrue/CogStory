// Copyright 2017-2019 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __PHYSICS_MANAGER_H__
#define __PHYSICS_MANAGER_H__

#include "Common.h"
#include "addons/triangulator/Triangulator.h"

namespace aga
{
    class MainLoop;

    struct PolygonCollisionResult
    {
        bool WillIntersect; // Are the polygons going to intersect forward in time?
        bool Intersect; // Are the polygons currently intersecting
        Point MinimumTranslationVector; // The translation to apply to polygon A to push the polygons appart.
    };

    class PhysicsManager : public Lifecycle
    {
    public:
        PhysicsManager (MainLoop* mainLoop);
        virtual ~PhysicsManager ();
        bool Initialize ();
        bool Destroy ();

        // Check if polygon A is going to collide with polygon B for the given velocity
        PolygonCollisionResult PolygonCollision (Polygon& polygonA, Polygon& polygonB, Point velocity);

        Triangulator& GetTriangulator ();
        MainLoop* GetMainLoop ();

    private:
        // Calculate the distance between [minA, maxA] and [minB, maxB]
        // The distance will be negative if the intervals overlap
        float IntervalDistance (float minA, float maxA, float minB, float maxB);

        // Calculate the projection of a polygon on an axis and returns it as a [min, max] interval
        Point ProjectPolygon (Point& axis, Polygon& polygon);

    private:
        MainLoop* m_MainLoop;
        Triangulator m_Triangulator;
    };
}

#endif //   __PHYSICS_MANAGER_H__
