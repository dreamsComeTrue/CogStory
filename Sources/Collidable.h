// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __COLLIDABLE_H__
#define __COLLIDABLE_H__

#include "Common.h"

namespace aga
{
    class PhysicsManager;

    class Collidable
    {
    public:
        Collidable (PhysicsManager* physicsManager);

        void AddPhysOffset (const Point& offset);
        void SetPhysOffset (const Point& offset);
        void SetPhysOffset (float offsetX, float offsetY);
        void DrawPhysBody ();
        Polygon& GetPhysPolygon (int index);
        size_t GetPhysPolygonsCount () const;
        void UpdatePhysPolygon ();

        bool IsCollidingWith (Collidable* other, Point velocity, Point&& offset);

    protected:
        virtual void CollisionEvent (Collidable* other) {}

    private:
        void BuildEdges ();

    public:
        std::vector<std::vector<Point>> PhysPoints;

    private:
        std::vector<Collidable*> m_Collisions;

        PhysicsManager* m_PhysicsManager;
        std::vector<Polygon> m_PhysPolygons;
        Point m_Offset;
    };
}

#endif //   __COLLIDABLE_H__
