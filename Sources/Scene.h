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
#define LANG_EN 0
#define LANG_PL 1

    class SceneManager;
    class AtlasManager;
    class Triangulator;

    struct SpeechOutcome
    {
        std::string Name = "";
        std::string Text = "";

        std::string Action = "";
    };

    struct SpeechData
    {
        std::string Name = "";
        std::map<int, std::string> Text;                    //  LangID, data
        std::map<int, std::vector<SpeechOutcome>> Outcomes; //  LangID, outcomes
    };

    struct FlagPoint
    {
        std::string Name = "";
        Point Pos = {};
        std::vector<FlagPoint*> Connections;
    };

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

        bool operator== (const Tile& rhs) const
        {
            return Tileset == rhs.Tileset && Name == rhs.Name && Bounds == rhs.Bounds && Rotation == rhs.Rotation;
        }

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
        std::vector<Entity*> GetVisibleEntities ();

        void AddFlagPoint (const std::string& name, Point point);
        FlagPoint* GetFlagPoint (const std::string& name);
        std::map<std::string, FlagPoint>& GetFlagPoints ();

        void AddTriggerArea (const std::string& name, std::vector<Point> points);
        std::map<std::string, TriggerArea>& GetTriggerAreas ();
        TriggerArea& GetTriggerArea (const std::string& name);
        void RemoveTriggerArea (const std::string& name);

        bool AddSpeech (const std::string& name, SpeechData data);
        std::map<std::string, SpeechData>& GetSpeeches ();
        SpeechData* GetSpeech (const std::string& name);
        void RemoveSpeech (const std::string& name);

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
        static void UpdateMaxTileID (Scene* scene);
        void DrawQuadTree (QuadTreeNode* node);

    private:
        std::string m_Name;
        Rect m_Size;
        std::map<std::string, FlagPoint> m_FlagPoints;
        std::map<std::string, TriggerArea> m_TriggerAreas;
        std::map<std::string, SpeechData> m_Speeches;
        std::vector<Tile*> m_Tiles;
        SceneManager* m_SceneManager;

        bool m_DrawPhysData;
        QuadTreeNode m_QuadTree;
    };
}

#endif //   __SCENE_H__
