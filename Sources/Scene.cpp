// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "Scene.h"
#include "SceneManager.h"
#include "Screen.h"

#include "addons/json/json.hpp"

#include <fstream>

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

    bool Scene::Initialize ()
    {
        Lifecycle::Initialize ();
    }

    //--------------------------------------------------------------------------------------------------

    bool Scene::Destroy ()
    {
        for (Tile& tile : m_Tiles)
        {
            al_destroy_bitmap (tile.Image);
        }

        Lifecycle::Destroy ();
    }

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
            tile.FileName = j_tile["img"];

            std::string filePath = GetDataPath () + std::string ("gfx/") + tile.FileName;
            tile.Image = al_load_bitmap (filePath.c_str ());
            tile.Pos = StringToPoint (j_tile["pos"]);

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

    void Scene::BeforeEnter ()
    {
        Point pos = m_SpawnPoints["DEFAULT"];
        m_SceneManager->GetPlayer ().SetPosition (pos);
    }

    //--------------------------------------------------------------------------------------------------

    void Scene::AfterLeave ()
    {
    }

    //--------------------------------------------------------------------------------------------------

    void Scene::Update (double deltaTime)
    {
    }

    //--------------------------------------------------------------------------------------------------

    void Scene::Render (double deltaTime)
    {
        for (Tile& tile : m_Tiles)
        {
            al_draw_bitmap (tile.Image, tile.Pos.X, tile.Pos.Y, 0);
        }

        m_SceneManager->GetScreen ()->GetFont ().DrawText (FONT_NAME_MAIN, al_map_rgb (255, 255, 255), 200, 200, "Robot Tale");
        m_SceneManager->GetScreen ()->GetFont ().DrawText (FONT_NAME_MAIN, al_map_rgb (0, 255, 0), 0, 0, m_Name, ALLEGRO_ALIGN_LEFT);
    }

    //--------------------------------------------------------------------------------------------------
}
