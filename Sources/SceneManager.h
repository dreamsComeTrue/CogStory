// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __SCENE_MANAGER_H__
#define __SCENE_MANAGER_H__

#include "Camera.h"
#include "Common.h"
#include "Player.h"

namespace aga
{
    class Scene;
    class Screen;

    class SceneManager : public Lifecycle
    {
    public:
        SceneManager (Screen* screen);
        virtual ~SceneManager ();
        bool Initialize ();
        bool Destroy ();

        void SetActiveScene (Scene* scene);

        void ProcessEvent (ALLEGRO_EVENT* event, double deltaTime);
        bool Update (double deltaTime);
        void Render (double deltaTime);

        Screen* GetScreen ();
        Player& GetPlayer ();

    private:
        void LoadScenes ();

        Player m_Player;
        Camera m_Camera;

        Screen* m_Screen;
        std::map<ResourceID, Scene*> m_Scenes;
        Scene* m_ActiveScene;
    };
}

#endif //   __SCENE_MANAGER_H__
