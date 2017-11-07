// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "SceneManager.h"
#include "AtlasManager.h"
#include "Common.h"
#include "MainLoop.h"
#include "Scene.h"
#include "Screen.h"

#include <algorithm>

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    SceneManager::SceneManager (MainLoop* mainLoop)
        : m_ActiveScene (nullptr)
        , m_MainLoop (mainLoop)
        , m_Player (this)
        , m_Camera (mainLoop->GetScreen ())
        , m_AtlasManager (nullptr)
    {
    }

    //--------------------------------------------------------------------------------------------------

    SceneManager::~SceneManager ()
    {
        if (!IsDestroyed ())
        {
            Destroy ();
        }
    }

    //--------------------------------------------------------------------------------------------------

    bool SceneManager::Initialize ()
    {
        m_AtlasManager = new AtlasManager ();
        m_AtlasManager->Initialize ();

        m_Player.Initialize ();
        m_Player.MoveCallback = [&](float dx, float dy) {
            Point scale = m_Camera.GetScale ();
            Point screenSize = m_MainLoop->GetScreen ()->GetWindowSize ();
            Point playerSize = m_Player.GetSize ();
            Point playerPosition = m_Player.GetPosition ();

            m_Camera.SetTranslate (screenSize.Width * 0.5 - playerSize.Width * 0.5 - playerPosition.X * scale.X,
                screenSize.Height * 0.5 - playerSize.Height * 0.5 - playerPosition.Y * scale.Y);
        };

        Lifecycle::Initialize ();
    }

    //--------------------------------------------------------------------------------------------------

    bool SceneManager::Destroy ()
    {
        SAFE_DELETE (m_AtlasManager);

        m_Player.Destroy ();

        for (std::map<ResourceID, Scene*>::iterator it = m_Scenes.begin (); it != m_Scenes.end (); it++)
        {
            SAFE_DELETE (it->second);
        }

        Lifecycle::Destroy ();
    }

    //--------------------------------------------------------------------------------------------------

    void SceneManager::AddScene (ResourceID id, Scene* scene)
    {
        if (m_Scenes.find (id) == m_Scenes.end ())
        {
            m_Scenes.insert (std::make_pair (id, scene));
        }
    }

    //--------------------------------------------------------------------------------------------------

    void SceneManager::RemoveScene (Scene* scene)
    {
        for (std::map<ResourceID, Scene*>::iterator it = m_Scenes.begin (); it != m_Scenes.end (); it++)
        {
            if (it->second == scene)
            {
                m_Scenes.erase (it);
                break;
            }
        }
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

    Scene* SceneManager::GetActiveScene () { return m_ActiveScene; }

    //--------------------------------------------------------------------------------------------------

    void SceneManager::ProcessEvent (ALLEGRO_EVENT* event, float deltaTime) {}

    //--------------------------------------------------------------------------------------------------

    bool SceneManager::Update (float deltaTime)
    {
        if (m_ActiveScene != nullptr)
        {
            m_ActiveScene->Update (deltaTime);
        }

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    void SceneManager::Render (float deltaTime)
    {
        if (m_ActiveScene != nullptr)
        {
            m_ActiveScene->Render (deltaTime);
        }
    }

    //--------------------------------------------------------------------------------------------------

    Player& SceneManager::GetPlayer () { return m_Player; }

    //--------------------------------------------------------------------------------------------------

    Camera& SceneManager::GetCamera () { return m_Camera; }

    //--------------------------------------------------------------------------------------------------

    MainLoop* SceneManager::GetMainLoop () { return m_MainLoop; }

    //--------------------------------------------------------------------------------------------------

    AtlasManager* SceneManager::GetAtlasManager () { return m_AtlasManager; }

    //--------------------------------------------------------------------------------------------------

    void SceneManager::AddTriggerCallback (const std::string& triggerName, std::function<void(float dx, float dy)> func)
    {
        if (m_ActiveScene)
        {
            m_ActiveScene->AddTriggerCallback (triggerName, func);
        }
    }

    //--------------------------------------------------------------------------------------------------

    void SceneManager::AddTriggerCallback (const std::string& triggerName, asIScriptFunction* func)
    {
        if (m_ActiveScene)
        {
            m_ActiveScene->AddTriggerCallback (triggerName, func);
        }
    }

    //--------------------------------------------------------------------------------------------------

    Point SceneManager::GetFlagPoint (const std::string& name)
    {
        if (m_ActiveScene)
        {
            return m_ActiveScene->GetFlagPoint (name);
        }

        return { 0, 0 };
    }

    //--------------------------------------------------------------------------------------------------
}
