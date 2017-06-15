// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "SceneManager.h"
#include "Common.h"
#include "Scene.h"
#include "Screen.h"

#include <algorithm>

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    SceneManager::SceneManager (Screen* screen)
        : m_ActiveScene (nullptr)
        , m_Screen (screen)
        , m_Player (screen)
        , m_Camera (screen)
    {
    }

    //--------------------------------------------------------------------------------------------------

    SceneManager::~SceneManager ()
    {
        if (!IsDestroyed ())
        {
            Destroy ();
        }

        Lifecycle::Destroy ();
    }

    //--------------------------------------------------------------------------------------------------

    bool SceneManager::Initialize ()
    {
        m_Player.Initialize ();

        const Point& screenSize = m_Screen->GetScreenSize ();
        Point& playerSize = m_Player.GetSize ();
        m_Camera.SetOffset (screenSize.Width * 0.5 - playerSize.Width * 0.5,
            screenSize.Height * 0.5 - playerSize.Height * 0.5);

        m_Player.MoveCallback = [&](double dx, double dy) {
            m_Camera.Move (-dx, -dy);
        };

        LoadScenes ();

        Lifecycle::Initialize ();
    }

    //--------------------------------------------------------------------------------------------------

    bool SceneManager::Destroy ()
    {
        m_Player.Destroy ();

        for (std::map<ResourceID, Scene*>::iterator it = m_Scenes.begin (); it != m_Scenes.end (); it++)
        {
            SAFE_DELETE (it->second);
        }

        Lifecycle::Destroy ();
    }

    //--------------------------------------------------------------------------------------------------

    void SceneManager::SetActiveScene (Scene* scene)
    {
        if (m_ActiveScene != nullptr)
        {
            m_ActiveScene->AfterLeave ();
        }

        m_ActiveScene = scene;
        m_ActiveScene->BeforeEnter ();
    }

    //--------------------------------------------------------------------------------------------------

    void SceneManager::ProcessEvent (ALLEGRO_EVENT* event, double deltaTime)
    {
        m_Player.ProcessEvent (event, deltaTime);
    }

    //--------------------------------------------------------------------------------------------------

    bool SceneManager::Update (double deltaTime)
    {
        m_Player.Update (deltaTime);
        m_Camera.Update (deltaTime);

        if (m_ActiveScene != nullptr)
        {
            m_ActiveScene->Update (deltaTime);
        }

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    void SceneManager::Render (double deltaTime)
    {
        if (m_ActiveScene != nullptr)
        {
            m_ActiveScene->Render (deltaTime);
        }

        m_Player.Render (deltaTime);
    }

    //--------------------------------------------------------------------------------------------------

    void SceneManager::LoadScenes ()
    {
        Scene* scene00 = Scene::LoadScene (this, GetResourcePath (SCENE_0_0));
        m_Scenes.insert (std::make_pair (SCENE_0_0, scene00));

        SetActiveScene (scene00);
    }

    //--------------------------------------------------------------------------------------------------

    Screen* SceneManager::GetScreen () { return m_Screen; }

    //--------------------------------------------------------------------------------------------------

    Player& SceneManager::GetPlayer () { return m_Player; }

    //--------------------------------------------------------------------------------------------------
}
