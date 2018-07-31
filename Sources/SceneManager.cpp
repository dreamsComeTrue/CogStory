// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "SceneManager.h"
#include "ActorFactory.h"
#include "Common.h"
#include "MainLoop.h"
#include "Scene.h"
#include "SceneLoader.h"
#include "Screen.h"
#include "states/GamePlayState.h"
#include "states/MainMenuState.h"

#include <algorithm>

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    static int SCENE_TWEEN_ID = 200000;
    static int SCENE_TWEEN_INTRO_ID = 200001;
    static int SCENE_TWEEN_OVERLAY_ID = 200002;

    //--------------------------------------------------------------------------------------------------

    SceneManager::SceneManager (MainLoop* mainLoop)
        : m_MainLoop (mainLoop)
        , m_Player (nullptr)
        , m_Camera (this)
        , m_SpeechFrameManager (this)
        , m_ActiveScene (nullptr)
        , m_Transitioning (true)
        , m_FadeColor (COLOR_BLACK)
        , m_TweenFade (nullptr)
        , m_DrawPhysData (true)
        , m_DrawBoundingBox (true)
        , m_DrawActorsNames (true)
        , m_NextScene (nullptr)
        , m_OverlayText ("")
        , m_OverlayPosition (Center)
        , m_OverlayDuration (2000.f)
        , m_OverlayActive (false)

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
        ActorFactory::RegisterActorComponents ();
        ActorFactory::RegisterAnimations ();

        m_Player = new Player (this);
        m_Player->Initialize ();
        m_Player->SetCheckOverlap (true);

        m_Camera.SetFollowActor (m_Player, {0, 0});

        m_SpeechFrameManager.Initialize ();

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    bool SceneManager::Destroy ()
    {
        SAFE_DELETE (m_Player);

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

            if (!scene->IsSuppressSceneInfo ())
            {
                SceneIntro (2500.0f);
            }
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

            if (m_MainLoop->GetStateManager ().GetActiveStateName () == GAMEPLAY_STATE_NAME)
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

            if (m_MainLoop->GetStateManager ().GetActiveStateName () == GAMEPLAY_STATE_NAME)
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

            if (!m_ActiveScene->IsSuppressSceneInfo ())
            {
                PrintOverlayText (m_ActiveScene->GetName (), TopCenter);
            }

            al_set_blender (blendOp, blendSrc, blendDst);
        }

        if (m_OverlayActive)
        {
            int blendOp, blendSrc, blendDst;
            al_get_blender (&blendOp, &blendSrc, &blendDst);
            al_set_blender (ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA);

            PrintOverlayText (m_OverlayText.substr (0, m_OverlayCharPos), m_OverlayPosition);

            al_set_blender (blendOp, blendSrc, blendDst);

            m_OverlayCharDuration += deltaTime;

            if (m_OverlayCharDuration > m_OverlayCharMaxDuration)
            {
                ++m_OverlayCharPos;
                m_OverlayCharPos = std::min (m_OverlayCharPos, (int)m_OverlayText.length ());

                m_OverlayCharDuration = 0.f;
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    Player* SceneManager::GetPlayer () { return m_Player; }

    //--------------------------------------------------------------------------------------------------

    Camera& SceneManager::GetCamera () { return m_Camera; }

    //--------------------------------------------------------------------------------------------------

    MainLoop* SceneManager::GetMainLoop () { return m_MainLoop; }

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

    void SceneManager::RemoveOnEnterCallback (const std::string& triggerName)
    {
        if (m_ActiveScene)
        {
            m_ActiveScene->RemoveOnEnterCallback (triggerName);
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

    void SceneManager::RemoveOnLeaveCallback (const std::string& triggerName)
    {
        if (m_ActiveScene)
        {
            m_ActiveScene->RemoveOnLeaveCallback (triggerName);
        }
    }

    //--------------------------------------------------------------------------------------------------
    //
    void SceneManager::RegisterChoiceFunction (const std::string& name, asIScriptFunction* func)
    {
        if (m_ActiveScene)
        {
            m_ActiveScene->RegisterChoiceFunction (name, func);
        }
    }

    //--------------------------------------------------------------------------------------------------

    Actor* SceneManager::GetActor (const std::string& name)
    {
        if (name == "PLAYER")
        {
            return m_Player;
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

    AudioSample* SceneManager::SetSceneAudioStream (const std::string& path)
    {
        if (m_ActiveScene)
        {
            return m_ActiveScene->SetSceneAudioStream (path);
        }

        return nullptr;
    }

	//--------------------------------------------------------------------------------------------------

    AudioSample* SceneManager::GetSceneAudioStream ()
    {
        if (m_ActiveScene)
        {
            return m_ActiveScene->GetSceneAudioStream ();
        }

        return nullptr;
    }

    //--------------------------------------------------------------------------------------------------

    void SceneManager::SetSuppressSceneInfo (bool suppress)
    {
        if (m_ActiveScene)
        {
            m_ActiveScene->SetSuppressSceneInfo (suppress);
        }
    }

    //--------------------------------------------------------------------------------------------------

    bool SceneManager::IsSuppressSceneInfo () const
    {
        return m_ActiveScene ? m_ActiveScene->IsSuppressSceneInfo () : false;
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

    void SceneManager::PrintOverlayText (const std::string& text, ScreenRelativePosition screenPos)
    {
        const Point winSize = m_MainLoop->GetScreen ()->GetWindowSize ();
        Font& font = m_MainLoop->GetScreen ()->GetFont ();
        Point textSize = font.GetTextDimensions (FONT_NAME_MENU_TITLE, text);
        float offset = 30.f;

        Point pos;
        int align = 0;

        float textSizeHeight = textSize.Height * (std::count (m_OverlayText.begin (), m_OverlayText.end (), '\n') + 1);

        switch (screenPos)
        {
        case aga::TopLeft:
            pos = {offset, offset};
            align = ALLEGRO_ALIGN_LEFT;
            break;

        case aga::TopCenter:
            pos = {winSize.Width * 0.5f, offset};
            align = ALLEGRO_ALIGN_CENTER;
            break;

        case aga::TopRight:
            pos = {winSize.Width - offset, offset};
            align = ALLEGRO_ALIGN_RIGHT;
            break;

        case aga::BottomLeft:
            pos = {offset, winSize.Height - textSizeHeight - offset};
            align = ALLEGRO_ALIGN_LEFT;
            break;

        case aga::BottomCenter:
            pos = {winSize.Width * 0.5f, winSize.Height - textSizeHeight - offset};
            align = ALLEGRO_ALIGN_CENTER;
            break;

        case aga::BottomRight:
            pos = {winSize.Width - textSizeHeight, winSize.Height - textSizeHeight - offset};
            align = ALLEGRO_ALIGN_RIGHT;
            break;

        case aga::Center:
            pos = {winSize.Width * 0.5f, winSize.Height * 0.5f - textSizeHeight * 0.5f};
            align = ALLEGRO_ALIGN_CENTER;
            break;
        }

        font.DrawMultilineText (FONT_NAME_MENU_TITLE, m_CenterTextColor, pos.X, pos.Y, winSize.Width, textSize.Height,
            align, text.c_str ());
    }

    //--------------------------------------------------------------------------------------------------

    void SceneManager::SetOverlayText (
        const std::string& text, float duration, float charTimeDelay, ScreenRelativePosition pos)
    {
        m_OverlayText = text;
        m_OverlayDuration = duration;
        m_OverlayPosition = pos;
        m_OverlayActive = true;
        m_OverlayCharPos = 0;
        m_OverlayCharDuration = 0.f;
        m_OverlayCharMaxDuration = charTimeDelay / 1000.f;

        m_CenterTextColor = al_map_rgb (160, 160, 160);

        auto introFunc = [&](float v) {
            m_CenterTextColor.a = std::min (1.0f, v);

            return false;
        };

        float halfTime = duration * 0.5f;
        tweeny::tween<float> tween = tweeny::from (0.0f)
                                         .to (1.0f)
                                         .during (halfTime)
                                         .to (1.0f)
                                         .during (duration)
                                         .to (0.0f)
                                         .during (halfTime)
                                         .onStep (introFunc);

        m_MainLoop->GetTweenManager ().RemoveTween (SCENE_TWEEN_OVERLAY_ID);
        m_MainLoop->GetTweenManager ().AddTween (SCENE_TWEEN_OVERLAY_ID, tween, [&](int) {
            m_OverlayActive = false;
            m_OverlayText = "";
            m_OverlayDuration = 0.f;
        });
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
