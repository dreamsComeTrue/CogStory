// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __MOVEMENT_COMPONENT_H__
#define __MOVEMENT_COMPONENT_H__

#include "Component.h"

namespace aga
{
    enum MovementType
    {
        MoveHorizontal,
        MoveVertical,
        MovePoints,
        MoveWander,
    };

    class MovementComponent : public Component
    {
    public:
        static std::string TypeName;

    public:
        MovementComponent (Actor* owner);

        virtual bool Update (float deltaTime) override;

        virtual bool Render (float deltaTime) override;

        void SetMovementType (MovementType type); 
        MovementType GetMovementType () const { return m_MoveType; }

        void SetMoveExtents (Point min, Point max);

        void SetWaitTimeBounds (float minWaitSeconds, float maxWaitSeconds) 
        { m_WaitTimeBounds = {minWaitSeconds, maxWaitSeconds};}

        void SetWaitLikelihood (float percentage) { m_WaitLikelihood = percentage; }

        void SetSpeed (float speed) { m_Speed = speed; }
        float GetSpeed () const { return m_Speed; }

        void SetWalkPoints (CScriptArray* array);
        void SetWalkPoints (std::vector<Point> points) { m_Points = points; }

        virtual std::string GetTypeName () override { return TypeName; }

    private:
        void ComputerTargetPos ();
        void ComputeMovePoints ();
        void ComputeMoveWander ();

    private:
        MovementType m_MoveType;
        Point m_MinExtent; 
        Point m_MaxExtent; 
        Point m_InitialPos;
        Point m_StartPos;
        Point m_TargetPos;
        float m_MaxTargetTime;
        float m_CurrentTargetTime;
        float m_Speed;

        std::vector<Point> m_Points;
        int m_CurrentPointIndex;
        bool m_PointsMovingForward;

        Point m_WaitTimeBounds;
        float m_WaitLikelihood;
        float m_WaitTimeElapsed;
        float m_MaxWaitTime;
    };
}

#endif //   __MOVEMENT_COMPONENT_H__

