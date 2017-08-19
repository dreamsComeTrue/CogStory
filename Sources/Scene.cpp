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

    auto StringToPoint = [](std::string in) -> Point {
        size_t count = 0;
        const char* delimiter = " ";
        double nums[2] = { 0, 0 };
        char* str = const_cast<char*> (in.c_str ());

        for (char* pch = strtok (str, delimiter); pch != NULL; pch = strtok (NULL, delimiter))
        {
            nums[count++] = atof (pch);
        }

        return Point (nums[0], nums[1]);
    };

    //--------------------------------------------------------------------------------------------------

    void Tile::Draw (AtlasManager* atlasManager)
    {
        Atlas* atlas = atlasManager->GetAtlas (Tileset);

        if (atlas)
        {
            atlas->DrawRegion (Name, Pos.X, Pos.Y, 1, 1, DegressToRadians (Rotation));
        }
    }

    //--------------------------------------------------------------------------------------------------

    Scene::Scene (SceneManager* sceneManager)
      : m_SceneManager (sceneManager)
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

    bool Scene::Destroy () { return Lifecycle::Destroy (); }

    //--------------------------------------------------------------------------------------------------

    Scene* Scene::LoadScene (SceneManager* sceneManager, const std::string& filePath)
    {
        Scene* scene = new Scene (sceneManager);
        std::ifstream file (filePath.c_str ());
        json j;
        file >> j;
        file.close ();

        scene->m_Name = j["name"];
        scene->m_Size = StringToPoint (j["size"]);

        auto& tiles = j["tiles"];

        for (auto& j_tile : tiles)
        {
            Tile tile;
            tile.Tileset = j_tile["tileset"];
            tile.Name = j_tile["name"];
            tile.Pos = StringToPoint (j_tile["pos"]);
            std::string rot = j_tile["rot"];
            tile.Rotation = atof (rot.c_str ());

            scene->m_Tiles.push_back (tile);
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

    //--------------------------------------------------------------------------------------------------

    void Scene::BeforeEnter () { RunAllScripts ("void BeforeEnterScene ()"); }

    //--------------------------------------------------------------------------------------------------

    void Scene::AfterLeave () {}

    //--------------------------------------------------------------------------------------------------

    void Scene::Update (double deltaTime)
    {
        m_SceneManager->GetPlayer ().Update (deltaTime);
        m_SceneManager->GetCamera ().Update (deltaTime);
        UpdateScripts (deltaTime);
    }

    //--------------------------------------------------------------------------------------------------

    void Scene::Render (double deltaTime)
    {
        for (Tile& tile : m_Tiles)
        {
            tile.Draw (m_SceneManager->GetAtlasManager ());
        }

        m_SceneManager->GetMainLoop ()->GetScreen ()->GetFont ().DrawText (
          FONT_NAME_MAIN, al_map_rgb (255, 255, 255), 200, 200, "Robot Tale");
        m_SceneManager->GetMainLoop ()->GetScreen ()->GetFont ().DrawText (
          FONT_NAME_MAIN, al_map_rgb (0, 255, 0), 0, 0, m_Name, ALLEGRO_ALIGN_LEFT);

        m_SceneManager->GetPlayer ().Render (deltaTime);
        m_SceneManager->GetCamera ().Reset ();
    }

    //--------------------------------------------------------------------------------------------------

    void Scene::AddTile (Tile& tile) { m_Tiles.push_back (tile); }

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
}
