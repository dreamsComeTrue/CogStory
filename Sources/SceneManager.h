// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __SCENE_MANAGER_H__
#define __SCENE_MANAGER_H__

#include "Camera.h"
#include "Common.h"
#include "Player.h"
#include "SpeechFrameManager.h"

namespace aga
{
    class Scene;
    class MainLoop;
    class AtlasManager;
    struct TweenData;
    struct FlagPoint;

    class SceneManager : public Lifecycle
    {
    public:
        SceneManager (MainLoop* mainLoop);
        virtual ~SceneManager ();
        bool Initialize ();
        bool Destroy ();

        void AddScene (Scene* scene);
        void RemoveScene (Scene* scene);
        void SetActiveScene (Scene* scene);
        Scene* GetActiveScene ();
        Scene* GetScene (const std::string& path);

        void SetActiveScene (const std::string& scenePath, bool fadeAnimation = true);

        void ProcessEvent (ALLEGRO_EVENT* event, float deltaTime);
        bool Update (float deltaTime);
        void Render (float deltaTime);

        Player* GetPlayer ();
        Camera& GetCamera ();
        MainLoop* GetMainLoop ();

        void AddOnEnterCallback (const std::string& triggerName, std::function<void(float dx, float dy)> func);
        void AddOnEnterCallback (const std::string& triggerName, asIScriptFunction* func);

        void RemoveOnEnterCallback (const std::string& triggerName);

        void AddOnLeaveCallback (const std::string& triggerName, std::function<void(float dx, float dy)> func);
        void AddOnLeaveCallback (const std::string& triggerName, asIScriptFunction* func);

        void RemoveOnLeaveCallback (const std::string& triggerName);

        void RegisterChoiceFunction (const std::string& name, asIScriptFunction* func);

        Actor* GetActor (const std::string& name);
        FlagPoint* GetFlagPoint (const std::string& name);

        Actor* GetCurrentlyProcessedActor ();

        SpeechFrameManager& GetSpeechFrameManager ();

        void Reset ();
        void SceneFadeInOut (float fadeInMs = 500, float fadeOutMs = 500, ALLEGRO_COLOR color = COLOR_BLACK);
        bool IsTransitioning () const;

        void SetDrawPhysData (bool enable);
        bool IsDrawPhysData ();

        void SetDrawBoundingBox (bool enable);
        bool IsDrawBoundingBox ();

        void SetDrawActorsNames (bool enable);
        bool IsDrawActorsNames ();

        AudioSample* SetSceneAudioStream (const std::string& path);
        AudioSample* GetSceneAudioStream ();

    private:
        void SceneIntro (float duration = 1000.f);
        void PrintCenterText (const std::string& text);

    private:
        SpeechFrameManager m_SpeechFrameManager;
        Player* m_Player;
        Camera m_Camera;

        MainLoop* m_MainLoop;
        std::map<std::string, Scene*> m_Scenes;
        Scene* m_ActiveScene;
        Scene* m_NextScene;

        TweenData* m_TweenFade;
        bool m_Transitioning;
        ALLEGRO_COLOR m_FadeColor;

        bool m_SceneIntro;
        ALLEGRO_COLOR m_CenterTextColor;

        bool m_DrawPhysData;
        bool m_DrawBoundingBox;
        bool m_DrawActorsNames;
    };
}

#endif //   __SCENE_MANAGER_H__
