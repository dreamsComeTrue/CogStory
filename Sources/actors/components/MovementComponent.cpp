// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "MovementComponent.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    std::string MovementComponent::TypeName = "MovementComponent";

    //--------------------------------------------------------------------------------------------------
    //--------------------------------------------------------------------------------------------------

    MovementComponent::MovementComponent (Actor* owner) :
        Component (owner),
        m_MoveType (MoveHorizontal),
        m_Speed (4.f)
    {
        SetMoveExtents ({-50.f, -50.f}, {50.f, 50.f});
    }

    //--------------------------------------------------------------------------------------------------

    void MovementComponent::SetMovementType (MovementType type) 
    { 
        m_MoveType = type; 

        ComputerTargetPos ();
    }

    //--------------------------------------------------------------------------------------------------

    void MovementComponent::SetMoveExtents (Point min, Point max)
    {
        Point ownerPos = m_Actor->Bounds.Pos;
        m_MinExtent = ownerPos + min;
        m_MaxExtent = ownerPos + max;

        ComputerTargetPos ();
    }

    //--------------------------------------------------------------------------------------------------

    bool MovementComponent::Update (float deltaTime)
    {
        Point deltaMove;
        Point currentPos = m_Actor->Bounds.Pos;

//            Log ("%f %f === %f %f\n", currentPos.X, currentPos.Y, m_TargetPos.X, m_TargetPos.Y);
        if ((int)currentPos.X != (int)m_TargetPos.X || (int)currentPos.Y != (int)m_TargetPos.Y)
        {
            switch (m_MoveType)
            {
                case MoveHorizontal:
                {
                    if (m_TargetPos.X >= currentPos.X)
                    {
                        deltaMove.X += m_Speed * deltaTime;
                    }
                    else
                    {
                        deltaMove.X -= m_Speed * deltaTime;
                    }
                }
                break;

                case MoveVertical:
                    deltaMove.Y += m_Speed * deltaTime;
                    break;

                case MoveWander:
                    deltaMove.X += m_Speed * deltaTime;
                    break;
            }

            m_Actor->Move (deltaMove * m_Speed);
        }
        else
        {
            ComputerTargetPos ();
        }

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    bool MovementComponent::Render (float deltaTime)
    {
        return true;
    }

    //--------------------------------------------------------------------------------------------------

    void MovementComponent::ComputerTargetPos ()
    {
        Point ownerPos = m_Actor->Bounds.Pos;

        switch (m_MoveType)
        {
            case MoveHorizontal:
                if ((int)ownerPos.X >= (int)m_MaxExtent.X)
                {
                    m_TargetPos.X = m_MinExtent.X;
                }
                else
                {
                    m_TargetPos.X = m_MaxExtent.X;
                }

                m_TargetPos.Y = ownerPos.Y;
                break;

            case MoveVertical:
                m_TargetPos.X = ownerPos.X;
                m_TargetPos.Y = m_MaxExtent.Y;
                break;

            case MoveWander:
                m_TargetPos.X = m_MaxExtent.X;
                m_TargetPos.Y = ownerPos.Y;
                break;
        }
    }

    //--------------------------------------------------------------------------------------------------
}


