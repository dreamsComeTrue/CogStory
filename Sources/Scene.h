// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __SCENE_H__
#define __SCENE_H__

#include "Common.h"
#include "Scriptable.h"

namespace aga
{
    class SceneManager;

    struct Tile
    {
        std::string FileName;
        ALLEGRO_BITMAP* Image;
        Point Pos;
    };

    class Scene : public Lifecycle, public Scriptable
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

        Point GetSpawnPoint (const std::string& name);

    private:
        std::string m_Name;
        Point m_Size;
        std::map<std::string, Point> m_SpawnPoints;
        std::vector<Tile> m_Tiles;
        SceneManager* m_SceneManager;
    };
}

#endif //   __SCENE_H__
