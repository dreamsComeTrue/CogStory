// Copyright 2017-2019 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "Collidable.h"
#include "Actor.h"
#include "MainLoop.h"

namespace aga
{
	//--------------------------------------------------------------------------------------------------

	Collidable::Collidable ()
		: m_PhysicsManager (nullptr)
		, m_CollisionEnabled (false)
		, m_Collidable (true)
	{
	}

	//--------------------------------------------------------------------------------------------------

	Collidable::Collidable (PhysicsManager* physicsManager)
		: m_PhysicsManager (physicsManager)
		, m_CollisionEnabled (false)
		, m_Collidable (true)
	{
	}

	//--------------------------------------------------------------------------------------------------

	Collidable::Collidable (const Collidable& rhs)
	{
		this->m_PhysicsManager = rhs.m_PhysicsManager;
		this->m_CollisionEnabled = rhs.m_CollisionEnabled;
		this->m_Collidable = rhs.m_Collidable;
		this->PhysPoints = rhs.PhysPoints;
		this->m_Collisions.clear ();
		this->m_Callbacks = rhs.m_Callbacks;
		this->m_PhysPolygons = rhs.m_PhysPolygons;
		this->m_Offset = rhs.m_Offset;
	}

	//--------------------------------------------------------------------------------------------------

	void Collidable::AddPhysOffset (const Point& offset)
	{
		SetPhysOffset (m_Offset.X + offset.X, m_Offset.Y + offset.Y);
	}

	//--------------------------------------------------------------------------------------------------

	void Collidable::SetPhysOffset (const Point& offset) { SetPhysOffset (offset.X, offset.Y); }

	//--------------------------------------------------------------------------------------------------

	void Collidable::SetPhysOffset (float offsetX, float offsetY)
	{
		m_Offset.Set (offsetX, offsetY);
		UpdatePhysPolygon ();
	}

	//--------------------------------------------------------------------------------------------------

	void Collidable::DrawPhysBody ()
	{
		for (size_t i = 0; i < m_PhysPolygons.size (); ++i)
		{
			std::vector<float> out;
			std::vector<Point>& points = m_PhysPolygons[i].Points;

			for (size_t j = 0; j < points.size (); ++j)
			{
				out.push_back (points[j].X);
				out.push_back (points[j].Y);
			}

			al_draw_polygon (out.data (), static_cast<int> (out.size ()) / 2, 0, COLOR_GREEN, 1, 0);
		}

		// for (int i = 0; i < PhysPoints.size (); ++i)
		//{
		// std::vector<float> out;

		// for (int j = 0; j < PhysPoints[i].size (); ++j)
		//{
		// float xPoint = m_Offset.X + PhysPoints[i][j].X;
		// float yPoint = m_Offset.Y + PhysPoints[i][j].Y;

		// out.push_back (xPoint);
		// out.push_back (yPoint);
		//}

		// al_draw_polygon (out.data (), out.size () / 2, 0, COLOR_GREEN, 1, 0);
		//}
	}

	//--------------------------------------------------------------------------------------------------

	Polygon& Collidable::GetPhysPolygon (size_t index) { return m_PhysPolygons[index]; }

	//--------------------------------------------------------------------------------------------------

	size_t Collidable::GetPhysPolygonsCount () const { return m_PhysPolygons.size (); }

	//--------------------------------------------------------------------------------------------------

	void Collidable::UpdatePhysPolygon ()
	{
		m_PhysPolygons.clear ();

		int counter = 0;
		for (size_t i = 0; i < PhysPoints.size (); ++i)
		{
			std::vector<Point> pointsCopy = PhysPoints[i];

			int validate = m_PhysicsManager->GetTriangulator ().Validate (pointsCopy);

			if (validate == 2)
			{
				std::reverse (pointsCopy.begin (), pointsCopy.end ());
			}

			if (m_PhysicsManager->GetTriangulator ().Validate (pointsCopy) == 0)
			{
				std::vector<std::vector<Point>> result;
				m_PhysicsManager->GetTriangulator ().ProcessVertices (&pointsCopy, result);

				for (size_t j = 0; j < result.size (); ++j)
				{
					Polygon poly = Polygon ();
					poly.Points = result[j];
					poly.Offset (m_Offset);
					poly.BuildEdges ();

					m_PhysPolygons.push_back (poly);

					++counter;
				}
			}
		}
	}

	//--------------------------------------------------------------------------------------------------
	
	void Collidable::ClearPhysPolygons () { m_PhysPolygons.clear (); }

	//--------------------------------------------------------------------------------------------------

	void Collidable::SetCollisionEnabled (bool enabled) { m_CollisionEnabled = enabled; }

	//--------------------------------------------------------------------------------------------------

	bool Collidable::IsCollisionEnabled () const { return m_CollisionEnabled; }

	//--------------------------------------------------------------------------------------------------

	void Collidable::SetCollidable (bool enabled) { m_Collidable = enabled; }

	//--------------------------------------------------------------------------------------------------

	bool Collidable::IsCollidable () const { return m_Collidable; }

	//--------------------------------------------------------------------------------------------------

	void Collidable::BuildEdges ()
	{
		for (Polygon& poly : m_PhysPolygons)
		{
			poly.BuildEdges ();
		}
	}

	//--------------------------------------------------------------------------------------------------

	bool Collidable::IsCollidingWith (Collidable* other, Point velocity, Point&& offset)
	{
		if (m_CollisionEnabled && !PhysPoints.empty () && !other->PhysPoints.empty ())
		{
			Point combinedOffset = Point::ZERO_POINT;

			for (size_t i = 0; i < GetPhysPolygonsCount (); ++i)
			{
				Polygon& myPolygon = GetPhysPolygon (i);

				if (!myPolygon.Points.empty ())
				{
					for (size_t j = 0; j < other->GetPhysPolygonsCount (); ++j)
					{
						if (other->m_Collidable && !other->GetPhysPolygon (j).Points.empty ())
						{
							PolygonCollisionResult r = m_PhysicsManager->PolygonCollision (
								myPolygon, other->GetPhysPolygon (j), {velocity.X, velocity.Y});

							bool found = false;

							for (Collidable* saved : m_Collisions)
							{
								if (saved == other)
								{
									found = true;
									break;
								}
							}

							if (r.WillIntersect)
							{
								combinedOffset += r.MinimumTranslationVector;

								if (!found)
								{
									m_Collisions.push_back (other);

									CollisionEvent (other);
									other->CollisionEvent (this);

									RunCallbacks (other);
									other->RunCallbacks (this);
								}
							}
							else
							{
								for (std::vector<Collidable*>::iterator it = m_Collisions.begin ();
									 it != m_Collisions.end (); ++it)
								{
									if (*it == other)
									{
										m_Collisions.erase (it);
										break;
									}
								}
							}
						}
					}
				}
			}

			if (!AreSame (combinedOffset, Point::ZERO_POINT))
			{
				offset = combinedOffset;

				return true;
			}
		}

		return false;
	}

	//--------------------------------------------------------------------------------------------------

	void Collidable::RunCallbacks (Collidable* other)
	{
		for (CollisionCallback& callback : m_Callbacks)
		{
			if (callback.Func)
			{
				callback.Func (other);
			}

			if (callback.ScriptFunc)
			{
				Actor* actor = dynamic_cast<Actor*> (other);

				asIScriptContext* ctx = m_PhysicsManager->GetMainLoop ()->GetScriptManager ().GetContext ();
				ctx->Prepare (callback.ScriptFunc);
				ctx->SetArgObject (0, actor);
				ctx->Execute ();
				ctx->Unprepare ();
				ctx->GetEngine ()->ReturnContext (ctx);
			}
		}
	}

	//--------------------------------------------------------------------------------------------------

	void Collidable::AddCollisionCallback (std::function<void(Collidable* other)> func)
	{
		CollisionCallback callback;
		callback.Func = func;

		m_Callbacks.push_back (callback);
	}

	//--------------------------------------------------------------------------------------------------

	void Collidable::AddCollisionCallback (asIScriptFunction* func)
	{
		CollisionCallback callback;
		callback.ScriptFunc = func;

		m_Callbacks.push_back (callback);
	}

	//--------------------------------------------------------------------------------------------------

	void Collidable::RemoveCollisionCallback (std::function<void(Collidable* other)>* func)
	{
		for (std::vector<CollisionCallback>::iterator it = m_Callbacks.begin (); it != m_Callbacks.end (); ++it)
		{
			if (&(*it).Func == func)
			{
				m_Callbacks.erase (it);
				break;
			}
		}
	}

	//--------------------------------------------------------------------------------------------------

	void Collidable::ClearCollisionCallbacks () { m_Callbacks.clear (); }

	//--------------------------------------------------------------------------------------------------

	void Collidable::CollisionEvent (Collidable* /*other*/) {}

	//--------------------------------------------------------------------------------------------------
}
