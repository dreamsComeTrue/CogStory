// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __SCENE_MANAGER_H__
#define __SCENE_MANAGER_H__

#include "Camera.h"
#include "Common.h"
#include "Player.h"
#include "SpeechFrameManager.h"
#include <stack>

namespace aga
{
    class Scene;
    class MainLoop;
    class AtlasManager;
    class AudioStream;
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

        bool ProcessEvent (ALLEGRO_EVENT* event, float deltaTime);
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
        bool IsTransitioning () const { return m_Transitioning; }

        void SetDrawPhysData (bool enable) { m_DrawPhysData = enable; }
        bool IsDrawPhysData () { return m_DrawPhysData; }

        void SetDrawBoundingBox (bool enable) { m_DrawBoundingBox = enable; }
        bool IsDrawBoundingBox () { return m_DrawBoundingBox; }

        void SetDrawActorsNames (bool enable) { m_DrawActorsNames = enable; }
        bool IsDrawActorsNames () { return m_DrawActorsNames; }

        void SetSuppressSceneInfo (bool suppress);
        bool IsSuppressSceneInfo () const;

        AudioStream* SetSceneAudioStream (const std::string& path);
        AudioStream* GetSceneAudioStream ();

        void SetOverlayText (const std::string& text, float duration = 2000.f, float charTimeDelay = 5.f,
            ScreenRelativePosition pos = BottomRight);
        void SetOverlayActive (bool active) { m_OverlayActive = active; }

        void PushPoint (Point p) { m_SavedPoints.push (p); }
        Point PopPoint ();

    private:
        void SceneIntro (float duration = 1000.f);
        void PrintOverlayText (const std::string& text, ScreenRelativePosition pos = BottomRight);

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

        std::string m_OverlayText;
        int m_OverlayCharPos;
        float m_OverlayCharDuration;
        ScreenRelativePosition m_OverlayPosition;
        float m_OverlayDuration;
        float m_OverlayCharMaxDuration;
        bool m_OverlayActive;

        std::stack<Point> m_SavedPoints;
    };
}

#endif //   __SCENE_MANAGER_H__
