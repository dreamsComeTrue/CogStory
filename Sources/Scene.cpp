// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "Scene.h"
#include "Atlas.h"
#include "AtlasManager.h"
#include "MainLoop.h"
#include "SceneManager.h"
#include "Screen.h"

#include "addons/json/json.hpp"

using json = nlohmann::json;

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    /*
     *
     *  {
     *      "name": "Home",
     *      "spawn_points" :
     *      [
     *          "SPAWN_1" : "0 0"
     *      ]
     *  }
     *
     */

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
      : m_SceneManager (sceneManager)
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

        Lifecycle::Destroy ();
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

                Script* script =
                  sceneManager->GetMainLoop ()->GetScriptManager ().LoadScriptFromFile (GetDataPath () + "scripts/" + path, name);
                scene->AttachScript (script, path);
            }

            auto& tiles = j["tiles"];

            for (auto& j_tile : tiles)
            {
                Tile* tile = new Tile ();

                std::string id = j_tile["id"];
                tile->ID = atoi (id.c_str ());
                tile->Tileset = j_tile["tileset"];
                tile->Name = j_tile["name"];
                tile->Bounds.Transform.Pos = StringToPoint (j_tile["pos"]);
                tile->Bounds.Transform.Size =
                  sceneManager->GetAtlasManager ()->GetAtlas (tile->Tileset)->GetRegion (tile->Name).Bounds.Transform.Size;
                std::string zOrder = j_tile["z-order"];
                tile->ZOrder = atoi (zOrder.c_str ());
                std::string rot = j_tile["rot"];
                tile->Rotation = atof (rot.c_str ());

                //  Physics
                if (!j_tile["phys"].is_null ())
                {
                    tile->PhysPoints = StringToVector (j_tile["phys"]);
                }

                tile->SetPhysOffset (tile->Bounds.Transform.Pos);

                scene->AddTile (tile);
            }

            auto& spawn_points = j["spawn_points"];

            for (auto& spawn_point : spawn_points)
            {
                std::string name = spawn_point["name"];
                Point pos = StringToPoint (spawn_point["pos"]);

                scene->m_SpawnPoints.insert (make_pair (name, pos));
            }

            return scene;
        }
        catch (const std::exception& e)
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

            for (std::map<ScriptMetaData, Script*>::iterator it = scene->m_Scripts.begin (); it != scene->m_Scripts.end (); ++it)
            {
                json scriptObj = json::object ({});

                scriptObj["name"] = it->first.Name;
                scriptObj["path"] = it->first.Path;

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
                tileObj["phys"] = VectorToString (tile->PhysPoints);

                j["tiles"].push_back (tileObj);
            }

            j["spawn_points"] = json::array ({});

            for (std::map<std::string, Point>::iterator it = scene->m_SpawnPoints.begin (); it != scene->m_SpawnPoints.end (); ++it)
            {
                json spawnObj = json::object ({});

                spawnObj["name"] = it->first;
                spawnObj["pos"] = PointToString (it->second);

                j["spawn_points"].push_back (spawnObj);
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
        m_SceneManager->GetPlayer ().CreatePhysics (this);
        RunAllScripts ("void BeforeEnterScene ()");
    }

    //--------------------------------------------------------------------------------------------------

    void Scene::AfterLeave () { m_SceneManager->GetPlayer ().DestroyPhysics (this); }

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

        DrawQuadTree (&m_QuadTree);

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
          FONT_NAME_MAIN, al_map_rgb (0, 255, 0), -100, -50, m_Name, ALLEGRO_ALIGN_LEFT);
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

    Point Scene::GetSpawnPoint (const std::string& name)
    {
        std::map<std::string, Point>::iterator it = m_SpawnPoints.find (name);

        if (it != m_SpawnPoints.end ())
        {
            return (*it).second;
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Scene::Reset () { m_Tiles.clear (); }

    //--------------------------------------------------------------------------------------------------

    std::string Scene::GetName () { return m_Name; }

    //--------------------------------------------------------------------------------------------------

    void Scene::SetDrawPhysData (bool enable) { m_DrawPhysData = enable; }

    //--------------------------------------------------------------------------------------------------

    QuadTreeNode& Scene::GetQuadTree () { return m_QuadTree; }

    //--------------------------------------------------------------------------------------------------

    void Scene::DrawQuadTree (QuadTreeNode* node)
    {
        Rect bounds = node->GetBounds ();
        al_draw_rectangle (bounds.Dim.TopLeft.X, bounds.Dim.TopLeft.Y, bounds.Dim.BottomRight.X, bounds.Dim.BottomRight.Y, COLOR_WHITE, 1);

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
}
