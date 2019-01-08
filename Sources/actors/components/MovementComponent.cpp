// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "MovementComponent.h"
#include "AudioSample.h"
#include "MainLoop.h"
#include "Resources.h"
#include "Scene.h"
#include "Screen.h"

namespace aga
{
	//--------------------------------------------------------------------------------------------------

	std::string MovementComponent::TypeName = "MovementComponent";

	//--------------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------------

	MovementComponent::MovementComponent (Actor* owner)
		: Component (owner)
		, m_MoveType (MoveWander)
		, m_MaxTargetTime (0.f)
		, m_CurrentTargetTime (0.f)
		, m_Speed (150.f)
		, m_CurrentPointIndex (0)
		, m_PointsMovingForward (true)
		, m_WaitTimeBounds (1.f, 4.f)
		, m_WaitLikelihood (0.001f)
		, m_WaitTimeElapsed (0.f)
		, m_MaxWaitTime (0.f)
		, m_ScriptMoveCallback (nullptr)
		, m_SampleCounter (0)
	{
		m_InitialPos = owner->Bounds.Pos;

		SetMoveExtents ({-50.f, -50.f}, {50.f, 50.f});

		m_Actor->AddCollisionCallback ([&](Collidable*) { ComputeTargetPos (); });

		m_FootStepSample = owner->GetSceneManager ()->GetMainLoop ()->GetAudioManager ().LoadSampleFromFile (
			"FOOT_STEP", GetResource (SOUND_FOOT_STEP).Dir + GetResource (SOUND_FOOT_STEP).Name);
		m_FootStepSample->SetVolume (1.0f);
	}

	//--------------------------------------------------------------------------------------------------

	MovementComponent::MovementComponent (const MovementComponent& rhs)
		: Component (rhs.m_Actor)
	{
		this->m_MoveType = rhs.m_MoveType;
		this->m_MinExtent = rhs.m_MinExtent;
		this->m_MaxExtent = rhs.m_MaxExtent;
		this->m_InitialPos = rhs.m_InitialPos;
		this->m_StartPos = rhs.m_StartPos;
		this->m_TargetPos = rhs.m_TargetPos;
		this->m_MaxTargetTime = rhs.m_MaxTargetTime;
		this->m_CurrentTargetTime = rhs.m_CurrentTargetTime;
		this->m_Speed = rhs.m_Speed;
		this->m_Points = rhs.m_Points;
		this->m_CurrentPointIndex = rhs.m_CurrentPointIndex;
		this->m_PointsMovingForward = rhs.m_PointsMovingForward;
		this->m_WaitTimeBounds = rhs.m_WaitTimeBounds;
		this->m_WaitLikelihood = rhs.m_WaitLikelihood;
		this->m_WaitTimeElapsed = rhs.m_WaitTimeElapsed;
		this->m_MaxWaitTime = rhs.m_MaxWaitTime;
		this->m_LastAngle = rhs.m_LastAngle;
		this->m_ScriptMoveCallback = rhs.m_ScriptMoveCallback;
		this->m_SampleCounter = rhs.m_SampleCounter;
		this->m_FootStepSample = rhs.m_FootStepSample;
	}

	//--------------------------------------------------------------------------------------------------

	MovementComponent* MovementComponent::Clone () const { return new MovementComponent (*this); }

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
					Collidable* collidable = static_cast<Actor*> (ent);
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

					MainLoop* mainLoop = m_Actor->GetSceneManager ()->GetMainLoop ();
					Actor* followActor = mainLoop->GetSceneManager ().GetCamera ().GetFollowActor ();

					Point screenSize = mainLoop->GetScreen ()->GetWindowSize ();
					Point followActorPos = followActor->GetPosition () + followActor->Bounds.GetHalfSize ();
					Point thisPos = m_Actor->GetPosition ();

					if (thisPos.X < followActorPos.X)
					{
						thisPos.X += m_Actor->Bounds.GetSize ().Width;
					}

					if (thisPos.Y < followActorPos.Y)
					{
						thisPos.Y += m_Actor->Bounds.GetSize ().Height;
					}

					Rect followActorBounds
						= Rect (followActorPos - screenSize * 0.5f * 0.5f, followActorPos + screenSize * 0.5f * 0.5f);

					m_SampleCounter += deltaTime;

					if (InsideRect (thisPos, followActorBounds) && m_SampleCounter > 0.2f
						&& !mainLoop->GetAudioManager ().IsGloballyPlaying (m_FootStepSample->GetName ()))
					{
						float distanceToFollowActor = (followActorPos - thisPos).Magnitude ();
						float volume = 1.0f;

						// m_FootStepSample->SetVolume (volume);
						m_FootStepSample->Play ();

						m_SampleCounter = 0;
					}

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
		ctx->Execute ();
		ctx->Unprepare ();
		ctx->GetEngine ()->ReturnContext (ctx);
	}

	//--------------------------------------------------------------------------------------------------

	bool MovementComponent::Render (float)
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
		if (m_CurrentPointIndex < m_Points.size ())
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

					for (size_t i = 0; i < next->Connections.size (); ++i)
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
			m_Points.push_back (*(static_cast<Point*> (points->At (i))));
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
		double minDistance = 1e100;
		size_t closestIndex = 0;
		Point actorPos = m_Actor->GetPosition ();

		for (size_t i = 0; i < m_Points.size (); ++i)
		{
			double distance = static_cast<double> (Distance (actorPos, m_Points[i]));

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
