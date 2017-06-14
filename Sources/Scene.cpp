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

        auto stringToPoint = [](std::string in) -> Point {
            size_t pos = 0, count = 0;
            std::string token, delimiter = " ";
            double nums[2] = { 0, 0 };

            while ((pos = in.find (delimiter)) != std::string::npos)
            {
                token = in.substr (0, pos);
                in.erase (0, pos + delimiter.length ());
                nums[count] = atof (token.c_str ());

                ++count;
            }

            return Point (nums[0], nums[1]);
        };

        scene->m_Name = j["name"];
        scene->m_Size = stringToPoint (j["size"]);

        return scene;
    }

    //--------------------------------------------------------------------------------------------------

    void Scene::BeforeEnter ()
    {
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
        m_SceneManager->GetScreen ()->GetFont ().DrawText (FONT_NAME_MAIN, al_map_rgb (255, 255, 255), 200, 200, "Robot Tale");
        m_SceneManager->GetScreen ()->GetFont ().DrawText (FONT_NAME_MAIN, al_map_rgb (0, 255, 0), 0, 0, m_Name, ALLEGRO_ALIGN_LEFT);
    }

    //--------------------------------------------------------------------------------------------------
}
