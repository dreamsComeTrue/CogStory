// Copyright 2017-2019 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __SCENE_LOADER_H__
#define __SCENE_LOADER_H__

#include "Common.h"

namespace aga
{
#define LANG_EN 0
#define LANG_PL 1

    class SceneManager;
    class Scene;

    class SceneLoader
    {
    public:
        static Scene* LoadScene (SceneManager* sceneManager, const std::string& filePath, bool loadBounds = true);
        static void SaveScene (Scene* scene, const std::string& filePath);

        static void UpdateMaxEntityID (Scene* scene);
    };
}

#endif //   __SCENE_LOADER_H__
