// Copyright 2017-2019 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.
#include "Actor.h"
#include "Atlas.h"
#include "MainLoop.h"
#include "Scene.h"
#include "Screen.h"
#include "actors/components/MovementComponent.h"
#include "actors/components/ParticleEmitterComponent.h"

namespace aga
{
	//--------------------------------------------------------------------------------------------------

	Actor::Actor (SceneManager* sceneManager)
		: Entity (sceneManager)
		, Animable (&sceneManager->GetMainLoop ()->GetAtlasManager ())
		, Scriptable (&sceneManager->GetMainLoop ()->GetScriptManager ())
		, Collidable (&sceneManager->GetMainLoop ()->GetPhysicsManager ())
		, m_IsUpdating (true)
		, m_FocusHeight (100.0f) //  We set it way down for small tiles grid layout
		, m_ActionSpeech ("")
		, m_ActionSpeechHandling (false)
		, m_IsVisible (true)
	{
		ID = Entity::GetNextID ();
	}

	//--------------------------------------------------------------------------------------------------

	Actor::Actor (const Actor& rhs)
		: Entity (rhs)
		, Animable (rhs)
		, Scriptable (rhs)
		, Collidable (rhs)
	{
		this->m_IsUpdating = rhs.m_IsUpdating;
		this->m_OldPosition = rhs.m_OldPosition;
		this->m_FocusHeight = rhs.m_FocusHeight;
		this->m_ActionSpeech = rhs.m_ActionSpeech;
		this->m_ActionSpeechHandling = rhs.m_ActionSpeechHandling;
		this->TemplateBounds = rhs.TemplateBounds;

		std::map<std::string, Component*> rhsComps = rhs.m_Components;

		for (std::map<std::string, Component*>::iterator it = rhsComps.begin (); it != rhsComps.end (); ++it)
		{
			Component* newComponent = it->second->Clone ();
			newComponent->SetActor (this);

			this->AddComponent (it->first, newComponent);
		}
	}

	//--------------------------------------------------------------------------------------------------

	Actor::~Actor ()
	{
		if (!IsDestroyed ())
		{
			Destroy ();
		}
	}

	//--------------------------------------------------------------------------------------------------

	bool Actor::Initialize ()
	{
		if (m_AtlasName != "" && m_AtlasRegionName != "")
		{
			Bounds.Size = m_SceneManager->GetMainLoop ()
							  ->GetAtlasManager ()
							  .GetAtlas (m_AtlasName)
							  ->GetRegion (m_AtlasRegionName)
							  .Bounds.GetSize ();
		}

		return Lifecycle::Initialize ();
	}

	//--------------------------------------------------------------------------------------------------

	void Actor::BeforeEnter () {}

	//--------------------------------------------------------------------------------------------------

	void Actor::AfterLeave () {}

	//--------------------------------------------------------------------------------------------------

	bool Actor::Destroy ()
	{
		for (std::map<std::string, Component*>::iterator it = m_Components.begin (); it != m_Components.end ();)
		{
			SAFE_DELETE (it->second);
			m_Components.erase (it++);
		}

		return Lifecycle::Destroy ();
	}

	//--------------------------------------------------------------------------------------------------

	void Actor::ChooseWalkAnimation (float angleDeg)
	{
		if (GetAnimationsData ().empty ())
		{
			return;
		}

		if (angleDeg > 45 && angleDeg < 135)
		{
			SetCurrentAnimation (ANIM_MOVE_UP_NAME);
		}

		if (angleDeg > 225 && angleDeg < 315)
		{
			SetCurrentAnimation (ANIM_MOVE_DOWN_NAME);
		}

		if (angleDeg >= 135 && angleDeg <= 225)
		{
			SetCurrentAnimation (ANIM_MOVE_LEFT_NAME);
		}

		if (angleDeg <= 45 || angleDeg >= 315)
		{
			SetCurrentAnimation (ANIM_MOVE_RIGHT_NAME);
		}
	}

	//--------------------------------------------------------------------------------------------------

	void Actor::ChooseStandAnimation (float angleDeg)
	{
		if (GetAnimationsData ().empty ())
		{
			return;
		}

		if (angleDeg > 45 && angleDeg < 135)
		{
			SetCurrentAnimation (ANIM_STAND_UP_NAME);
		}

		if (angleDeg > 225 && angleDeg < 315)
		{
			SetCurrentAnimation (ANIM_STAND_DOWN_NAME);
		}

		if (angleDeg >= 135 && angleDeg <= 225)
		{
			SetCurrentAnimation (ANIM_STAND_LEFT_NAME);
		}

		if (angleDeg <= 45 || angleDeg >= 315)
		{
			SetCurrentAnimation (ANIM_STAND_RIGHT_NAME);
		}
	}

	//--------------------------------------------------------------------------------------------------

	bool Actor::Update (float deltaTime)
	{
		Animable::Update (deltaTime);

		if (!m_IsUpdating)
		{
			return true;
		}

		UpdateScripts (deltaTime);

		if (m_CheckOverlap)
		{
			CheckOverlap ();
		}

		if (!m_Components.empty ())
		{
			for (std::map<std::string, Component*>::iterator it = m_Components.begin (); it != m_Components.end ();
				 ++it)
			{
				it->second->Update (deltaTime);
			}
		}

		return true;
	}

	//--------------------------------------------------------------------------------------------------

	void Actor::Render (float deltaTime)
	{
		if (m_IsVisible)
		{
			Animable::Render (this);

			RenderComponents (deltaTime);
		}
	}

	//--------------------------------------------------------------------------------------------------

	void Actor::AddComponent (const std::string& name, Component* component)
	{
		if (m_Components.find (name) == m_Components.end ())
		{
			m_Components.insert (std::make_pair (name, component));
			component->Initialize ();
		}
	}

	//--------------------------------------------------------------------------------------------------

	void Actor::RemoveComponent (const std::string& name)
	{
		for (std::map<std::string, Component*>::iterator it = m_Components.begin (); it != m_Components.end (); ++it)
		{
			if (it->first == name)
			{
				SAFE_DELETE (it->second);
				m_Components.erase (it);
				break;
			}
		}
	}

	//--------------------------------------------------------------------------------------------------

	void Actor::RemoveComponent (Component* component)
	{
		for (std::map<std::string, Component*>::iterator it = m_Components.begin (); it != m_Components.end (); ++it)
		{
			if (it->second == component)
			{
				SAFE_DELETE (component);
				m_Components.erase (it);
				break;
			}
		}
	}

	//--------------------------------------------------------------------------------------------------

	Component* Actor::FindComponent (const std::string& name, const std::string& typeName)
	{
		for (std::map<std::string, Component*>::iterator it = m_Components.begin (); it != m_Components.end (); ++it)
		{
			if (it->first == name && it->second->GetTypeName () == typeName)
			{
				return it->second;
			}
		}

		return nullptr;
	}

	//--------------------------------------------------------------------------------------------------

	void Actor::RenderComponents (float deltaTime)
	{
		if (!m_Components.empty ())
		{
			for (std::map<std::string, Component*>::iterator it = m_Components.begin (); it != m_Components.end ();
				 ++it)
			{
				it->second->Render (deltaTime);
			}
		}
	}

	//--------------------------------------------------------------------------------------------------

	MovementComponent* Actor::GetMovementComponent (const std::string& name)
	{
		return static_cast<MovementComponent*> (FindComponent (name, MovementComponent::TypeName));
	}

	//--------------------------------------------------------------------------------------------------

	ParticleEmitterComponent* Actor::GetParticleEmitterComponent (const std::string& name)
	{
		return static_cast<ParticleEmitterComponent*> (FindComponent (name, ParticleEmitterComponent::TypeName));
	}

	//--------------------------------------------------------------------------------------------------

	std::map<std::string, Component*>& Actor::GetComponents () { return m_Components; }

	//--------------------------------------------------------------------------------------------------

	void Actor::DrawBounds ()
	{
		if (m_SceneManager->IsDrawBoundingBox ())
		{
			al_draw_rectangle (Bounds.GetTopLeft ().X, Bounds.GetTopLeft ().Y, Bounds.GetBottomRight ().X,
				Bounds.GetBottomRight ().Y, COLOR_YELLOW, 2);
		}
	}

	//--------------------------------------------------------------------------------------------------

	void Actor::DrawName ()
	{
		Point translate = m_SceneManager->GetCamera ().GetTranslate ();
		Point scale = m_SceneManager->GetCamera ().GetScale ();

		float xPoint = Bounds.GetCenter ().X * scale.X - translate.X;
		float yPoint = Bounds.GetBottomRight ().Y * scale.Y - translate.Y;

		Font& font = m_SceneManager->GetMainLoop ()->GetScreen ()->GetFont ();
		std::string str = Name + "[" + std::to_string (ID) + "]";
		font.DrawText (FONT_NAME_SMALL, str, al_map_rgb (0, 255, 0), xPoint, yPoint, 1.0f, ALLEGRO_ALIGN_CENTER);
	}

	//--------------------------------------------------------------------------------------------------

	void Actor::SuspendUpdate () { m_IsUpdating = false; }

	//--------------------------------------------------------------------------------------------------

	void Actor::ResumeUpdate () { m_IsUpdating = true; }

	//--------------------------------------------------------------------------------------------------

	void Actor::Move (float dx, float dy)
	{
		m_OldPosition = Bounds.GetPos ();
		Bounds.SetPos (Bounds.GetPos () + Point (dx, dy));

		if (MoveCallback != nullptr)
		{
			MoveCallback (dx, dy);
		}

		AddPhysOffset ({dx, dy});
	}

	//--------------------------------------------------------------------------------------------------

	void Actor::Move (Point deltaMove) { Actor::Move (deltaMove.X, deltaMove.Y); }

	//--------------------------------------------------------------------------------------------------

	void Actor::SetPosition (float x, float y)
	{
		m_OldPosition = Bounds.GetPos ();
		Bounds.Pos = Point (x, y);

		FireMoveCallback ();

		SetPhysOffset (Bounds.GetPos ());

		MovementComponent* movementComponent = GetMovementComponent ("MOVEMENT_COMPONENT");
		if (movementComponent)
		{
			movementComponent->SetStartPos ({x, y});
		}
	}

	//--------------------------------------------------------------------------------------------------

	void Actor::SetPosition (Point pos) { SetPosition (pos.X, pos.Y); }

	//--------------------------------------------------------------------------------------------------

	void Actor::SetCenterPosition (float x, float y)
	{
		Point halfSize = Bounds.GetHalfSize ();

		SetPosition (x - halfSize.Width, y - halfSize.Height);
	}

	//--------------------------------------------------------------------------------------------------

	void Actor::SetCenterPosition (Point pos) { SetCenterPosition (pos.X, pos.Y); }

	//--------------------------------------------------------------------------------------------------

	Point Actor::GetPosition () { return Bounds.GetPos (); }

	//--------------------------------------------------------------------------------------------------

	Point Actor::GetSize () { return Bounds.GetSize (); }

	//--------------------------------------------------------------------------------------------------

	void Actor::ProcessTriggerAreas (float dx, float dy, Point&& offset)
	{
		std::map<std::string, TriggerArea>& triggerAreas = m_SceneManager->GetActiveScene ()->GetTriggerAreas ();

		for (std::map<std::string, TriggerArea>::iterator it = triggerAreas.begin (); it != triggerAreas.end (); ++it)
		{
			TriggerArea& area = it->second;

			for (Polygon& polygon : area.Polygons)
			{
				if (GetPhysPolygonsCount () > 0
					&& (area.OnEnterCallback || area.ScriptOnEnterCallback || area.OnLeaveCallback
						   || area.ScriptOnLeaveCallback || area.Collidable))
				{
					PolygonCollisionResult r = m_SceneManager->GetMainLoop ()->GetPhysicsManager ().PolygonCollision (
						GetPhysPolygon (0), polygon, {dx, dy});

					if (r.WillIntersect || r.Intersect)
					{
						if (area.Collidable)
						{
							if (!AreSame (r.MinimumTranslationVector, Point::ZERO_POINT))
							{
								offset += r.MinimumTranslationVector;
							}
						}

						if (!area.WasEntered)
						{
							if (area.OnEnterCallback)
							{
								area.OnEnterCallback (
									area.Name, dx + r.MinimumTranslationVector.X, dy + r.MinimumTranslationVector.Y);
							}

							if (area.ScriptOnEnterCallback)
							{
								Point point = {dx + r.MinimumTranslationVector.X, dy + r.MinimumTranslationVector.Y};

								m_SceneManager->GetMainLoop ()->GetScriptManager ().RunScriptFunction (
									area.ScriptOnEnterCallback, &area.Name, &point);
							}
						}

						area.WasEntered = true;
					}
					else if (area.WasEntered)
					{
						area.WasEntered = false;

						if (area.OnLeaveCallback)
						{
							area.OnLeaveCallback (
								area.Name, dx + r.MinimumTranslationVector.X, dy + r.MinimumTranslationVector.Y);
						}

						if (area.ScriptOnLeaveCallback)
						{
							Point point = {dx + r.MinimumTranslationVector.X, dy + r.MinimumTranslationVector.Y};

							m_SceneManager->GetMainLoop ()->GetScriptManager ().RunScriptFunction (
								area.ScriptOnLeaveCallback, &area.Name, &point);
						}
					}
				}
			}
		}
	}

	//--------------------------------------------------------------------------------------------------

	void Actor::FireMoveCallback ()
	{
		if (MoveCallback != nullptr)
		{
			MoveCallback (Bounds.GetPos ().X - m_OldPosition.X, Bounds.GetPos ().Y - m_OldPosition.Y);
		}
	}

	//--------------------------------------------------------------------------------------------------

	void Actor::OrientTo (Actor* actor)
	{
		float angleDeg = GetAngleWith (actor);

		if (GetAnimationsData ().empty ())
		{
			return;
		}

		if (angleDeg > 45 && angleDeg < 135)
		{
			SetCurrentAnimation (ANIM_STAND_DOWN_NAME);
		}

		if (angleDeg > 225 && angleDeg < 315)
		{
			SetCurrentAnimation (ANIM_STAND_UP_NAME);
		}

		if (angleDeg >= 135 && angleDeg <= 225)
		{
			SetCurrentAnimation (ANIM_STAND_RIGHT_NAME);
		}

		if (angleDeg <= 45 || angleDeg >= 315)
		{
			SetCurrentAnimation (ANIM_STAND_LEFT_NAME);
		}
	}

	//--------------------------------------------------------------------------------------------------

	void Actor::AssignFlagPointsToWalk (const std::string& flagPointName)
	{
		MovementComponent* movementComponent = GetMovementComponent ("MOVEMENT_COMPONENT");
		if (movementComponent)
		{
			movementComponent->SetMovementType (MovementType::MovePoints);

			FlagPoint* flagPoint = m_SceneManager->GetFlagPoint (flagPointName);

			if (flagPoint)
			{
				movementComponent->SetWalkPoints (flagPoint);
			}
		}
	}

	//--------------------------------------------------------------------------------------------------

	float Actor::GetFocusHeight () const { return m_FocusHeight; }

	//--------------------------------------------------------------------------------------------------

	void Actor::SetFocusHeight (float focusHeight) { m_FocusHeight = focusHeight; }

	//--------------------------------------------------------------------------------------------------

	std::string Actor::GetActionSpeech () { return m_ActionSpeech; }

	//--------------------------------------------------------------------------------------------------

	void Actor::SetActionSpeech (const std::string& speechID) { m_ActionSpeech = speechID; }

	//--------------------------------------------------------------------------------------------------

	bool Actor::IsActionSpeechHandling () { return m_ActionSpeechHandling; }

	//--------------------------------------------------------------------------------------------------

	void Actor::SetActionSpeechHandling (bool handling) { m_ActionSpeechHandling = handling; }

	//--------------------------------------------------------------------------------------------------

	void Actor::Show () { m_IsVisible = true; }

	//--------------------------------------------------------------------------------------------------

	void Actor::Hide () { m_IsVisible = false; }

	//--------------------------------------------------------------------------------------------------

	bool Actor::IsVisible () { return m_IsVisible; }

	//--------------------------------------------------------------------------------------------------
}
