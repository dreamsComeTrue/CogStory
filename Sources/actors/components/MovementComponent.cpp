// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "MovementComponent.h"
#include "MainLoop.h"
#include "Scene.h"
#include "SceneManager.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    std::string MovementComponent::TypeName = "MovementComponent";

    //--------------------------------------------------------------------------------------------------
    //--------------------------------------------------------------------------------------------------

    MovementComponent::MovementComponent (Actor* owner)
        : Component (owner)
        , m_MoveType (MoveWander)
        , m_Speed (150.f)
        , m_WaitTimeBounds (1.f, 4.f)
        , m_WaitLikelihood (0.001f)
        , m_WaitTimeElapsed (0.f)
        , m_MaxWaitTime (0.f)
        , m_MaxTargetTime (0.f)
        , m_CurrentTargetTime (0.f)
        , m_CurrentPointIndex (0)
        , m_PointsMovingForward (true)
        , m_ScriptMoveCallback (nullptr)
    {
        m_InitialPos = owner->Bounds.Pos;

        SetMoveExtents ({-50.f, -50.f}, {50.f, 50.f});

        m_Actor->AddCollisionCallback ([&](Collidable* other) { ComputeTargetPos (); });
    }

    //--------------------------------------------------------------------------------------------------

    void MovementComponent::SetMovementType (MovementType type)
    {
        m_MoveType = type;

        ComputeTargetPos ();
    }

    //--------------------------------------------------------------------------------------------------

    void MovementComponent::SetMoveExtents (Point min, Point max)
    {
        m_MinExtent = min;
        m_MaxExtent = max;

        ComputeTargetPos ();
    }

    //--------------------------------------------------------------------------------------------------

    void MovementComponent::SetStartPos (Point pos)
    {
        m_StartPos = pos;

        ComputeTargetPos ();
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

            m_Actor->ChooseStandAnimation (m_LastAngle);
        }
        else
        {
            bool xEqual = m_TargetPos.X > m_StartPos.X ? currentPos.X >= m_TargetPos.X : currentPos.X <= m_TargetPos.X;
            bool yEqual = m_TargetPos.Y > m_StartPos.Y ? currentPos.Y >= m_TargetPos.Y : currentPos.Y <= m_TargetPos.Y;

            if (!xEqual || !yEqual)
            {
                m_CurrentTargetTime += m_Speed * deltaTime;

                Point newPos = Lerp (m_StartPos, m_TargetPos, m_CurrentTargetTime / m_MaxTargetTime);
                Point deltaPos = newPos - currentPos;

                std::vector<Entity*> entites
                    = m_Actor->GetSceneManager ()->GetActiveScene ()->RecomputeVisibleEntities (true);
                for (Entity* ent : entites)
                {
                    Collidable* collidable = (Actor*)ent;
                    Point collisionDelta;

                    if (ent != m_Actor && m_Actor->IsCollidingWith (collidable, deltaPos, std::move (collisionDelta)))
                    {
                        float positiveMoveBoundary = 1.5f;
                        float negativeMoveBoundary = -0.5f;

                        if (collisionDelta.X < negativeMoveBoundary || collisionDelta.X > positiveMoveBoundary)
                        {
                            deltaPos.X = deltaPos.X + collisionDelta.X;
                        }

                        if (collisionDelta.Y < negativeMoveBoundary || collisionDelta.Y > positiveMoveBoundary)
                        {
                            deltaPos.Y = deltaPos.Y + collisionDelta.Y;
                        }
                    }
                }

                if (!((AreSame (deltaPos.X, 0) && AreSame (deltaPos.Y, 0))))
                {
                    m_LastAngle = ToPositiveAngle (RadiansToDegrees (std::atan2 (deltaPos.Y, deltaPos.X)));
                    m_Actor->ChooseWalkAnimation (m_LastAngle);

                    m_Actor->Move (deltaPos);

                    if (m_ScriptMoveCallback)
                    {
                        CallScriptMoveCallback (m_Actor->GetPosition ());
                    }
                }
            }
            else
            {
                ComputeTargetPos ();
            }
        }

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    void MovementComponent::CallScriptMoveCallback (Point newPos)
    {
        asIScriptContext* ctx = m_Actor->GetSceneManager ()->GetMainLoop ()->GetScriptManager ().GetContext ();
        ctx->Prepare (m_ScriptMoveCallback);
        ctx->SetArgObject (0, &newPos);

        int r = ctx->Execute ();

        asDWORD ret = 0;
        if (r == asEXECUTION_FINISHED)
        {
        }

        ctx->Unprepare ();
        ctx->GetEngine ()->ReturnContext (ctx);
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

    void MovementComponent::ComputeTargetPos ()
    {
        Point ownerPos = m_Actor->Bounds.Pos;
        m_TargetPos = ownerPos;
        m_StartPos = ownerPos;

        float randValue = RandInRange (-1.f, 1.f);

        switch (m_MoveType)
        {
        case MoveHorizontal:
        {
            m_TargetPos.X
                = m_InitialPos.X + (randValue < 0.f ? std::abs (randValue) * m_MinExtent.X : randValue * m_MaxExtent.X);
            break;
        }

        case MoveVertical:
        {
            m_TargetPos.Y
                = m_InitialPos.Y + (randValue < 0.f ? std::abs (randValue) * m_MinExtent.Y : randValue * m_MaxExtent.Y);
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
        if (m_CurrentPointIndex >= 0 && m_CurrentPointIndex < m_Points.size ())
        {
            m_TargetPos = m_Points[m_CurrentPointIndex];
        }

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
    }

    //--------------------------------------------------------------------------------------------------

    void MovementComponent::ComputeMoveWander ()
    {
        float randValue = RandInRange (-1.f, 1.f);

        if (RandBool ())
        {
            m_TargetPos.X
                = m_InitialPos.X + (randValue < 0.f ? std::abs (randValue) * m_MinExtent.X : randValue * m_MaxExtent.X);
        }
        else
        {
            m_TargetPos.Y
                = m_InitialPos.Y + (randValue < 0.f ? std::abs (randValue) * m_MinExtent.Y : randValue * m_MaxExtent.Y);
        }
    }

    //--------------------------------------------------------------------------------------------------

    void MovementComponent::SetWalkPoints (FlagPoint* flagPoint)
    {
        if (flagPoint)
        {
            m_Points.clear ();

            std::vector<std::string> visited;

            m_Points.push_back (flagPoint->Pos);
            visited.push_back (flagPoint->Name);

            FlagPoint* next = flagPoint->Connections[0];

            while (next != nullptr)
            {
                m_Points.push_back (next->Pos);
                visited.push_back (next->Name);

                if (!next->Connections.empty ())
                {
                    bool found = false;

                    for (int i = 0; i < next->Connections.size (); ++i)
                    {
                        if (std::find (visited.begin (), visited.end (), next->Connections[i]->Name) == visited.end ())
                        {
                            next = next->Connections[i];
                            found = true;
                            break;
                        }
                    }

                    if (!found)
                    {
                        break;
                    }
                }
                else
                {
                    break;
                }
            }
        }

        ComputeClosestWalkPoint ();
    }

    //--------------------------------------------------------------------------------------------------

    void MovementComponent::SetWalkPoints (CScriptArray* points)
    {
        m_Points.clear ();

        for (asUINT i = 0; i < points->GetSize (); ++i)
        {
            m_Points.push_back (*((Point*)points->At (i)));
        }

        ComputeClosestWalkPoint ();
    }

    //--------------------------------------------------------------------------------------------------

    void MovementComponent::SetWalkPoints (std::vector<Point> points)
    {
        m_Points = points;

        ComputeClosestWalkPoint ();
    }

    //--------------------------------------------------------------------------------------------------

    void MovementComponent::ComputeClosestWalkPoint ()
    {
        float minDistance = MAXFLOAT;
        int closestIndex = 0;
        Point actorPos = m_Actor->GetPosition ();

        for (int i = 0; i < m_Points.size (); ++i)
        {
            float distance = Distance (actorPos, m_Points[i]);

            if (distance < minDistance)
            {
                minDistance = distance;
                closestIndex = i;
            }
        }

        m_CurrentPointIndex = closestIndex;

        ComputeTargetPos ();
    }

    //--------------------------------------------------------------------------------------------------
}
