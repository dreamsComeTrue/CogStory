// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "Scene.h"
#include "Atlas.h"
#include "AtlasManager.h"
#include "MainLoop.h"
#include "SceneManager.h"
#include "Screen.h"
#include "actors/TileActor.h"

#include "addons/triangulator/Triangulator.h"

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

            int counter = 0;
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

                for (int j = 0; j < result.size (); ++j)
                {
                    Polygons.push_back (Polygon ());
                    Polygons[counter].Points = result[j];
                    Polygons[counter].BuildEdges ();

                    ++counter;
                }
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    const float boundSize = 10000;

    Scene::Scene (SceneManager* sceneManager)
        : Scriptable (&sceneManager->GetMainLoop ()->GetScriptManager ())
        , m_SceneManager (sceneManager)
        , m_QuadTree (Rect ({ -boundSize, -boundSize }, { boundSize, boundSize }))
        , m_CurrentActor (nullptr)
        , m_BackgroundColor (al_map_rgb (60, 60, 70))
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

    bool Scene::Initialize () { return Lifecycle::Initialize (); }

    //--------------------------------------------------------------------------------------------------

    bool Scene::Destroy ()
    {
        for (int i = 0; i < m_Actors.size (); ++i)
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
        Player& player = m_SceneManager->GetPlayer ();

        if (m_SceneManager->GetMainLoop ()->GetStateManager ().GetActiveStateName () != "EDITOR_STATE")
        {
            player.BeforeEnter ();
            player.SetPosition (m_PlayerStartLocation);
        }
        else
        {
            player.SetPosition (player.TemplateBounds.Pos);
        }

        ResetAllActorsPositions ();
        RunAllScripts ("void BeforeEnterScene ()");

        for (Actor* actor : m_Actors)
        {
            m_CurrentActor = actor;
            actor->RunAllScripts ("void BeforeEnterScene ()");
        }

        m_CurrentActor = nullptr;
    }

    //--------------------------------------------------------------------------------------------------

    void Scene::AfterLeave ()
    {
        if (m_SceneManager->GetMainLoop ()->GetStateManager ().GetActiveStateName () != "EDITOR_STATE")
        {
            m_SceneManager->GetPlayer ().AfterLeave ();
        }

        for (Actor* actor : m_Actors)
        {
            m_CurrentActor = actor;
            actor->RunAllScripts ("void AfterLeaveScene ()");
            actor->Bounds = actor->TemplateBounds;
        }

        m_CurrentActor = nullptr;

        RunAllScripts ("void AfterLeaveScene ()");

        m_VisibleEntities.clear ();
        m_VisibleLastCameraPos = Point::MIN_POINT;

        CleanUpTriggerAreas ();
    }

    //--------------------------------------------------------------------------------------------------
    //--------------------------------------------------------------------------------------------------

    void Scene::Update (float deltaTime)
    {
        if (m_SceneManager->GetMainLoop ()->GetStateManager ().GetActiveStateName () != "EDITOR_STATE")
        {
            m_SceneManager->GetPlayer ().Update (deltaTime);
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

    void Scene::Render (float deltaTime)
    {
        m_SceneManager->GetCamera ().Update (deltaTime);

        if (m_SceneManager->IsDrawBoundingBox ())
        {
            DrawQuadTree (&m_QuadTree);
        }

        bool isPlayerDrawn = false;
        std::vector<Entity*> entities;

        if (m_SceneManager->GetMainLoop ()->GetStateManager ().GetActiveStateName () == "EDITOR_STATE")
        {
            for (Actor* actor : m_Actors)
            {
                if (!isPlayerDrawn && actor->ZOrder >= PLAYER_Z_ORDER)
                {
                    m_SceneManager->GetPlayer ().Render (deltaTime);
                    isPlayerDrawn = true;
                }

                actor->Render (deltaTime);

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
        else
        {
            entities = GetVisibleEntities ();

            for (int i = 0; i < entities.size (); ++i)
            {
                Actor* actor = (Actor*)entities[i];

                if (!isPlayerDrawn && actor->ZOrder >= PLAYER_Z_ORDER)
                {
                    m_SceneManager->GetPlayer ().Render (deltaTime);
                    isPlayerDrawn = true;
                }

                actor->RenderID = i;
                actor->Render (deltaTime);

                if (m_SceneManager->IsDrawBoundingBox ())
                {
                    actor->DrawBounds ();
                }

                if (m_SceneManager->IsDrawPhysData ())
                {
                    actor->DrawPhysBody ();
                }
            }
        }

        if (!isPlayerDrawn)
        {
            m_SceneManager->GetPlayer ().Render (deltaTime);
        }

        if (m_SceneManager->IsDrawBoundingBox ())
        {
            m_SceneManager->GetPlayer ().DrawBounds ();
        }

        if (m_SceneManager->IsDrawPhysData ())
        {
            m_SceneManager->GetPlayer ().DrawPhysBody ();
        }

        if (m_SceneManager->GetMainLoop ()->GetStateManager ().GetActiveStateName () != "EDITOR_STATE")
        {
            m_SceneManager->GetCamera ().UseIdentityTransform ();

            Font& font = m_SceneManager->GetMainLoop ()->GetScreen ()->GetFont ();
            font.DrawText (FONT_NAME_SMALL, al_map_rgb (0, 255, 0), 10, 10, ToString (entities.size ()),
                           ALLEGRO_ALIGN_LEFT);
        }

        al_draw_filled_circle (0, 0, 5, COLOR_RED);
    }

    //--------------------------------------------------------------------------------------------------

    std::vector<Entity*> Scene::GetVisibleEntities ()
    {
        Point cameraPos = m_SceneManager->GetCamera ().GetTranslate ();

        if (cameraPos != m_VisibleLastCameraPos)
        {
            Point size = m_SceneManager->GetCamera ().GetScale ()
                * m_SceneManager->GetMainLoop ()->GetScreen ()->GetWindowSize ();
            float offsetMultiplier = 0.5f;

            m_VisibleEntities = m_QuadTree.GetEntitiesWithinRect (
                Rect (cameraPos - size * offsetMultiplier, cameraPos + size + size * offsetMultiplier));

            std::sort (m_VisibleEntities.begin (), m_VisibleEntities.end (), Entity::CompareByZOrder);
        }

        m_VisibleLastCameraPos = cameraPos;

        return m_VisibleEntities;
    }

    //--------------------------------------------------------------------------------------------------

    void Scene::AddActor (const std::string& name, Actor* actor)
    {
        Actor* act = GetActor (name);

        if (!act)
        {
            m_Actors.push_back (actor);
            m_QuadTree.Insert (actor);
            m_QuadTree.UpdateStructures ();
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Scene::RemoveActor (const std::string& name)
    {
        for (int i = 0; i < m_Actors.size (); ++i)
        {
            if (m_Actors[i]->Name == name)
            {
                m_QuadTree.Remove (m_Actors[i]);
                m_QuadTree.UpdateStructures ();
                SAFE_DELETE (m_Actors[i]);
                m_Actors.erase (m_Actors.begin () + i);
                break;
            }
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

    std::vector<Actor*>& Scene::GetActors () { return m_Actors; }

    //--------------------------------------------------------------------------------------------------

    void Scene::AddTile (TileActor* tile)
    {
        m_Actors.push_back (tile);
        m_QuadTree.Insert (tile);
        m_QuadTree.UpdateStructures ();
    }

    //--------------------------------------------------------------------------------------------------

    void Scene::RemoveTile (TileActor* tile)
    {
        m_Actors.erase (std::remove (m_Actors.begin (), m_Actors.end (), tile), m_Actors.end ());
        m_QuadTree.Remove (tile);
        m_QuadTree.UpdateStructures ();
    }

    //--------------------------------------------------------------------------------------------------

    void Scene::SortActors ()
    {
        std::sort (m_Actors.begin (), m_Actors.end (), Entity::CompareByZOrder);
        UpdateRenderIDs ();
    }

    //--------------------------------------------------------------------------------------------------

    void Scene::UpdateRenderIDs ()
    {
        std::sort (m_Actors.begin (), m_Actors.end (), Entity::CompareByZOrder);

        for (int i = 0; i < m_Actors.size (); ++i)
        {
            m_Actors[i]->RenderID = i;
        }
    }

    //--------------------------------------------------------------------------------------------------

    std::vector<TileActor*> Scene::GetTiles ()
    {
        std::vector<TileActor*> tiles;

        for (Actor* actor : m_Actors)
        {
            if (actor->GetTypeName () == TileActor::TypeName)
            {
                tiles.push_back (dynamic_cast<TileActor*> (actor));
            }
        }

        return tiles;
    }

    //--------------------------------------------------------------------------------------------------

    void Scene::AddFlagPoint (const std::string& name, Point point)
    {
        if (m_FlagPoints.find (name) == m_FlagPoints.end ())
        {
            FlagPoint fp;
            fp.Name = name;
            fp.Pos = point;

            m_FlagPoints.insert (make_pair (name, fp));
        }
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

    void Scene::AddTriggerArea (const std::string& name, std::vector<Point> points)
    {
        if (m_TriggerAreas.find (name) == m_TriggerAreas.end ())
        {
            TriggerArea area{ name, points };
            area.UpdatePolygons (&m_SceneManager->GetMainLoop ()->GetPhysicsManager ().GetTriangulator ());

            m_TriggerAreas.insert (std::make_pair (name, area));
        }
    }

    //--------------------------------------------------------------------------------------------------

    std::map<std::string, TriggerArea>& Scene::GetTriggerAreas () { return m_TriggerAreas; }

    //--------------------------------------------------------------------------------------------------

    TriggerArea& Scene::GetTriggerArea (const std::string& name) { return m_TriggerAreas[name]; }

    //--------------------------------------------------------------------------------------------------

    void Scene::RemoveTriggerArea (const std::string& name) { m_TriggerAreas.erase (name); }

    //--------------------------------------------------------------------------------------------------

    bool Scene::AddSpeech (SpeechData data)
    {
        if (m_Speeches.find (data.Name) == m_Speeches.end ())
        {
            m_Speeches.insert (std::make_pair (data.Name, data));

            return true;
        }

        return false;
    }

    //--------------------------------------------------------------------------------------------------

    std::map<std::string, SpeechData>& Scene::GetSpeeches () { return m_Speeches; }

    //--------------------------------------------------------------------------------------------------

    SpeechData* Scene::GetSpeech (const std::string& name)
    {
        if (m_Speeches.find (name) != m_Speeches.end ())
        {
            return &m_Speeches[name];
        }

        return nullptr;
    }

    //--------------------------------------------------------------------------------------------------

    void Scene::RemoveSpeech (const std::string& name)
    {
        if (m_Speeches.find (name) != m_Speeches.end ())
        {
            m_Speeches.erase (name);
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Scene::Reset ()
    {
        RemoveAllScripts ();

        for (int i = 0; i < m_Actors.size (); ++i)
        {
            SAFE_DELETE (m_Actors[i]);
        }

        m_Actors.clear ();

        m_SceneManager->GetSpeechFrameManager ().Clear ();
        m_SceneManager->GetMainLoop ()->GetTweenManager ().Clear ();

        m_FlagPoints.clear ();
        m_TriggerAreas.clear ();

        m_QuadTree = QuadTreeNode (Rect ({ -boundSize, -boundSize }, { boundSize, boundSize }));

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
        font.DrawText (FONT_NAME_SMALL, al_map_rgb (255, 255, 0), bounds.GetCenter ().X, bounds.GetCenter ().Y,
                       ToString (node->GetData ().size ()), ALLEGRO_ALIGN_CENTER);

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

    void Scene::AddOnEnterCallback (const std::string& triggerName, std::function<void(float dx, float dy)> func)
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
            if (m_TriggerAreas[triggerName].ScriptOnEnterCallback)
            {
                m_TriggerAreas[triggerName].ScriptOnEnterCallback->Release ();
            }

            m_TriggerAreas[triggerName].ScriptOnEnterCallback = func;
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Scene::AddOnLeaveCallback (const std::string& triggerName, std::function<void(float dx, float dy)> func)
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
            if (m_TriggerAreas[triggerName].ScriptOnLeaveCallback)
            {
                m_TriggerAreas[triggerName].ScriptOnLeaveCallback->Release ();
            }

            m_TriggerAreas[triggerName].ScriptOnLeaveCallback = func;
        }
    }

    //--------------------------------------------------------------------------------------------------

    Rect Scene::GetRenderBounds (Entity* entity, bool drawOOBBox)
    {
        Point translate = m_SceneManager->GetCamera ().GetTranslate ();
        Point scale = m_SceneManager->GetCamera ().GetScale ();

        Rect b = entity->Bounds;
        int halfWidth = b.GetHalfSize ().Width;
        int halfHeight = b.GetHalfSize ().Height;

        float x1 = (b.GetPos ().X - translate.X * (1.0f / scale.X)) * (scale.X);
        float y1 = (b.GetPos ().Y - translate.Y * (1.0f / scale.Y)) * (scale.Y);
        float x2 = (b.GetPos ().X - translate.X * (1.0f / scale.X) + 2 * halfWidth) * (scale.X);
        float y2 = (b.GetPos ().Y - translate.Y * (1.0f / scale.Y) + 2 * halfHeight) * (scale.Y);

        Point origin = { x1 + (x2 - x1) * 0.5f, y1 + (y2 - y1) * 0.5f };
        Point pointA = RotatePoint (x1, y1, origin, entity->Rotation);
        Point pointB = RotatePoint (x1, y2, origin, entity->Rotation);
        Point pointC = RotatePoint (x2, y1, origin, entity->Rotation);
        Point pointD = RotatePoint (x2, y2, origin, entity->Rotation);

        if (drawOOBBox)
        {
            float vertices[] = { pointA.X, pointA.Y, pointB.X, pointB.Y, pointD.X, pointD.Y, pointC.X, pointC.Y };
            ALLEGRO_COLOR color = { 0.5f, 0.0f, 0.0f, 1.0f };
            al_draw_polygon (vertices, 4, ALLEGRO_LINE_JOIN_BEVEL, color, 2, 0);
        }

        float minX = std::min (pointA.X, std::min (pointB.X, std::min (pointC.X, pointD.X)));
        float minY = std::min (pointA.Y, std::min (pointB.Y, std::min (pointC.Y, pointD.Y)));
        float maxX = std::max (pointA.X, std::max (pointB.X, std::max (pointC.X, pointD.X)));
        float maxY = std::max (pointA.Y, std::max (pointB.Y, std::max (pointC.Y, pointD.Y)));

        return { { minX, minY }, { maxX, maxY } };
    }

    //--------------------------------------------------------------------------------------------------

    Actor* Scene::GetCurrentlyProcessedActor () { return m_CurrentActor; }

    //--------------------------------------------------------------------------------------------------

    void Scene::ResetAllActorsPositions ()
    {
        for (Actor* actor : m_Actors)
        {
            actor->Bounds = actor->TemplateBounds;
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
}
