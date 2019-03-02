// Copyright 2017-2019 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "Scene.h"
#include "Atlas.h"
#include "MainLoop.h"
#include "Player.h"
#include "Screen.h"
#include "SpeechFrameManager.h"
#include "actors/TileActor.h"
#include "states/EditorState.h"
#include "states/GamePlayState.h"

using json = nlohmann::json;

namespace aga
{
	//--------------------------------------------------------------------------------------------------

	/*
	 *
	 *  {
	 *      "name": "Home",
	 *      "flag_points" :
	 *      [
	 *          "SPAWN_1" : "0 0"
	 *      ]
	 *  }
	 *
	 */

	//--------------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------------

	int Entity::GlobalID = 0;

	//--------------------------------------------------------------------------------------------------

	void TriggerArea::UpdatePolygons (Triangulator* triangulator)
	{
		if (Points.size () > 2)
		{
			Polygons.clear ();

			std::vector<std::vector<Point>> result;
			std::vector<Point> pointsCopy = Points;

			int validate = triangulator->Validate (pointsCopy);

			if (validate == 2)
			{
				std::reverse (pointsCopy.begin (), pointsCopy.end ());
			}

			if (triangulator->Validate (pointsCopy) == 0)
			{
				triangulator->ProcessVertices (&pointsCopy, result);

				for (size_t j = 0; j < result.size (); ++j)
				{
					Polygon poly = Polygon ();
					poly.Points = result[j];
					poly.BuildEdges ();

					Polygons.push_back (poly);
				}
			}
		}
	}

	//--------------------------------------------------------------------------------------------------

	const float SCENE_INFINITE_BOUND_SIZE = 10000.f;

	Scene::Scene (SceneManager* sceneManager, Rect size)
		: Scriptable (&sceneManager->GetMainLoop ()->GetScriptManager ())
		, m_BackgroundColor (COLOR_BLACK)
		, m_SceneManager (sceneManager)
		, m_CurrentActor (nullptr)
		, m_QuadTree (size)
		, m_ActorsTreeChanged (false)
		, m_SceneAudioStream (nullptr)
		, m_SuppressSceneInfo (false)
		, m_DummyActor (nullptr)
	{
	}

	//--------------------------------------------------------------------------------------------------

	Scene::~Scene ()
	{
		if (!IsDestroyed ())
		{
			Destroy ();
		}
	}

	//--------------------------------------------------------------------------------------------------

	bool Scene::Initialize ()
	{
		m_DummyActor = new TileActor (m_SceneManager);

		AddActor (m_DummyActor);

		return Lifecycle::Initialize ();
	}

	//--------------------------------------------------------------------------------------------------

	bool Scene::Destroy ()
	{
		CleanUpSceneAudio ();

		for (size_t i = 0; i < m_Actors.size (); ++i)
		{
			SAFE_DELETE (m_Actors[i]);
		}

		return Lifecycle::Destroy ();
	}

	//--------------------------------------------------------------------------------------------------

	void Scene::BeforeEnter ()
	{
		SetBackgroundColor (m_BackgroundColor);

		m_VisibleEntities.clear ();
		m_VisibleLastCameraPos = Point::MIN_POINT;

		//  Reset camera to player
		Player* player = m_SceneManager->GetPlayer ();
		Point newPosition;

		ResetAllActorsPositions ();

		if (m_SceneManager->GetMainLoop ()->GetStateManager ().GetActiveStateName () == GAMEPLAY_STATE_NAME)
		{
			player->BeforeEnter ();

			Point playerHalfSize = player->Bounds.GetHalfSize ();
			newPosition
				= {m_PlayerStartLocation.X - playerHalfSize.Width, m_PlayerStartLocation.Y - playerHalfSize.Height};
		}
		else
		{
			newPosition = player->TemplateBounds.Pos;
		}

		player->SetPosition (newPosition);

		if (m_SceneManager->GetMainLoop ()->GetStateManager ().GetActiveStateName () == GAMEPLAY_STATE_NAME)
		{
			RunAllScripts ("void BeforeEnterScene ()");

			for (Actor* actor : m_Actors)
			{
				m_CurrentActor = actor;
				actor->RunAllScripts ("void BeforeEnterScene ()");
			}
		}

		m_CurrentActor = nullptr;
	}

	//--------------------------------------------------------------------------------------------------

	void Scene::AfterLeave ()
	{
		if (m_SceneManager->GetMainLoop ()->GetStateManager ().GetActiveStateName () == GAMEPLAY_STATE_NAME)
		{
			m_SceneManager->GetPlayer ()->AfterLeave ();

			for (Actor* actor : m_Actors)
			{
				m_CurrentActor = actor;
				actor->RunAllScripts ("void AfterLeaveScene ()");
				actor->Bounds = actor->TemplateBounds;
			}

			m_CurrentActor = nullptr;

			RunAllScripts ("void AfterLeaveScene ()");
		}

		m_VisibleEntities.clear ();
		m_VisibleLastCameraPos = Point::MIN_POINT;

		m_SceneManager->GetSpeechFrameManager ()->Clear ();

		CleanUpTriggerAreas ();
	}

	//--------------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------------

	void Scene::Update (float deltaTime)
	{
		if (m_SceneManager->GetMainLoop ()->GetStateManager ().GetActiveStateName () == GAMEPLAY_STATE_NAME)
		{
			UpdateCameraBounds ();

			m_SceneManager->GetPlayer ()->Update (deltaTime);
			UpdateScripts (deltaTime);

			for (Actor* actor : m_Actors)
			{
				m_CurrentActor = actor;
				actor->Update (deltaTime);
			}

			m_CurrentActor = nullptr;
		}
	}

	//--------------------------------------------------------------------------------------------------

	void Scene::UpdateCameraBounds ()
	{
		TriggerArea* sceneBounds = GetTriggerArea ("SCENE_BOUNDS");

		Camera& camera = m_SceneManager->GetCamera ();
		Actor* followActor = camera.GetFollowActor ();

		if (!followActor)
		{
			followActor = m_SceneManager->GetPlayer ();
		}

		if (sceneBounds)
		{
			float scaleX = camera.GetScale ().X;
			float scaleY = camera.GetScale ().Y;

			Point screenSize = m_SceneManager->GetMainLoop ()->GetScreen ()->GetBackBufferSize ();
			const Point halfScreen = screenSize * 0.5f;

			Rect followActorBounds = followActor->Bounds;
			Point followActorPos = followActorBounds.Pos + followActorBounds.GetHalfSize ();

			std::vector<Point>& scenePoints = sceneBounds->Points;
			Point p1;
			Point p2;
			bool foundXIntersection = false;
			bool foundYIntersection = false;
			Point intersectPoint = camera.GetSavedFollowPoint ();

			for (size_t i = 0; i < scenePoints.size (); ++i)
			{
				p1 = scenePoints[i];

				if (i + 1 >= scenePoints.size ())
				{
					p2 = scenePoints[0];
				}
				else
				{
					p2 = scenePoints[i + 1];
				}

				Point tmpPoint;

				//  Left & Right
				if (LineCollision (
						followActorPos, Point (followActorPos.X - halfScreen.Width, followActorPos.Y), p1, p2, tmpPoint)
					&& (followActorPos.X * scaleX - tmpPoint.X * scaleX) <= halfScreen.Width)
				{
					intersectPoint.X = -tmpPoint.X * scaleX;
					foundXIntersection = true;
				}
				else if (LineCollision (followActorPos, Point (followActorPos.X + halfScreen.Width, followActorPos.Y),
							 p1, p2, tmpPoint)
					&& (tmpPoint.X * scaleX - followActorPos.X * scaleX) <= halfScreen.Width)
				{
					intersectPoint.X = -tmpPoint.X * scaleX + screenSize.Width;
					foundXIntersection = true;
				}

				//  Top && Bottom
				if (LineCollision (followActorPos, Point (followActorPos.X, followActorPos.Y - halfScreen.Height), p1,
						p2, tmpPoint)
					&& (followActorPos.Y * scaleY - tmpPoint.Y * scaleY) <= halfScreen.Height)
				{
					intersectPoint.Y = -tmpPoint.Y * scaleY;
					foundYIntersection = true;
				}
				else if (LineCollision (followActorPos, Point (followActorPos.X, followActorPos.Y + halfScreen.Height),
							 p1, p2, tmpPoint)
					&& (tmpPoint.Y * scaleY - followActorPos.Y * scaleY) <= halfScreen.Height)
				{
					intersectPoint.Y = -tmpPoint.Y * scaleY + screenSize.Height;
					foundYIntersection = true;
				}
			}

			if (foundXIntersection)
			{
				camera.SetSavedFollowPoint (intersectPoint);
			}

			if (foundYIntersection)
			{
				camera.SetSavedFollowPoint (intersectPoint);
			}

			camera.SetFollowingXAxis (!foundXIntersection);
			camera.SetFollowingYAxis (!foundYIntersection);
		}
	}

	//--------------------------------------------------------------------------------------------------

	void Scene::Render (float deltaTime)
	{
		m_SceneManager->GetCamera ().Update (deltaTime);

		Player* player = m_SceneManager->GetPlayer ();

		RecomputeVisibleEntities (false);

		if (m_SceneManager->IsDrawBoundingBox ())
		{
			DrawQuadTree (&m_QuadTree);
		}

		bool isPlayerDrawn = false;

		int minZOrder = 1000000;
		int maxZOrder = -1000000;
		for (size_t i = 0; i < m_VisibleEntities.size (); ++i)
		{
			int zOrder = m_VisibleEntities[i]->ZOrder;

			if (zOrder < minZOrder)
			{
				minZOrder = zOrder;
			}

			if (zOrder > maxZOrder)
			{
				maxZOrder = zOrder;
			}
		}

		std::vector<std::vector<Entity*>> sortedEntities;

		for (int currZOrder = minZOrder; currZOrder <= maxZOrder; ++currZOrder)
		{
			std::vector<Entity*> entitiesPack;

			for (size_t i = 0; i < m_VisibleEntities.size (); ++i)
			{
				Actor* actor = static_cast<Actor*> (m_VisibleEntities[i]);

				if (actor->ZOrder == currZOrder)
				{
					entitiesPack.push_back (actor);
				}
			}

			if (!entitiesPack.empty ())
			{
				sortedEntities.push_back (entitiesPack);
			}
		}

		int renderID = 0;
		for (std::vector<Entity*> pack : sortedEntities)
		{
			for (Entity* ent : pack)
			{
				Actor* actor = static_cast<Actor*> (ent);

				float heightPercentage = actor->GetFocusHeight ();
				float maxHeight = actor->Bounds.Pos.Y + actor->Bounds.Size.Height * heightPercentage;
				float playerMidPoint = player->Bounds.Pos.Y + player->Bounds.Size.Height;

				if (Intersect (actor->Bounds, player->Bounds) && !isPlayerDrawn && actor->ZOrder >= PLAYER_Z_ORDER
					&& playerMidPoint < maxHeight)
				{
					player->Render (deltaTime);
					isPlayerDrawn = true;
				}

				actor->RenderID = renderID++;
				actor->Render (deltaTime);

				if (!isPlayerDrawn && actor->ZOrder >= PLAYER_Z_ORDER && maxHeight < playerMidPoint)
				{
					//      player->Render (deltaTime);
					//       isPlayerDrawn = true;
				}

				if (m_SceneManager->IsDrawBoundingBox ())
				{
					actor->DrawBounds ();
				}

				if (m_SceneManager->IsDrawPhysData ())
				{
					actor->DrawPhysBody ();
				}

				if (m_SceneManager->IsDrawActorsNames ())
				{
					actor->DrawName ();
				}
			}
		}

		if (!isPlayerDrawn)
		{
			player->Render (deltaTime);
		}

		if (m_SceneManager->IsDrawBoundingBox ())
		{
			player->DrawBounds ();
		}

		if (m_SceneManager->IsDrawPhysData ())
		{
			player->DrawPhysBody ();
		}

		if (m_SceneManager->GetMainLoop ()->GetStateManager ().GetActiveStateName () == EDITOR_STATE_NAME)
		{
			//  Draw circle in center for reference
			al_draw_filled_circle (0, 0, 4, COLOR_RED);

			al_draw_filled_circle (m_PlayerStartLocation.X, m_PlayerStartLocation.Y, 4, COLOR_LIGHTBLUE);
		}

		m_SceneManager->GetCamera ().UseIdentityTransform ();

		if (player->IsInventoryShown ())
		{
			int blendOp, blendSrc, blendDst;
			al_get_blender (&blendOp, &blendSrc, &blendDst);
			al_set_blender (ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA);

			Point winSize = m_SceneManager->GetMainLoop ()->GetScreen ()->GetBackBufferSize ();
			ALLEGRO_COLOR color = al_map_rgba (0, 0, 0, 128);
			al_draw_filled_rectangle (0, 0, winSize.Width, winSize.Height, color);

			al_set_blender (blendOp, blendSrc, blendDst);

			player->DrawInventory (deltaTime);
		}

		Font& font = m_SceneManager->GetMainLoop ()->GetScreen ()->GetFont ();
		font.DrawText (FONT_NAME_SMALL, std::to_string (m_VisibleEntities.size ()), al_map_rgb (0, 255, 0), 10,
			m_SceneManager->GetMainLoop ()->GetScreen ()->GetBackBufferSize ().Height - 12, 1.0f, ALLEGRO_ALIGN_LEFT);
	}

	//--------------------------------------------------------------------------------------------------

	std::vector<Entity*> Scene::RecomputeVisibleEntities (bool force)
	{
		Point cameraCenter = m_SceneManager->GetCamera ().GetCenter ();

		if ((cameraCenter != m_VisibleLastCameraPos) || m_ActorsTreeChanged || force)
		{
			Point cameraScale = m_SceneManager->GetCamera ().GetScale ();
			Point screenSize = m_SceneManager->GetMainLoop ()->GetScreen ()->GetBackBufferSize ();

			float visibleScale = 0.7f;
			Point moveBy (
				screenSize.Width * visibleScale / cameraScale.X, screenSize.Height * visibleScale / cameraScale.Y);
			Rect targetRect = Rect (cameraCenter - moveBy, cameraCenter + moveBy);

			m_VisibleEntities = m_QuadTree.GetEntitiesWithinRect (targetRect);

			std::sort (m_VisibleEntities.begin (), m_VisibleEntities.end (), Entity::CompareByZOrder);

			m_ActorsTreeChanged = false;
		}

		m_VisibleLastCameraPos = cameraCenter;

		return m_VisibleEntities;
	}

	//--------------------------------------------------------------------------------------------------

	void Scene::AddActor (Actor* actor)
	{
		Actor* act = GetActor (actor->ID);

		if (!act && actor)
		{
			m_Actors.push_back (actor);
			m_QuadTree.Insert (actor);
			m_QuadTree.UpdateStructures ();

			m_ActorsTreeChanged = true;
		}
	}

	//--------------------------------------------------------------------------------------------------

	void Scene::RemoveActor (Actor* actor)
	{
		for (size_t i = 0; i < m_Actors.size (); ++i)
		{
			if (m_Actors[i] == actor)
			{
				m_ActorsTreeChanged = true;

				m_QuadTree.Remove (m_Actors[i]);
				m_QuadTree.UpdateStructures ();
				SAFE_DELETE (m_Actors[i]);
				m_Actors.erase (m_Actors.begin () + i);
				break;
			}
		}
	}

	//--------------------------------------------------------------------------------------------------

	void Scene::RemoveActor (const std::string& name)
	{
		Actor* actor = GetActor (name);

		if (actor)
		{
			RemoveActor (actor);
		}
	}

	//--------------------------------------------------------------------------------------------------

	void Scene::RemoveActor (int id)
	{
		Actor* actor = GetActor (id);

		if (actor)
		{
			RemoveActor (actor);
		}
	}

	//--------------------------------------------------------------------------------------------------

	Actor* Scene::GetActor (const std::string& name)
	{
		for (Actor* actor : m_Actors)
		{
			if (actor->Name == name)
			{
				return actor;
			}
		}

		return nullptr;
	}

	//--------------------------------------------------------------------------------------------------

	Actor* Scene::GetActor (int id)
	{
		for (Actor* actor : m_Actors)
		{
			if (actor->ID == id)
			{
				return actor;
			}
		}

		return nullptr;
	}

	//--------------------------------------------------------------------------------------------------

	std::vector<Actor*>& Scene::GetActors () { return m_Actors; }

	//--------------------------------------------------------------------------------------------------

	void Scene::SortActors () { std::sort (m_Actors.begin (), m_Actors.end (), Entity::CompareByZOrder); }

	//--------------------------------------------------------------------------------------------------

	std::vector<TileActor*> Scene::GetTiles ()
	{
		std::vector<TileActor*> tiles;

		for (Actor* actor : m_Actors)
		{
			if (actor->GetTypeName () == TileActor::TypeName)
			{
				TileActor* tile = dynamic_cast<TileActor*> (actor);

				if (tile)
				{
					tiles.push_back (tile);
				}
			}
		}

		return tiles;
	}

	//--------------------------------------------------------------------------------------------------

	FlagPoint* Scene::AddFlagPoint (const std::string& name, Point point)
	{
		if (m_FlagPoints.find (name) == m_FlagPoints.end ())
		{
			FlagPoint fp;
			fp.Name = name;
			fp.Pos = point;

			m_FlagPoints.insert (make_pair (name, fp));
		}

		return GetFlagPoint (name);
	}

	//--------------------------------------------------------------------------------------------------

	FlagPoint* Scene::GetFlagPoint (const std::string& name)
	{
		if (!m_FlagPoints.empty ())
		{
			std::map<std::string, FlagPoint>::iterator it = m_FlagPoints.find (name);

			if (it != m_FlagPoints.end ())
			{
				return &(*it).second;
			}
		}

		return nullptr;
	}

	//--------------------------------------------------------------------------------------------------

	std::map<std::string, FlagPoint>& Scene::GetFlagPoints () { return m_FlagPoints; }

	//--------------------------------------------------------------------------------------------------

	void Scene::RemoveFlagPoint (const std::string& name) { m_FlagPoints.erase (name); }

	//--------------------------------------------------------------------------------------------------

	TriggerArea* Scene::AddTriggerArea (
		const std::string& name, const std::string& data, std::vector<Point> points, bool collidable)
	{
		if (m_TriggerAreas.find (name) == m_TriggerAreas.end ())
		{
			TriggerArea area{name, data, points};
			area.Collidable = collidable;

			area.UpdatePolygons (&m_SceneManager->GetMainLoop ()->GetPhysicsManager ().GetTriangulator ());

			m_TriggerAreas.insert (std::make_pair (name, area));
		}

		return &m_TriggerAreas[name];
	}

	//--------------------------------------------------------------------------------------------------

	std::map<std::string, TriggerArea>& Scene::GetTriggerAreas () { return m_TriggerAreas; }

	//--------------------------------------------------------------------------------------------------

	TriggerArea* Scene::GetTriggerArea (const std::string& name)
	{
		if (m_TriggerAreas.find (name) != m_TriggerAreas.end ())
		{
			return &m_TriggerAreas[name];
		}
		else
		{
			return nullptr;
		}
	}

	//--------------------------------------------------------------------------------------------------

	void Scene::RemoveTriggerArea (const std::string& name)
	{
		if (m_TriggerAreas.find (name) != m_TriggerAreas.end ())
		{
			m_TriggerAreas.erase (name);
		}
	}

	//--------------------------------------------------------------------------------------------------

	bool Scene::AddSpeech (SpeechData data)
	{
		if (!GetSpeech (data.ID))
		{
			m_Speeches.insert (std::make_pair (data.ID, data));

			return true;
		}

		return false;
	}

	//--------------------------------------------------------------------------------------------------

	std::map<long, SpeechData>& Scene::GetSpeeches () { return m_Speeches; }

	//--------------------------------------------------------------------------------------------------

	SpeechData* Scene::GetSpeech (long id)
	{
		for (std::map<long, SpeechData>::iterator it = m_Speeches.begin (); it != m_Speeches.end (); ++it)
		{
			if (it->first == id)
			{
				return &(*it).second;
			}
		}

		return nullptr;
	}

	//--------------------------------------------------------------------------------------------------

	SpeechData* Scene::GetSpeech (const std::string& name)
	{
		for (std::map<long, SpeechData>::iterator it = m_Speeches.begin (); it != m_Speeches.end (); ++it)
		{
			if (it->second.Name == name)
			{
				return &(*it).second;
			}
		}

		return nullptr;
	}

	//--------------------------------------------------------------------------------------------------

	void Scene::RemoveSpeech (long id)
	{
		for (std::map<long, SpeechData>::iterator it = m_Speeches.begin (); it != m_Speeches.end (); ++it)
		{
			if (it->first == id)
			{
				m_Speeches.erase (it);
				return;
			}
		}
	}

	//--------------------------------------------------------------------------------------------------

	void Scene::RemoveSpeech (const std::string& name)
	{
		for (std::map<long, SpeechData>::iterator it = m_Speeches.begin (); it != m_Speeches.end (); ++it)
		{
			if (it->second.Name == name)
			{
				m_Speeches.erase (it);
				return;
			}
		}
	}

	//--------------------------------------------------------------------------------------------------

	void Scene::Reset ()
	{
		RemoveAllScripts ();

		for (size_t i = 0; i < m_Actors.size (); ++i)
		{
			SAFE_DELETE (m_Actors[i]);
		}

		m_Actors.clear ();
		m_VisibleEntities.clear ();

		m_SceneManager->GetSpeechFrameManager ()->Clear ();
		m_SceneManager->GetMainLoop ()->GetTweenManager ().Clear ();

		m_FlagPoints.clear ();
		m_TriggerAreas.clear ();
		m_Speeches.clear ();

		m_QuadTree = QuadTreeNode (Rect ({-SCENE_INFINITE_BOUND_SIZE, -SCENE_INFINITE_BOUND_SIZE},
			{SCENE_INFINITE_BOUND_SIZE, SCENE_INFINITE_BOUND_SIZE}));

		Entity::GlobalID = 0;
	}

	//--------------------------------------------------------------------------------------------------

	void Scene::SetName (const std::string& name) { m_Name = name; }

	//--------------------------------------------------------------------------------------------------

	std::string Scene::GetName () const { return m_Name; }

	//--------------------------------------------------------------------------------------------------

	void Scene::SetBackgroundColor (ALLEGRO_COLOR color) { SetBackgroundColor (color.r, color.g, color.b, color.a); }

	//--------------------------------------------------------------------------------------------------

	void Scene::SetBackgroundColor (float r, float g, float b, float a)
	{
		m_BackgroundColor = al_map_rgba_f (r, g, b, a);
		m_SceneManager->GetMainLoop ()->GetScreen ()->SetBackgroundColor (m_BackgroundColor);
	}

	//--------------------------------------------------------------------------------------------------

	ALLEGRO_COLOR Scene::GetBackgroundColor () const { return m_BackgroundColor; }

	//--------------------------------------------------------------------------------------------------

	QuadTreeNode& Scene::GetQuadTree () { return m_QuadTree; }

	//--------------------------------------------------------------------------------------------------

	void Scene::DrawQuadTree (QuadTreeNode* node)
	{
		Rect bounds = node->GetBounds ();
		al_draw_rectangle (bounds.GetTopLeft ().X, bounds.GetTopLeft ().Y, bounds.GetBottomRight ().X,
			bounds.GetBottomRight ().Y, COLOR_WHITE, 1);

		Font& font = m_SceneManager->GetMainLoop ()->GetScreen ()->GetFont ();
		font.DrawText (FONT_NAME_SMALL, std::to_string (node->GetData ().size ()), al_map_rgb (255, 255, 0),
			bounds.GetCenter ().X, bounds.GetCenter ().Y, 1.0f, ALLEGRO_ALIGN_CENTER);

		if (node->GetTopLeftTree ())
		{
			DrawQuadTree (node->GetTopLeftTree ());
		}

		if (node->GetTopRightTree ())
		{
			DrawQuadTree (node->GetTopRightTree ());
		}

		if (node->GetBottomLeftTree ())
		{
			DrawQuadTree (node->GetBottomLeftTree ());
		}

		if (node->GetBottomRightTree ())
		{
			DrawQuadTree (node->GetBottomRightTree ());
		}
	}

	//--------------------------------------------------------------------------------------------------

	void ReleaseASFunction (asIScriptFunction* function)
	{
		if (function)
		{
			int refCount = function->AddRef ();

			for (int i = 0; i < refCount; ++i)
			{
				function->Release ();
			}
		}
	}

	//--------------------------------------------------------------------------------------------------

	void Scene::AddOnEnterCallback (
		const std::string& triggerName, std::function<void(std::string areaName, float dx, float dy)> func)
	{
		if (m_TriggerAreas.find (triggerName) != m_TriggerAreas.end ())
		{
			m_TriggerAreas[triggerName].OnEnterCallback = func;
		}
	}

	//--------------------------------------------------------------------------------------------------

	void Scene::AddOnEnterCallback (const std::string& triggerName, asIScriptFunction* func)
	{
		if (m_TriggerAreas.find (triggerName) != m_TriggerAreas.end ())
		{
			ReleaseASFunction (m_TriggerAreas[triggerName].ScriptOnEnterCallback);
		}

		m_TriggerAreas[triggerName].ScriptOnEnterCallback = func;
	}

	//--------------------------------------------------------------------------------------------------

	void Scene::RemoveOnEnterCallback (const std::string& triggerName)
	{
		if (m_TriggerAreas.find (triggerName) != m_TriggerAreas.end ())
		{
			ReleaseASFunction (m_TriggerAreas[triggerName].ScriptOnEnterCallback);

			m_TriggerAreas[triggerName].ScriptOnEnterCallback = nullptr;
		}
	}

	//--------------------------------------------------------------------------------------------------

	void Scene::AddOnLeaveCallback (
		const std::string& triggerName, std::function<void(std::string areaName, float dx, float dy)> func)
	{
		if (m_TriggerAreas.find (triggerName) != m_TriggerAreas.end ())
		{
			m_TriggerAreas[triggerName].OnLeaveCallback = func;
		}
	}

	//--------------------------------------------------------------------------------------------------

	void Scene::AddOnLeaveCallback (const std::string& triggerName, asIScriptFunction* func)
	{
		if (m_TriggerAreas.find (triggerName) != m_TriggerAreas.end ())
		{
			ReleaseASFunction (m_TriggerAreas[triggerName].ScriptOnLeaveCallback);
		}

		m_TriggerAreas[triggerName].ScriptOnLeaveCallback = func;
	}

	//--------------------------------------------------------------------------------------------------

	void Scene::RemoveOnLeaveCallback (const std::string& triggerName)
	{
		if (m_TriggerAreas.find (triggerName) != m_TriggerAreas.end ())
		{
			ReleaseASFunction (m_TriggerAreas[triggerName].ScriptOnLeaveCallback);

			m_TriggerAreas[triggerName].ScriptOnLeaveCallback = nullptr;
		}
	}

	//--------------------------------------------------------------------------------------------------

	Rect Scene::GetRenderBounds (Entity* entity, bool drawOOBBox)
	{
		Point translate = m_SceneManager->GetCamera ().GetTranslate ();
		Point scale = m_SceneManager->GetCamera ().GetScale ();

		Rect b = entity->Bounds;
		float halfWidth = b.GetHalfSize ().Width;
		float halfHeight = b.GetHalfSize ().Height;

		float x1 = (b.GetPos ().X - translate.X * (1.0f / scale.X)) * (scale.X);
		float y1 = (b.GetPos ().Y - translate.Y * (1.0f / scale.Y)) * (scale.Y);
		float x2 = (b.GetPos ().X - translate.X * (1.0f / scale.X) + 2 * halfWidth) * (scale.X);
		float y2 = (b.GetPos ().Y - translate.Y * (1.0f / scale.Y) + 2 * halfHeight) * (scale.Y);

		Point origin = {x1 + (x2 - x1) * 0.5f, y1 + (y2 - y1) * 0.5f};
		Point pointA = RotatePoint (x1, y1, origin, entity->Rotation);
		Point pointB = RotatePoint (x1, y2, origin, entity->Rotation);
		Point pointC = RotatePoint (x2, y1, origin, entity->Rotation);
		Point pointD = RotatePoint (x2, y2, origin, entity->Rotation);

		if (drawOOBBox)
		{
			float vertices[] = {pointA.X, pointA.Y, pointB.X, pointB.Y, pointD.X, pointD.Y, pointC.X, pointC.Y};
			ALLEGRO_COLOR color = {0.5f, 0.0f, 0.0f, 1.0f};
			al_draw_polygon (vertices, 4, ALLEGRO_LINE_JOIN_BEVEL, color, 2, 0);
		}

		float minX = std::min (pointA.X, std::min (pointB.X, std::min (pointC.X, pointD.X)));
		float minY = std::min (pointA.Y, std::min (pointB.Y, std::min (pointC.Y, pointD.Y)));
		float maxX = std::max (pointA.X, std::max (pointB.X, std::max (pointC.X, pointD.X)));
		float maxY = std::max (pointA.Y, std::max (pointB.Y, std::max (pointC.Y, pointD.Y)));

		return OrientRect (minX, minY, maxX, maxY);
	}

	//--------------------------------------------------------------------------------------------------

	Rect Scene::GetRenderBounds (Rect b)
	{
		Point translate = m_SceneManager->GetCamera ().GetTranslate ();
		Point scale = m_SceneManager->GetCamera ().GetScale ();

		float halfWidth = b.GetHalfSize ().Width;
		float halfHeight = b.GetHalfSize ().Height;

		float x1 = (b.GetPos ().X - translate.X * (1.0f / scale.X)) * (scale.X);
		float y1 = (b.GetPos ().Y - translate.Y * (1.0f / scale.Y)) * (scale.Y);
		float x2 = (b.GetPos ().X - translate.X * (1.0f / scale.X) + 2 * halfWidth) * (scale.X);
		float y2 = (b.GetPos ().Y - translate.Y * (1.0f / scale.Y) + 2 * halfHeight) * (scale.Y);

		return OrientRect (x1, y1, x2, y2);
	}

	//--------------------------------------------------------------------------------------------------

	Actor* Scene::GetCurrentlyProcessedActor () { return m_CurrentActor; }

	//--------------------------------------------------------------------------------------------------

	void Scene::ResetAllActorsPositions ()
	{
		for (Actor* actor : m_Actors)
		{
			actor->SetPosition (actor->TemplateBounds.Pos);
			actor->SetPhysOffset (actor->Bounds.GetPos () + actor->Bounds.GetHalfSize ());
		}
	}

	//--------------------------------------------------------------------------------------------------

	void Scene::CleanUpTriggerAreas ()
	{
		for (std::map<std::string, TriggerArea>::iterator it = m_TriggerAreas.begin (); it != m_TriggerAreas.end ();
			 ++it)
		{
			if (it->second.ScriptOnEnterCallback)
			{
				it->second.ScriptOnEnterCallback = nullptr;
			}

			if (it->second.ScriptOnLeaveCallback)
			{
				it->second.ScriptOnLeaveCallback = nullptr;
			}
		}
	}

	//--------------------------------------------------------------------------------------------------

	void Scene::RegisterChoiceFunction (const std::string& name, asIScriptFunction* func)
	{
		if (m_ChoiceFunctions.find (name) == m_ChoiceFunctions.end ())
		{
			m_ChoiceFunctions.insert (std::make_pair (name, func));
		}
	}

	//--------------------------------------------------------------------------------------------------

	AudioStream* Scene::SetSceneAudioStream (const std::string& path)
	{
		CleanUpSceneAudio ();

		if (path != "")
		{
			AudioManager& audioManager = m_SceneManager->GetMainLoop ()->GetAudioManager ();
			std::string sampleName = GetBaseName (path) + "_SCENE_STREAM";
			m_SceneAudioStream = audioManager.LoadStreamFromFile (sampleName, path);
		}

		return m_SceneAudioStream;
	}

	//--------------------------------------------------------------------------------------------------

	void Scene::CleanUpSceneAudio ()
	{
		AudioManager& audioManager = m_SceneManager->GetMainLoop ()->GetAudioManager ();
		std::string streamName = this->GetName () + "_SCENE_STREAM";

		audioManager.RemoveStream (streamName);

		m_SceneAudioStream = nullptr;
	}

	//--------------------------------------------------------------------------------------------------

	void Scene::AddSceneTransition (const std::string& triggerAreaName, const std::string& newSceneName)
	{
		m_ScenesTransitions[triggerAreaName] = newSceneName;
	}

	//--------------------------------------------------------------------------------------------------

	std::string Scene::GetSceneTransition (const std::string& triggerAreaName)
	{
		return m_ScenesTransitions[triggerAreaName];
	}

	//--------------------------------------------------------------------------------------------------

	Actor* Scene::GetDummyActor () { return m_DummyActor; }

	//--------------------------------------------------------------------------------------------------
}
