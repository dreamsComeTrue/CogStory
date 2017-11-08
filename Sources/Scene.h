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
    class SceneManager;
    class AtlasManager;
    class Triangulator;

    struct TriggerArea
    {
        std::string Name = "";
        std::vector<Point> Points = {};
        std::vector<Polygon> Polygons = {};
        std::function<void(float dx, float dy)> OnEnterCallback = nullptr;
        std::function<void(float dx, float dy)> OnLeaveCallback = nullptr;
        asIScriptFunction* ScriptOnEnterCallback = nullptr;
        asIScriptFunction* ScriptOnLeaveCallback = nullptr;

        void UpdatePolygons (Triangulator* triangulator);

        bool WasEntered = false;
    };

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

        void BeforeEnter ();
        void AfterLeave ();

        static Scene* LoadScene (SceneManager* sceneManager, const std::string& filePath);
        static void SaveScene (Scene* scene, const std::string& filePath);

        void Update (float deltaTime);
        void Render (float deltaTime);

        void AddTile (Tile* tile);
        void RemoveTile (Tile* tile);
        std::vector<Tile*>& GetTiles ();

        void AddFlagPoint (const std::string& name, Point point);
        Point GetFlagPoint (const std::string& name);
        std::map<std::string, Point>& GetFlagPoints ();

        void AddTriggerArea (const std::string& name, std::vector<Point> points);
        std::map<std::string, TriggerArea>& GetTriggerAreas ();
        TriggerArea& GetTriggerArea (const std::string& name);
        void RemoveTriggerArea (const std::string& name);

        void SortTiles ();

        void Reset ();

        std::string GetName ();

        void SetDrawPhysData (bool enable);
        bool IsDrawPhysData ();
        QuadTreeNode& GetQuadTree ();

        void AddOnEnterCallback (const std::string& triggerName, std::function<void(float dx, float dy)> func);
        void AddOnEnterCallback (const std::string& triggerName, asIScriptFunction* func);

        void AddOnLeaveCallback (const std::string& triggerName, std::function<void(float dx, float dy)> func);
        void AddOnLeaveCallback (const std::string& triggerName, asIScriptFunction* func);

    private:
        void DrawQuadTree (QuadTreeNode* node);

    private:
        std::string m_Name;
        Point m_Size;
        std::map<std::string, Point> m_FlagPoints;
        std::map<std::string, TriggerArea> m_TriggerAreas;
        std::vector<Tile*> m_Tiles;
        SceneManager* m_SceneManager;

        bool m_DrawPhysData;
        QuadTreeNode m_QuadTree;
    };
}

#endif //   __SCENE_H__
