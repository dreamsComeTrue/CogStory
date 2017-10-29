// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __SCENE_H__
#define __SCENE_H__

#include "Collidable.h"
#include "Common.h"
#include "Entity.h"
#include "QuadTree.h"
#include "Scriptable.h"

namespace aga
{
#define PTM_RATIO 64.0

    class SceneManager;
    class AtlasManager;

    struct Tile : public Entity, public Collidable
    {
        Tile (PhysicsManager* physicsManager)
          : Collidable (physicsManager)
        {
        }

        std::string Tileset;
        std::string Name;
        int ZOrder = 0;
        float Rotation = 0;

        int RenderID = 0;

        bool operator== (const Tile& rhs) const
        {
            return Tileset == rhs.Tileset && Name == rhs.Name && Bounds == rhs.Bounds && Rotation == rhs.Rotation;
        }

        static bool CompareByZOrder (const Tile* a, const Tile* b) { return a->ZOrder < b->ZOrder; }

        void Draw (AtlasManager* atlasManager);
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
        static void SaveScene (Scene* scene, const std::string& filePath);

        virtual void Update (float deltaTime);
        virtual void Render (float deltaTime);

        void AddTile (Tile* tile);
        void RemoveTile (Tile* tile);
        std::vector<Tile*>& GetTiles ();

        Point GetSpawnPoint (const std::string& name);

        void SortTiles ();

        void Reset ();

        std::string GetName ();

        void SetDrawPhysData (bool enable);
        bool IsDrawPhysData ();
        QuadTreeNode& GetQuadTree ();

    private:
        void DrawQuadTree (QuadTreeNode* node);

    private:
        std::string m_Name;
        Point m_Size;
        std::map<std::string, Point> m_SpawnPoints;
        std::vector<Tile*> m_Tiles;
        SceneManager* m_SceneManager;

        bool m_DrawPhysData;
        QuadTreeNode m_QuadTree;
    };
}

#endif //   __SCENE_H__
