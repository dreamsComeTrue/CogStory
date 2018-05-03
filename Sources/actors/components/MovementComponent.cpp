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
        m_MoveType (MoveWander),
        m_Speed (30.f),
        m_MovingHorizontal (false),
        m_WaitTimeBounds (1.f, 4.f),
        m_WaitLikelihood (0.001f),
        m_WaitTimeElapsed (0.f),
        m_MaxWaitTime (0.f)
    {
        m_StartPos = owner->Bounds.Pos;

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
        m_MinExtent = min;
        m_MaxExtent = max;

        ComputerTargetPos ();
    }

    //--------------------------------------------------------------------------------------------------

    bool MovementComponent::Update (float deltaTime)
    {
        if (!m_IsEnabled)
        {
            return true;
        }

        Point currentPos = m_Actor->Bounds.Pos;

        if (AreSame (m_MaxWaitTime, 0.0f) && RandZeroToOne () < m_WaitLikelihood)
        {
            m_MaxWaitTime = RandInRange (m_WaitTimeBounds.X, m_WaitTimeBounds.Y);
            m_WaitTimeElapsed = 0.f;
        }

        if (m_WaitTimeElapsed >= m_MaxWaitTime)
        {
            m_MaxWaitTime = 0.0f;
        }

        if (m_MaxWaitTime > 0.0f)
        {
            m_WaitTimeElapsed += deltaTime;
        }
        else
        {
            if (!AreSame (currentPos, m_TargetPos, {0.5f, 0.5f}))
            {
                Point deltaMove;

                if (m_MovingHorizontal)
                {
                    deltaMove.X = m_Speed * deltaTime;
                }
                else
                {
                    deltaMove.Y = m_Speed * deltaTime;
                }

                if (currentPos.X >= m_TargetPos.X)
                {
                    deltaMove.X = -deltaMove.X;
                }

                if (currentPos.Y >= m_TargetPos.Y)
                {
                    deltaMove.Y = -deltaMove.Y;
                }

                m_Actor->Move (deltaMove);
            }
            else
            {
                ComputerTargetPos ();
            }
        }

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    bool MovementComponent::Render (float deltaTime)
    {
        if (!m_IsEnabled)
        {
            return true;
        }

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    void MovementComponent::ComputerTargetPos ()
    {
        Point ownerPos = m_Actor->Bounds.Pos;
        m_TargetPos = ownerPos;

        float randValue = RandInRange (-1.f, 1.f);

        switch (m_MoveType)
        {
            case MoveHorizontal:
            {
                m_TargetPos.X = m_StartPos.X + (randValue < 0.f ? std::abs (randValue) * m_MinExtent.X : 
                              randValue * m_MaxExtent.X);
                m_MovingHorizontal = true;

                break;
            }

            case MoveVertical:
            {
                m_TargetPos.Y = m_StartPos.Y + (randValue < 0.f ? std::abs (randValue) * m_MinExtent.Y : 
                              randValue * m_MaxExtent.Y);
                m_MovingHorizontal = false;

                break;
            }

            case MoveWander:
            {
                if (RandBool ())
                {
                    m_TargetPos.X = m_StartPos.X + (randValue < 0.f ? std::abs (randValue) * m_MinExtent.X : 
                        randValue * m_MaxExtent.X);
                    m_MovingHorizontal = true;
                }
                else
                {
                    m_TargetPos.Y = m_StartPos.Y + (randValue < 0.f ? std::abs (randValue) * m_MinExtent.Y : 
                        randValue * m_MaxExtent.Y);
                    m_MovingHorizontal = false;
                }

                break;
            }
        }
    }

    //--------------------------------------------------------------------------------------------------
}


