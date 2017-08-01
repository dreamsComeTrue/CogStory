// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __SCENE_MANAGER_H__
#define __SCENE_MANAGER_H__

#include "Camera.h"
#include "Common.h"
#include "Player.h"

namespace aga
{
    class Scene;
    class MainLoop;

    class SceneManager : public Lifecycle
    {
    public:
        SceneManager (MainLoop* mainLoop);
        virtual ~SceneManager ();
        bool Initialize ();
        bool Destroy ();

        void AddScene (ResourceID id, Scene* scene);
        void SetActiveScene (Scene* scene);
        Scene* GetActiveScene ();

        void ProcessEvent (ALLEGRO_EVENT* event, double deltaTime);
        bool Update (double deltaTime);
        void Render (double deltaTime);

        Player& GetPlayer ();
        Camera& GetCamera ();
        MainLoop* GetMainLoop ();

    private:
        Player m_Player;
        Camera m_Camera;

        MainLoop* m_MainLoop;
        std::map<ResourceID, Scene*> m_Scenes;
        Scene* m_ActiveScene;
    };
}

#endif //   __SCENE_MANAGER_H__
