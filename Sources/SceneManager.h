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
    class AtlasManager;

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

        Point GetFlagPoint (const std::string& name);

    private:
        Player m_Player;
        Camera m_Camera;

        AtlasManager* m_AtlasManager;
        MainLoop* m_MainLoop;
        std::map<ResourceID, Scene*> m_Scenes;
        Scene* m_ActiveScene;
    };
}

#endif //   __SCENE_MANAGER_H__
