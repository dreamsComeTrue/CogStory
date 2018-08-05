// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __SCENE_LOADER_H__
#define __SCENE_LOADER_H__

#include "Scene.h"

namespace aga
{
#define LANG_EN 0
#define LANG_PL 1

    class SceneLoader
    {
    public:
        static Scene* LoadScene (SceneManager* sceneManager, const std::string& filePath, bool loadBounds = true);
        static void SaveScene (Scene* scene, const std::string& filePath);

        static void UpdateMaxEntityID (Scene* scene);
    };
}

#endif //   __SCENE_LOADER_H__
