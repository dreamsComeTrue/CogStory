// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __SCENE_H__
#define __SCENE_H__

#include "Collidable.h"
#include "Common.h"
#include "Entity.h"
#include "QuadTree.h"
#include "Scriptable.h"
#include "SpeechFrameManager.h"

namespace aga
{
#define LANG_EN 0
#define LANG_PL 1
#define CURRENT_LANG LANG_EN

    class Actor;
    class SceneManager;
    class AtlasManager;
    class Triangulator;
    class SceneLoader;
    class TileActor;

    struct SpeechOutcome
    {
        std::string Name = "";
        std::string Text = "";

        std::string Action = "";
    };

    struct SpeechData
    {
        std::string Name = "";
        std::map<int, std::string> Text; //  LangID, data
        std::map<int, std::vector<SpeechOutcome>> Outcomes; //  LangID, outcomes

        std::string ActorRegionName;
        int MaxCharsInLine = 0;
        int MaxLines = 0;
        Point AbsoluteFramePosition = { 0, 0 };
        SpeechFramePosition RelativeFramePosition = SpeechFramePosition::Center;
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

    class Scene : public Lifecycle, public Scriptable
    {
        friend class SceneLoader;

    public:
        Scene (SceneManager* sceneManager);

        virtual ~Scene ();
        bool Initialize ();
        bool Destroy ();

        void BeforeEnter ();
        void AfterLeave ();

        void Update (float deltaTime);
        void Render (float deltaTime);

        void AddActor (const std::string& name, Actor* actor);
        void RemoveActor (const std::string& name);
        Actor* GetActor (const std::string& name);
        std::vector<Actor*>& GetActors ();

        void AddTile (TileActor* tile);
        void RemoveTile (TileActor* tile);
        std::vector<TileActor*> GetTiles ();
        std::vector<Entity*> GetVisibleEntities ();

        void AddFlagPoint (const std::string& name, Point point);
        FlagPoint* GetFlagPoint (const std::string& name);
        std::map<std::string, FlagPoint>& GetFlagPoints ();

        void AddTriggerArea (const std::string& name, std::vector<Point> points);
        std::map<std::string, TriggerArea>& GetTriggerAreas ();
        TriggerArea& GetTriggerArea (const std::string& name);
        void RemoveTriggerArea (const std::string& name);

        bool AddSpeech (SpeechData data);
        std::map<std::string, SpeechData>& GetSpeeches ();
        SpeechData* GetSpeech (const std::string& name);
        void RemoveSpeech (const std::string& name);

        void SortActors ();
        void UpdateRenderIDs ();

        void Reset ();

        void SetName (const std::string& name);
        std::string GetName () const;

        QuadTreeNode& GetQuadTree ();

        Rect GetRenderBounds (Entity* entity);

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
        std::vector<Actor*> m_Actors;
        SceneManager* m_SceneManager;

        QuadTreeNode m_QuadTree;
    };
}

#endif //   __SCENE_H__
