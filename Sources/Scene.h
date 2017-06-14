// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __SCENE_H__
#define __SCENE_H__

#include "Common.h"

namespace aga
{
    class SceneManager;

    class Scene : public Lifecycle
    {
    public:
        Scene (SceneManager* sceneManager);

        virtual ~Scene ();
        bool Initialize ();
        bool Destroy ();

        virtual void BeforeEnter ();
        virtual void AfterLeave ();

        static Scene* LoadScene (SceneManager* sceneManager, const std::string& filePath);

        virtual void Update (double deltaTime);
        virtual void Render (double deltaTime);

    private:
        std::string m_Name;
        SceneManager* m_SceneManager;
    };
}

#endif //   __SCENE_H__
