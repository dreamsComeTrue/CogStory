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

        void AddPhysOffset (const Point& offset);
        void SetPhysOffset (const Point& offset);
        void SetPhysOffset (float offsetX, float offsetY);
        void DrawPhysBody ();
        Polygon& GetPhysPolygon (int index);
        size_t GetPhysPolygonsCount () const;
        void UpdatePhysPolygon ();

        void SetCollisionEnabled (bool enabled) { m_CollisionEnabled = enabled; }
        bool IsCollisionEnabled () const { return m_CollisionEnabled; }

        void SetCollidable (bool enabled) { m_Collidable = enabled; }
        bool IsCollidable () const { return m_Collidable; }

        bool IsCollidingWith (Collidable* other, Point velocity, Point&& offset);

        void AddCollisionCallback (std::function<void(Collidable* other)> func);
        virtual void AddCollisionCallback (asIScriptFunction* func);
        void RemoveCollisionCallback (std::function<void(Collidable* other)>* func);
        void ClearCollisionCallbacks () { m_Callbacks.clear (); }

    protected:
        virtual void CollisionEvent (Collidable* other) {}

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
