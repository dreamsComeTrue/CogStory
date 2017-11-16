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

        void AddScene (ResourceID id, Scene* scene);
        void RemoveScene (Scene* scene);
        void SetActiveScene (Scene* scene);
        Scene* GetActiveScene ();

        void ProcessEvent (ALLEGRO_EVENT* event, float deltaTime);
        bool Update (float deltaTime);
        void Render (float deltaTime);

        Player& GetPlayer ();
        Camera& GetCamera ();
        MainLoop* GetMainLoop ();
        AtlasManager* GetAtlasManager ();

        void AddOnEnterCallback (const std::string& triggerName, std::function<void(float dx, float dy)> func);
        void AddOnEnterCallback (const std::string& triggerName, asIScriptFunction* func);

        void AddOnLeaveCallback (const std::string& triggerName, std::function<void(float dx, float dy)> func);
        void AddOnLeaveCallback (const std::string& triggerName, asIScriptFunction* func);

        FlagPoint* GetFlagPoint (const std::string& name);

        SpeechFrameManager& GetSpeechFrameManager ();

        void Reset ();
        void SceneFadeInOut ();

    private:
        SpeechFrameManager m_SpeechFrameManager;
        Player m_Player;
        Camera m_Camera;

        AtlasManager* m_AtlasManager;
        MainLoop* m_MainLoop;
        std::map<ResourceID, Scene*> m_Scenes;
        Scene* m_ActiveScene;

        TweenData* m_TweenFade;
        bool m_Transitioning;
        ALLEGRO_COLOR m_FadeColor;
    };
}

#endif //   __SCENE_MANAGER_H__
