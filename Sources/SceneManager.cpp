// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "SceneManager.h"
#include "ActorFactory.h"
#include "AtlasManager.h"
#include "Common.h"
#include "MainLoop.h"
#include "Scene.h"
#include "SceneLoader.h"
#include "Screen.h"

#include <algorithm>

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    const int SCENE_FADE_ID = 99999;

    //--------------------------------------------------------------------------------------------------

    SceneManager::SceneManager (MainLoop* mainLoop)
        : m_MainLoop (mainLoop)
        , m_Camera (mainLoop->GetScreen ())
        , m_Player (this)
        , m_SpeechFrameManager (this)
        , m_AtlasManager (nullptr)
        , m_ActiveScene (nullptr)
        , m_Transitioning (true)
        , m_FadeColor (COLOR_BLACK)
        , m_TweenFade (nullptr)
        , m_DrawPhysData (true)
        , m_DrawBoundingBox (true)
        , m_DrawActorsNames (true)
        , m_NextScene (nullptr)
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
        Lifecycle::Initialize ();

        ActorFactory::RegisterActorTypes ();
        ActorFactory::RegisterAnimations ();

        m_AtlasManager = new AtlasManager ();
        m_AtlasManager->Initialize ();

        m_Player.Initialize ();
        m_Player.SetCheckOverlap (true);

        m_Camera.SetFollowActor (&m_Player);

        m_SpeechFrameManager.Initialize ();

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    bool SceneManager::Destroy ()
    {
        SAFE_DELETE (m_AtlasManager);

        m_Player.Destroy ();
        m_SpeechFrameManager.Destroy ();

        for (std::map<std::string, Scene*>::iterator it = m_Scenes.begin (); it != m_Scenes.end ();)
        {
            SAFE_DELETE (it->second);
            m_Scenes.erase (it++);
        }

        return Lifecycle::Destroy ();
    }

    //--------------------------------------------------------------------------------------------------

    void SceneManager::AddScene (Scene* scene)
    {
        if (m_Scenes.find (scene->GetPath ()) == m_Scenes.end ())
        {
            m_Scenes.insert (std::make_pair (scene->GetPath (), scene));
        }
    }

    //--------------------------------------------------------------------------------------------------

    void SceneManager::RemoveScene (Scene* scene)
    {
        for (std::map<std::string, Scene*>::iterator it = m_Scenes.begin (); it != m_Scenes.end ();)
        {
            if (it->second == scene)
            {
                if (it->second == m_ActiveScene)
                {
                    m_ActiveScene->AfterLeave ();
                    m_ActiveScene = nullptr;
                }

                SAFE_DELETE (it->second);
                m_Scenes.erase (it);

                break;
            }
            else
            {
                it++;
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    void SceneManager::SetActiveScene (Scene* scene)
    {
        if (scene)
        {
            if (m_ActiveScene != nullptr)
            {
                m_ActiveScene->AfterLeave ();
            }

            m_ActiveScene = scene;
            m_ActiveScene->BeforeEnter ();
        }
    }

    //--------------------------------------------------------------------------------------------------

    void SceneManager::SetActiveScene (const std::string& scenePath, bool fadeAnimation)
    {
        Scene* scene = m_MainLoop->GetSceneManager ().GetScene (scenePath);

        if (!scene)
        {
            scene = SceneLoader::LoadScene (this, scenePath);
            m_MainLoop->GetSceneManager ().AddScene (scene);
        }

        if (fadeAnimation)
        {
            m_NextScene = scene;
            SceneFadeInOut ();
        }
        else
        {
            SetActiveScene (scene);
        }
    }

    //--------------------------------------------------------------------------------------------------

    Scene* SceneManager::GetActiveScene () { return m_ActiveScene; }

    //--------------------------------------------------------------------------------------------------

    Scene* SceneManager::GetScene (const std::string& path)
    {
        for (std::map<std::string, Scene*>::iterator it = m_Scenes.begin (); it != m_Scenes.end (); ++it)
        {
            if ((*it->second).GetPath () == path)
            {
                return it->second;
            }
        }

        return nullptr;
    }

    //--------------------------------------------------------------------------------------------------

    void SceneManager::ProcessEvent (ALLEGRO_EVENT* event, float deltaTime)
    {
        m_SpeechFrameManager.ProcessEvent (event, deltaTime);
    }

    //--------------------------------------------------------------------------------------------------

    bool SceneManager::Update (float deltaTime)
    {
        if (m_ActiveScene)
        {
            m_ActiveScene->Update (deltaTime);

            if (m_MainLoop->GetStateManager ().GetActiveStateName () != "EDITOR_STATE")
            {
                m_SpeechFrameManager.Update (deltaTime);
            }
        }

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    void SceneManager::Render (float deltaTime)
    {
        if (m_ActiveScene)
        {
            m_ActiveScene->Render (deltaTime);

            m_Camera.UseIdentityTransform ();

            if (m_MainLoop->GetStateManager ().GetActiveStateName () != "EDITOR_STATE")
            {
                m_SpeechFrameManager.Render (deltaTime);
            }
        }

        if (m_Transitioning)
        {
            const Point size = m_MainLoop->GetScreen ()->GetWindowSize ();
            al_draw_filled_rectangle (0, 0, size.Width, size.Height, m_FadeColor);
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

    void SceneManager::AddOnEnterCallback (const std::string& triggerName, std::function<void(float dx, float dy)> func)
    {
        if (m_ActiveScene)
        {
            m_ActiveScene->AddOnEnterCallback (triggerName, func);
        }
    }

    //--------------------------------------------------------------------------------------------------

    void SceneManager::AddOnEnterCallback (const std::string& triggerName, asIScriptFunction* func)
    {
        if (m_ActiveScene)
        {
            m_ActiveScene->AddOnEnterCallback (triggerName, func);
        }
    }

    //--------------------------------------------------------------------------------------------------

    void SceneManager::AddOnLeaveCallback (const std::string& triggerName, std::function<void(float dx, float dy)> func)
    {
        if (m_ActiveScene)
        {
            m_ActiveScene->AddOnLeaveCallback (triggerName, func);
        }
    }

    //--------------------------------------------------------------------------------------------------

    void SceneManager::AddOnLeaveCallback (const std::string& triggerName, asIScriptFunction* func)
    {
        if (m_ActiveScene)
        {
            m_ActiveScene->AddOnLeaveCallback (triggerName, func);
        }
    }

    //--------------------------------------------------------------------------------------------------

    Actor* SceneManager::GetActor (const std::string& name)
    {
        if (m_ActiveScene)
        {
            return m_ActiveScene->GetActor (name);
        }

        return nullptr;
    }

    //--------------------------------------------------------------------------------------------------

    FlagPoint* SceneManager::GetFlagPoint (const std::string& name)
    {
        if (m_ActiveScene)
        {
            return m_ActiveScene->GetFlagPoint (name);
        }

        return nullptr;
    }

    //--------------------------------------------------------------------------------------------------

    Actor* SceneManager::GetCurrentlyProcessedActor ()
    {
        if (m_ActiveScene)
        {
            return m_ActiveScene->GetCurrentlyProcessedActor ();
        }

        return nullptr;
    }

    //--------------------------------------------------------------------------------------------------

    SpeechFrameManager& SceneManager::GetSpeechFrameManager () { return m_SpeechFrameManager; }

    //--------------------------------------------------------------------------------------------------

    void SceneManager::Reset ()
    {
        m_Camera.ClearTransformations ();
        m_FadeColor.a = 0.0f;
        m_Transitioning = false;
    }

    //--------------------------------------------------------------------------------------------------

    void SceneManager::SceneFadeInOut (float fadeInMs, float fadeOutMs, ALLEGRO_COLOR color)
    {
        m_FadeColor = color;
        m_FadeColor.a = 0.0f;
        m_Transitioning = true;

        auto fadeOutFunc = [&](float v) {
            if (m_TweenFade->TweenF.progress () < 1.0f)
            {
                m_FadeColor.a = v;
            }
            else
            {
                m_Transitioning = false;
            }

            return false;
        };

        auto fadeInFunc = [&](float v) {

            if (m_TweenFade->TweenF.progress () < 1.0f)
            {
                m_FadeColor.a = v;
            }

            if (m_TweenFade->TweenF.progress () > 0.5f)
            {
                if (m_NextScene)
                {
                    SetActiveScene (m_NextScene);
                    m_NextScene = nullptr;
                }
            }

            return false;
        };

        tweeny::tween<float> tween = tweeny::from (0.0f)
                                         .to (1.0f)
                                         .during (fadeInMs)
                                         .onStep (fadeInFunc)
                                         .to (0.0f)
                                         .during (fadeOutMs)
                                         .onStep (fadeOutFunc);

        m_TweenFade = &m_MainLoop->GetTweenManager ().AddTween (SCENE_FADE_ID, tween);
    }

    //--------------------------------------------------------------------------------------------------

    bool SceneManager::IsTransitioning () const { return m_Transitioning; }

    //--------------------------------------------------------------------------------------------------

    void SceneManager::SetDrawPhysData (bool enable) { m_DrawPhysData = enable; }

    //--------------------------------------------------------------------------------------------------

    bool SceneManager::IsDrawPhysData () { return m_DrawPhysData; }

    //--------------------------------------------------------------------------------------------------

    void SceneManager::SetDrawBoundingBox (bool enable) { m_DrawBoundingBox = enable; }

    //--------------------------------------------------------------------------------------------------

    bool SceneManager::IsDrawBoundingBox () { return m_DrawBoundingBox; }

    //--------------------------------------------------------------------------------------------------

    void SceneManager::SetDrawActorsNames (bool enable) { m_DrawActorsNames = enable; }

    //--------------------------------------------------------------------------------------------------

    bool SceneManager::IsDrawActorsNames () { return m_DrawActorsNames; }

    //--------------------------------------------------------------------------------------------------
}
