// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __COLLIDABLE_H__
#define __COLLIDABLE_H__

#include "Common.h"

namespace aga
{
    class PhysicsManager;

    struct CollisionCallback
    {
        std::function<void(class Collidable* other)> Func;
        asIScriptFunction* ScriptFunc = nullptr;
    };

    class Collidable
    {
    public:
        Collidable (PhysicsManager* physicsManager);
        virtual ~Collidable () {}

        void AddPhysOffset (const Point& offset);
        void SetPhysOffset (const Point& offset);
        void SetPhysOffset (float offsetX, float offsetY);
        void DrawPhysBody ();
        Polygon& GetPhysPolygon (size_t index);
        size_t GetPhysPolygonsCount () const;
        void UpdatePhysPolygon ();

        void SetCollisionEnabled (bool enabled);
        bool IsCollisionEnabled () const;

        void SetCollidable (bool enabled);
        bool IsCollidable () const;

        bool IsCollidingWith (Collidable* other, Point velocity, Point&& offset);

        void AddCollisionCallback (std::function<void(Collidable* other)> func);
        virtual void AddCollisionCallback (asIScriptFunction* func);
        void RemoveCollisionCallback (std::function<void(Collidable* other)>* func);
        void ClearCollisionCallbacks ();

    protected:
        virtual void CollisionEvent (Collidable* other);

    private:
        void BuildEdges ();
        void RunCallbacks (Collidable* other);

    public:
        std::vector<std::vector<Point>> PhysPoints;

    private:
        std::vector<Collidable*> m_Collisions;
        std::vector<CollisionCallback> m_Callbacks;

        PhysicsManager* m_PhysicsManager;
        std::vector<Polygon> m_PhysPolygons;
        Point m_Offset;
        bool m_CollisionEnabled;
        bool m_Collidable;
    };
}

#endif //   __COLLIDABLE_H__
