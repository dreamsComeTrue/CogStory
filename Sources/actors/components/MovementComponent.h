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

        void SetSpeed (float speed) { m_Speed = speed; }
        float GetSpeed () const { return m_Speed; }

        virtual std::string GetTypeName () override { return TypeName; }

    private:
        void ComputerTargetPos ();

    private:
        MovementType m_MoveType;
        Point m_MinExtent; 
        Point m_MaxExtent; 
        Point m_TargetPos;
        float m_Speed;
    };
}

#endif //   __MOVEMENT_COMPONENT_H__

