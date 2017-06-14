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
        LoadScenes ();

        Lifecycle::Initialize ();
    }

    //--------------------------------------------------------------------------------------------------

    bool SceneManager::Destroy ()
    {
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

    bool SceneManager::Update (double deltaTime)
    {
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
}
