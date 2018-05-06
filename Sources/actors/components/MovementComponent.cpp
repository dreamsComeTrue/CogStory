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
        m_Speed (150.f),
        m_WaitTimeBounds (1.f, 4.f),
        m_WaitLikelihood (0.001f),
        m_WaitTimeElapsed (0.f),
        m_MaxWaitTime (0.f),
        m_MaxTargetTime (0.f),
        m_CurrentTargetTime (0.f),
        m_CurrentPointIndex (0),
        m_PointsMovingForward (true)
    {
        m_InitialPos = owner->Bounds.Pos;

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
            bool xEqual = m_TargetPos.X > m_StartPos.X ? currentPos.X >= m_TargetPos.X : currentPos.X <= m_TargetPos.X;
            bool yEqual = m_TargetPos.Y > m_StartPos.Y ? currentPos.Y >= m_TargetPos.Y : currentPos.Y <= m_TargetPos.Y;

            if (!xEqual || !yEqual)
            {
                m_CurrentTargetTime += m_Speed * deltaTime;

                Point newPos = Lerp (m_StartPos, m_TargetPos, m_CurrentTargetTime / m_MaxTargetTime);

                m_Actor->Move (newPos - currentPos);
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
        m_StartPos = ownerPos;

        float randValue = RandInRange (-1.f, 1.f);

        switch (m_MoveType)
        {
            case MoveHorizontal:
            {
                m_TargetPos.X = m_InitialPos.X + (randValue < 0.f ? std::abs (randValue) * m_MinExtent.X : 
                              randValue * m_MaxExtent.X);
                break;
            }

            case MoveVertical:
            {
                m_TargetPos.Y = m_InitialPos.Y + (randValue < 0.f ? std::abs (randValue) * m_MinExtent.Y : 
                              randValue * m_MaxExtent.Y);
                break;
            }

            case MovePoints:
            {
                ComputeMovePoints ();

                break;
            }

            case MoveWander:
            {
                ComputeMoveWander ();

                break;
            }
        }

        m_TargetPos -= m_Actor->Bounds.GetHalfSize ();

        m_CurrentTargetTime = 0.f;
        m_MaxTargetTime = Distance (m_StartPos, m_TargetPos) / m_Speed * 1000.f;
    }

    //--------------------------------------------------------------------------------------------------

    void MovementComponent::ComputeMovePoints ()
    {
        int index = m_CurrentPointIndex;

        if (m_PointsMovingForward)
        {
            if (index + 1 >= m_Points.size ())
            {
                m_PointsMovingForward = false;
                --m_CurrentPointIndex;
            }
            else
            {
                ++m_CurrentPointIndex;
            }
        }
        else if (!m_PointsMovingForward)
        {
            if (index - 1 < 0)
            {
                m_PointsMovingForward = true;
                ++m_CurrentPointIndex;
            }
            else
            {
                --m_CurrentPointIndex;
            }
        }

        if (m_CurrentPointIndex >= 0 && m_CurrentPointIndex < m_Points.size ())
        {
            m_TargetPos = m_Points[m_CurrentPointIndex];
        }
    }

    //--------------------------------------------------------------------------------------------------

    void MovementComponent::ComputeMoveWander ()
    {
        float randValue = RandInRange (-1.f, 1.f);

        if (RandBool ())
        {
            m_TargetPos.X = m_InitialPos.X + (randValue < 0.f ? std::abs (randValue) * m_MinExtent.X : 
                    randValue * m_MaxExtent.X);
        }
        else
        {
            m_TargetPos.Y = m_InitialPos.Y + (randValue < 0.f ? std::abs (randValue) * m_MinExtent.Y : 
                    randValue * m_MaxExtent.Y);
        }
    }

    //--------------------------------------------------------------------------------------------------

    void MovementComponent::SetWalkPoints (CScriptArray* array)
    {
        m_Points.clear ();

        for (asUINT i = 0; i < array->GetSize (); ++i)
        {
            m_Points.push_back (*((Point*)array->At (i)));
        }
    }

    //--------------------------------------------------------------------------------------------------
}


