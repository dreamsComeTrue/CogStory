// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "Scene.h"
#include "Atlas.h"
#include "AtlasManager.h"
#include "MainLoop.h"
#include "SceneManager.h"
#include "Screen.h"

#include "addons/json/json.hpp"
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
    //--------------------------------------------------------------------------------------------------

    auto StringToVector = [](std::string in) -> std::vector<Point> {
        size_t count = 0;
        const char* delimiter = " ";
        std::vector<Point> nums;
        char* str = const_cast<char*> (in.c_str ());

        for (char* pch = strtok (str, delimiter); pch != NULL; pch = strtok (NULL, delimiter))
        {
            float x = atof (pch);

            pch = strtok (NULL, delimiter);

            float y = atof (pch);

            nums.push_back ({ x, y });
        }

        return nums;
    };

    //--------------------------------------------------------------------------------------------------

    auto StringToPoint = [](std::string in) -> Point {
        std::vector<Point> vec = StringToVector (in);

        return vec[0];
    };

    //--------------------------------------------------------------------------------------------------

    auto PointToString = [](Point in) -> std::string { return ToString (in.X) + " " + ToString (in.Y); };

    //--------------------------------------------------------------------------------------------------

    auto VectorToString = [](std::vector<Point>& points) -> std::string {
        std::string out;

        for (Point& p : points)
        {
            out += PointToString (p) + " ";
        }

        return out;
    };

    //--------------------------------------------------------------------------------------------------

    void Tile::Draw (AtlasManager* atlasManager)
    {
        Atlas* atlas = atlasManager->GetAtlas (Tileset);

        if (atlas)
        {
            atlas->DrawRegion (Name, Bounds.Transform.Pos.X, Bounds.Transform.Pos.Y, 1, 1, DegressToRadians (Rotation));
        }
    }

    //--------------------------------------------------------------------------------------------------

    const float boundSize = 10000;

    Scene::Scene (SceneManager* sceneManager)
        : Scriptable (&sceneManager->GetMainLoop ()->GetScriptManager ())
        , m_SceneManager (sceneManager)
        , m_DrawPhysData (true)
        , m_QuadTree (Rect ({ -boundSize, -boundSize }, { boundSize, boundSize }))
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
        for (int i = 0; i < m_Tiles.size (); ++i)
        {
            delete m_Tiles[i];
        }

        return Lifecycle::Destroy ();
    }

    //--------------------------------------------------------------------------------------------------
    Script* script;
    Scene* Scene::LoadScene (SceneManager* sceneManager, const std::string& filePath)
    {
        try
        {
            Scene* scene = new Scene (sceneManager);
            std::ifstream file (filePath.c_str ());
            json j;
            file >> j;
            file.close ();

            scene->m_Name = j["name"];
            scene->m_Size = StringToPoint (j["size"]);

            auto& scripts = j["scripts"];

            for (auto& j_tile : scripts)
            {
                std::string name = j_tile["name"];
                std::string path = j_tile["path"];

                script = sceneManager->GetMainLoop ()->GetScriptManager ().LoadScriptFromFile (
                    GetDataPath () + "scripts/" + path, name);
                scene->AttachScript (script, path);
            }

            auto& tiles = j["tiles"];

            for (auto& j_tile : tiles)
            {
                Tile* tile = new Tile (&sceneManager->GetMainLoop ()->GetPhysicsManager ());

                std::string id = j_tile["id"];
                tile->ID = atoi (id.c_str ());
                tile->Tileset = j_tile["tileset"];
                tile->Name = j_tile["name"];
                tile->Bounds.Transform.Pos = StringToPoint (j_tile["pos"]);
                tile->Bounds.Transform.Size = sceneManager->GetAtlasManager ()
                                                  ->GetAtlas (tile->Tileset)
                                                  ->GetRegion (tile->Name)
                                                  .Bounds.Transform.Size;
                std::string zOrder = j_tile["z-order"];
                tile->ZOrder = atoi (zOrder.c_str ());
                std::string rot = j_tile["rot"];
                tile->Rotation = atof (rot.c_str ());

                //  Physics
                if (!j_tile["phys"].is_null ())
                {
                    auto& physTiles = j_tile["phys"];

                    for (auto& physTile : physTiles)
                    {
                        tile->PhysPoints.push_back (StringToVector (physTile["poly"]));
                    }

                    tile->SetPhysOffset (tile->Bounds.Transform.Pos);
                }

                scene->AddTile (tile);
            }

            auto& flag_points = j["flag_points"];

            for (auto& flag_point : flag_points)
            {
                std::string name = flag_point["name"];
                Point pos = StringToPoint (flag_point["pos"]);

                scene->AddFlagPoint (name, pos);
            }

            auto& triggerAreas = j["trigger_areas"];

            for (auto& triggerArea : triggerAreas)
            {
                std::string name = triggerArea["name"];
                std::vector<Point> poly = StringToVector (triggerArea["poly"]);

                scene->AddTriggerArea (name, poly);
            }

            return scene;
        }
        catch (const std::exception&)
        {
            return nullptr;
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Scene::SaveScene (Scene* scene, const std::string& filePath)
    {
        try
        {
            json j;

            j["name"] = scene->m_Name;
            j["size"] = PointToString (scene->m_Size);

            j["scripts"] = json::array ({});

            for (std::vector<ScriptMetaData>::iterator it = scene->m_Scripts.begin (); it != scene->m_Scripts.end ();
                 ++it)
            {
                json scriptObj = json::object ({});

                scriptObj["name"] = it->Name;
                scriptObj["path"] = it->Path;

                j["scripts"].push_back (scriptObj);
            }

            j["tiles"] = json::array ({});

            for (Tile* tile : scene->m_Tiles)
            {
                json tileObj = json::object ({});

                tileObj["id"] = ToString (tile->ID);
                tileObj["tileset"] = tile->Tileset;
                tileObj["name"] = tile->Name;
                tileObj["pos"] = PointToString (tile->Bounds.Transform.Pos);
                tileObj["z-order"] = ToString (tile->ZOrder);
                tileObj["rot"] = ToString (tile->Rotation);

                tileObj["phys"] = json::array ({});

                for (int i = 0; i < tile->PhysPoints.size (); ++i)
                {
                    json physObj = json::object ({});

                    physObj["poly"] = VectorToString (tile->PhysPoints[i]);

                    tileObj["phys"].push_back (physObj);
                }

                j["tiles"].push_back (tileObj);
            }

            j["flag_points"] = json::array ({});

            for (std::map<std::string, Point>::iterator it = scene->m_FlagPoints.begin ();
                 it != scene->m_FlagPoints.end (); ++it)
            {
                json flagObj = json::object ({});

                flagObj["name"] = it->first;
                flagObj["pos"] = PointToString (it->second);

                j["flag_points"].push_back (flagObj);
            }

            j["trigger_areas"] = json::array ({});

            for (std::map<std::string, TriggerArea>::iterator it = scene->m_TriggerAreas.begin ();
                 it != scene->m_TriggerAreas.end (); ++it)
            {
                json triggerObj = json::object ({});

                triggerObj["name"] = it->second.Name;
                triggerObj["poly"] = VectorToString (it->second.Points);

                j["trigger_areas"].push_back (triggerObj);
            }

            // write prettified JSON to another file
            std::ofstream out (filePath);
            out << std::setw (4) << j.dump (4, ' ') << "\n";
        }
        catch (const std::exception& e)
        {
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Scene::BeforeEnter ()
    {
        m_SceneManager->GetPlayer ().BeforeEnter ();
        RunAllScripts ("void BeforeEnterScene ()");
    }

    //--------------------------------------------------------------------------------------------------

    void Scene::AfterLeave ()
    {
        m_SceneManager->GetPlayer ().AfterLeave ();
        RunAllScripts ("void AfterLeaveScene ()");
    }

    //--------------------------------------------------------------------------------------------------

    void Scene::Update (float deltaTime)
    {
        m_SceneManager->GetPlayer ().Update (deltaTime);
        UpdateScripts (deltaTime);
    }

    //--------------------------------------------------------------------------------------------------

    void Scene::Render (float deltaTime)
    {
        m_SceneManager->GetCamera ().Update (deltaTime);

        if (false)
        {
            DrawQuadTree (&m_QuadTree);
        }

        bool isPlayerDrawn = false;

        for (int i = 0; i < m_Tiles.size (); ++i)
        {
            Tile* tile = m_Tiles[i];

            if (!isPlayerDrawn && tile->ZOrder >= PLAYER_Z_ORDER)
            {
                m_SceneManager->GetPlayer ().Render (deltaTime);
                isPlayerDrawn = true;
            }

            tile->RenderID = i;
            tile->Draw (m_SceneManager->GetAtlasManager ());

            if (m_DrawPhysData)
            {
                tile->DrawPhysVertices ();
            }
        }

        if (!isPlayerDrawn)
        {
            m_SceneManager->GetPlayer ().Render (deltaTime);
        }

        if (m_DrawPhysData)
        {
            m_SceneManager->GetPlayer ().DrawPhysVertices ();
        }

        m_SceneManager->GetMainLoop ()->GetScreen ()->GetFont ().DrawText (
            FONT_NAME_MAIN_SMALL, al_map_rgb (0, 255, 0), -100, -50, m_Name, ALLEGRO_ALIGN_LEFT);
    }

    //--------------------------------------------------------------------------------------------------

    void Scene::AddTile (Tile* tile)
    {
        m_Tiles.push_back (tile);
        m_QuadTree.Insert (tile);
    }

    //--------------------------------------------------------------------------------------------------

    void Scene::RemoveTile (Tile* tile)
    {
        m_Tiles.erase (std::remove (m_Tiles.begin (), m_Tiles.end (), tile), m_Tiles.end ());
        m_QuadTree.Remove (tile);
    }

    //--------------------------------------------------------------------------------------------------

    void Scene::SortTiles () { std::sort (m_Tiles.begin (), m_Tiles.end (), Tile::CompareByZOrder); }

    //--------------------------------------------------------------------------------------------------

    std::vector<Tile*>& Scene::GetTiles () { return m_Tiles; }

    //--------------------------------------------------------------------------------------------------

    void Scene::AddFlagPoint (const std::string& name, Point point) { m_FlagPoints.insert (make_pair (name, point)); }

    //--------------------------------------------------------------------------------------------------

    Point Scene::GetFlagPoint (const std::string& name)
    {
        if (!m_FlagPoints.empty ())
        {
            std::map<std::string, Point>::iterator it = m_FlagPoints.find (name);

            if (it != m_FlagPoints.end ())
            {
                return (*it).second;
            }
        }

        return {};
    }

    //--------------------------------------------------------------------------------------------------

    std::map<std::string, Point>& Scene::GetFlagPoints () { return m_FlagPoints; }

    //--------------------------------------------------------------------------------------------------

    void Scene::AddTriggerArea (const std::string& name, std::vector<Point> points)
    {
        TriggerArea area{ name, points };
        area.UpdatePolygons (&m_SceneManager->GetMainLoop ()->GetPhysicsManager ().GetTriangulator ());

        m_TriggerAreas.insert (std::make_pair (name, area));
    }

    //--------------------------------------------------------------------------------------------------

    std::map<std::string, TriggerArea>& Scene::GetTriggerAreas () { return m_TriggerAreas; }

    //--------------------------------------------------------------------------------------------------

    TriggerArea& Scene::GetTriggerArea (const std::string& name) { return m_TriggerAreas[name]; }

    //--------------------------------------------------------------------------------------------------

    void Scene::RemoveTriggerArea (const std::string& name) { m_TriggerAreas.erase (name); }

    //--------------------------------------------------------------------------------------------------

    void Scene::Reset ()
    {
        m_Tiles.clear ();
        m_FlagPoints.clear ();
        m_TriggerAreas.clear ();
    }

    //--------------------------------------------------------------------------------------------------

    std::string Scene::GetName () { return m_Name; }

    //--------------------------------------------------------------------------------------------------

    void Scene::SetDrawPhysData (bool enable) { m_DrawPhysData = enable; }

    //--------------------------------------------------------------------------------------------------

    bool Scene::IsDrawPhysData () { return m_DrawPhysData; }

    //--------------------------------------------------------------------------------------------------

    QuadTreeNode& Scene::GetQuadTree () { return m_QuadTree; }

    //--------------------------------------------------------------------------------------------------

    void Scene::DrawQuadTree (QuadTreeNode* node)
    {
        Rect bounds = node->GetBounds ();
        al_draw_rectangle (bounds.Dim.TopLeft.X, bounds.Dim.TopLeft.Y, bounds.Dim.BottomRight.X,
            bounds.Dim.BottomRight.Y, COLOR_WHITE, 1);

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

    void Scene::AddTriggerCallback (const std::string& triggerName, std::function<void(float dx, float dy)> func)
    {
        if (m_TriggerAreas.find (triggerName) != m_TriggerAreas.end ())
        {
            m_TriggerAreas[triggerName].TriggerCallback = func;
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Scene::AddTriggerCallback (const std::string& triggerName, asIScriptFunction* func)
    {
        if (m_TriggerAreas.find (triggerName) != m_TriggerAreas.end ())
        {
            m_TriggerAreas[triggerName].ScriptTriggerCallback = func;
        }
    }

    //--------------------------------------------------------------------------------------------------
}
