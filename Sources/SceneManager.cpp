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

    static int SCENE_TWEEN_ID = 200000;
    static int SCENE_TWEEN_INTRO_ID = 200001;

    //--------------------------------------------------------------------------------------------------

    SceneManager::SceneManager (MainLoop* mainLoop)
        : m_MainLoop (mainLoop)
        , m_Camera (this)
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
        Scene* foundScene = GetScene (scene->GetPath ());

        if (!foundScene)
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
                it->second->AfterLeave ();

                if (it->second == m_ActiveScene)
                {
                    m_ActiveScene = nullptr;
                }

                SAFE_DELETE (it->second);
                m_Scenes.erase (it);

                return;
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

            SceneIntro (2500.0f);
        }
    }

    //--------------------------------------------------------------------------------------------------

    void SceneManager::SetActiveScene (const std::string& scenePath, bool fadeAnimation)
    {
        Scene* scene = GetScene (scenePath);

        if (!scene)
        {
            scene = SceneLoader::LoadScene (this, scenePath);
            AddScene (scene);
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

        if (m_ActiveScene != nullptr && m_SceneIntro)
        {
            int blendOp, blendSrc, blendDst;
            al_get_blender (&blendOp, &blendSrc, &blendDst);
            al_set_blender (ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA);

            PrintCenterText (m_ActiveScene->GetName ());

            al_set_blender (blendOp, blendSrc, blendDst);
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
        if (name == "PLAYER")
        {
            return &m_Player;
        }

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

        m_CenterTextColor.a = 0.0f;
        m_SceneIntro = false;
    }

    //--------------------------------------------------------------------------------------------------

    void SceneManager::SceneFadeInOut (float fadeInMs, float fadeOutMs, ALLEGRO_COLOR color)
    {
        m_FadeColor = color;
        m_FadeColor.a = 0.0f;
        m_Transitioning = true;

        auto fadeFunc = [&](float v) {
            m_FadeColor.a = v;

            if (m_TweenFade->TweenF.progress () > 0.5f)
            {
                if (m_NextScene && m_NextScene != m_ActiveScene)
                {
                    SetActiveScene (m_NextScene);
                    m_NextScene = nullptr;
                }
            }

            return false;
        };

        tweeny::tween<float> tween
            = tweeny::from (0.0f).to (1.0f).during (fadeInMs).onStep (fadeFunc).to (0.0f).during (fadeOutMs).onStep (
                fadeFunc);

        m_MainLoop->GetTweenManager ().RemoveTween (SCENE_TWEEN_ID);
        m_TweenFade
            = &m_MainLoop->GetTweenManager ().AddTween (SCENE_TWEEN_ID, tween, [&](int) { m_Transitioning = false; });
    }

    //--------------------------------------------------------------------------------------------------

    void SceneManager::SceneIntro (float duration)
    {
        m_CenterTextColor = al_map_rgb (160, 160, 160);
        m_CenterTextColor.a = 0.0f;
        m_SceneIntro = true;

        auto introFunc = [&](float v) {
            m_CenterTextColor.a = std::min (1.0f, v);

            return false;
        };

        float fadeShowTime = duration * 0.5f;
        tweeny::tween<float> tween = tweeny::from (0.0f)
                                         .to (1.0f)
                                         .during (fadeShowTime)
                                         .to (1.0f)
                                         .during (duration)
                                         .to (0.0f)
                                         .during (fadeShowTime)
                                         .onStep (introFunc);

        m_MainLoop->GetTweenManager ().RemoveTween (SCENE_TWEEN_INTRO_ID);
        m_MainLoop->GetTweenManager ().AddTween (SCENE_TWEEN_INTRO_ID, tween, [&](int) { m_SceneIntro = false; });
    }

    //--------------------------------------------------------------------------------------------------

    void SceneManager::PrintCenterText (const std::string& text)
    {
        const Point winSize = m_MainLoop->GetScreen ()->GetWindowSize ();
        Font& font = m_MainLoop->GetScreen ()->GetFont ();

        font.DrawText (FONT_NAME_MENU_TITLE, m_CenterTextColor, winSize.Width * 0.5f, 50, text, ALLEGRO_ALIGN_CENTER);
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
