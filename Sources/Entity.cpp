// Copyright 2017-2019 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "Entity.h"
#include "MainLoop.h"
#include "Player.h"
#include "Scene.h"

namespace aga
{
	//--------------------------------------------------------------------------------------------------

	Entity::Entity () {}

	//--------------------------------------------------------------------------------------------------

	Entity::Entity (SceneManager* sceneManager)
		: m_SceneManager (sceneManager)
		, m_CheckOverlap (false)
	{
	}

	//--------------------------------------------------------------------------------------------------

	Entity::Entity (const Entity& rhs)
		: Transformable (rhs)
	{
		this->ID = rhs.ID;
		this->Name = rhs.Name;
		this->ZOrder = rhs.ZOrder;
		this->RenderID = rhs.RenderID;
		this->BlueprintID = rhs.BlueprintID;
		this->m_SceneManager = rhs.m_SceneManager;
		this->m_OverlapedEntities.clear ();
		this->m_OverlapCallbacks = rhs.m_OverlapCallbacks;
		this->m_CheckOverlap = rhs.m_CheckOverlap;
	}

	//--------------------------------------------------------------------------------------------------

	void Entity::SetCheckOverlap (bool check) { m_CheckOverlap = check; }

	//--------------------------------------------------------------------------------------------------

	bool Entity::IsCheckOverlap () { return m_CheckOverlap; }

	//--------------------------------------------------------------------------------------------------

	SceneManager* Entity::GetSceneManager () { return m_SceneManager; }

	//--------------------------------------------------------------------------------------------------

	void Entity::CheckOverlap ()
	{
		if (!m_CheckOverlap)
		{
			return;
		}

		Camera& camera = m_SceneManager->GetCamera ();

		Rect myBounds = camera.GetRenderBounds (this);
		std::vector<Entity*> visibleEntites = m_SceneManager->GetActiveScene ()->RecomputeVisibleEntities (true);

		//  Special-case entity :)
		visibleEntites.push_back (m_SceneManager->GetPlayer ());

		for (Entity* ent : visibleEntites)
		{
			if (ent != this && ent->IsCheckOverlap ())
			{
				Rect otherBounds = camera.GetRenderBounds (ent);

				if (Intersect (myBounds, otherBounds))
				{
					bool found = false;

					for (Entity* saved : m_OverlapedEntities)
					{
						if (saved == ent)
						{
							found = true;
							break;
						}
					}

					CallOverlappingCallbacks (this, ent);
					CallOverlappingCallbacks (ent, this);

					if (!found)
					{
						m_OverlapedEntities.push_back (ent);

						BeginOverlap (ent);

						CallBeginOverlapCallbacks (this, ent);
						CallBeginOverlapCallbacks (ent, this);
					}
				}
				else
				{
					for (std::vector<Entity*>::iterator it = m_OverlapedEntities.begin ();
						 it != m_OverlapedEntities.end (); ++it)
					{
						if (*it == ent)
						{
							EndOverlap (ent);

							CallEndOverlapCallbacks (this, ent);
							CallEndOverlapCallbacks (ent, this);

							m_OverlapedEntities.erase (it);
							break;
						}
					}
				}
			}
		}
	}

	//--------------------------------------------------------------------------------------------------

	bool Entity::IsOverlaping (Entity* entity)
	{
		for (Entity* ent : m_OverlapedEntities)
		{
			if (ent == entity)
			{
				return true;
			}
		}

		return false;
	}

	//--------------------------------------------------------------------------------------------------

	float Entity::GetAngleWith (Entity* ent)
	{
		Point thisHalf = this->Bounds.GetHalfSize ();
		Point entHalf = ent->Bounds.GetHalfSize ();
		float dx = (this->Bounds.Pos.X + thisHalf.Width) - (ent->Bounds.Pos.X + entHalf.Width);
		float dy = (this->Bounds.Pos.Y + thisHalf.Height) - (ent->Bounds.Pos.Y + entHalf.Height);

		return ToPositiveAngle (RadiansToDegrees (std::atan2 (dy, dx)));
	}

	//--------------------------------------------------------------------------------------------------

	void Entity::AddBeginOverlapCallback (asIScriptFunction* func)
	{
		OverlapCallback callback;
		callback.BeginFunc = func;

		m_OverlapCallbacks.push_back (callback);
	}

	//--------------------------------------------------------------------------------------------------

	void Entity::AddOverlappingCallback (asIScriptFunction* func)
	{
		OverlapCallback callback;
		callback.OverlappingFunc = func;

		m_OverlapCallbacks.push_back (callback);
	}

	//--------------------------------------------------------------------------------------------------

	void Entity::AddEndOverlapCallback (asIScriptFunction* func)
	{
		OverlapCallback callback;
		callback.EndFunc = func;

		m_OverlapCallbacks.push_back (callback);
	}

	//--------------------------------------------------------------------------------------------------

	void Entity::AddOverlapCallbacks (asIScriptFunction* begin, asIScriptFunction* update, asIScriptFunction* end)
	{
		OverlapCallback callback;
		callback.BeginFunc = begin;
		callback.OverlappingFunc = update;
		callback.EndFunc = end;

		m_OverlapCallbacks.push_back (callback);
	}

	//--------------------------------------------------------------------------------------------------

	void Entity::CallBeginOverlapCallbacks (Entity* whom, Entity* target)
	{
		for (OverlapCallback& callback : whom->m_OverlapCallbacks)
		{
			if (callback.BeginFunc)
			{
				m_SceneManager->GetMainLoop ()->GetScriptManager ().RunScriptFunction (callback.BeginFunc, target);
			}
		}
	}

	//--------------------------------------------------------------------------------------------------

	void Entity::CallOverlappingCallbacks (Entity* whom, Entity* target)
	{
		for (OverlapCallback& callback : whom->m_OverlapCallbacks)
		{
			if (callback.OverlappingFunc)
			{
				m_SceneManager->GetMainLoop ()->GetScriptManager ().RunScriptFunction (
					callback.OverlappingFunc, target);
			}
		}
	}

	//--------------------------------------------------------------------------------------------------

	void Entity::CallEndOverlapCallbacks (Entity* whom, Entity* target)
	{
		for (OverlapCallback& callback : whom->m_OverlapCallbacks)
		{
			if (callback.EndFunc)
			{
				m_SceneManager->GetMainLoop ()->GetScriptManager ().RunScriptFunction (callback.EndFunc, target);
			}
		}
	}

	//--------------------------------------------------------------------------------------------------

	std::vector<Entity*> Entity::GetBluprintChildren ()
	{
		std::vector<Actor*>& actors = m_SceneManager->GetActiveScene ()->GetActors ();
		std::vector<Entity*> entities;

		for (Actor* actor : actors)
		{
			if (actor->BlueprintID == this->ID)
			{
				entities.push_back (actor);
			}
		}

		return entities;
	}

	//--------------------------------------------------------------------------------------------------

	int Entity::GetNextID () { return ++GlobalID; }

	//--------------------------------------------------------------------------------------------------

	bool Entity::CompareByZOrder (const Entity* a, const Entity* b) { return a->ZOrder < b->ZOrder; }

	//--------------------------------------------------------------------------------------------------

	void Entity::BeginOverlap (Entity*) {}

	//--------------------------------------------------------------------------------------------------

	void Entity::EndOverlap (Entity*) {}

	//--------------------------------------------------------------------------------------------------
}
