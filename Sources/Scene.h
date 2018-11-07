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

    extern const float SCENE_INFINITE_BOUND_SIZE;

    class Actor;
    class SceneManager;
    class AtlasManager;
    class Triangulator;
    class SceneLoader;
    class TileActor;
    class AudioStream;

    struct SpeechOutcome
    {
        std::string Name = "";
        std::string Text = "";

        std::string Action = "";
    };

    struct SpeechData
    {
        long ID = -1;
        std::string Name = "";
        std::string Group = "";
        std::map<int, std::string> Text; //  LangID, data
        std::map<int, std::vector<SpeechOutcome>> Outcomes; //  LangID, outcomes

        std::string Action = ""; //  If no outcomes, action to fire

        std::string ActorRegionName;
        int MaxCharsInLine = 0;
        int MaxLines = 0;
        Point AbsoluteFramePosition = {0, 0};
        ScreenRelativePosition RelativeFramePosition = ScreenRelativePosition::Center;
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
        std::string Data = "";
        std::vector<Point> Points = {};
        std::vector<Polygon> Polygons = {};
        std::function<void(std::string areaName, float dx, float dy)> OnEnterCallback = nullptr;
        std::function<void(std::string areaName, float dx, float dy)> OnLeaveCallback = nullptr;
        asIScriptFunction* ScriptOnEnterCallback = nullptr;
        asIScriptFunction* ScriptOnLeaveCallback = nullptr;

        void UpdatePolygons (Triangulator* triangulator);

        bool WasEntered = false;
        bool Collidable = false;

        Rect GetBounds ()
        {
            Rect out = {Point::MAX_POINT, Point::MIN_POINT};

            for (Point& point : Points)
            {
                if (point.X < out.GetTopLeft ().X)
                {
                    out.SetTopLeft ({point.X, out.GetTopLeft ().Y});
                }
                if (point.Y < out.GetTopLeft ().Y)
                {
                    out.SetTopLeft ({out.GetTopLeft ().X, point.Y});
                }
                if (point.X > out.GetBottomRight ().X)
                {
                    out.SetBottomRight ({point.X, out.GetBottomRight ().Y});
                }
                if (point.Y > out.GetBottomRight ().Y)
                {
                    out.SetBottomRight ({out.GetBottomRight ().X, point.Y});
                }
            }

            return out;
        }
    };

    class Scene : public Lifecycle, public Scriptable
    {
        friend class SceneLoader;

    public:
        Scene (SceneManager* sceneManager, Rect size);

        virtual ~Scene ();
        bool Initialize ();
        bool Destroy ();

        void BeforeEnter ();
        void AfterLeave ();

        void Update (float deltaTime);
        void Render (float deltaTime);

        void AddActor (Actor* actor);
        void RemoveActor (Actor* actor);
        void RemoveActor (int id);
        void RemoveActor (const std::string& name);
        Actor* GetActor (const std::string& name);
        Actor* GetActor (int id);
        std::vector<Actor*>& GetActors ();

        std::vector<TileActor*> GetTiles ();
        std::vector<Entity*> RecomputeVisibleEntities (bool force);

        FlagPoint* AddFlagPoint (const std::string& name, Point point);
        FlagPoint* GetFlagPoint (const std::string& name);
        std::map<std::string, FlagPoint>& GetFlagPoints ();
        void RemoveFlagPoint (const std::string& name);

        TriggerArea* AddTriggerArea (
            const std::string& name, const std::string& data, std::vector<Point> points, bool collidable);
        std::map<std::string, TriggerArea>& GetTriggerAreas ();
        TriggerArea* GetTriggerArea (const std::string& name);
        void RemoveTriggerArea (const std::string& name);

        bool AddSpeech (SpeechData data);
        std::map<int, SpeechData>& GetSpeeches ();
        SpeechData* GetSpeech (int id);
        SpeechData* GetSpeech (const std::string& name);
        void RemoveSpeech (int id);
        void RemoveSpeech (const std::string& name);

        void RegisterChoiceFunction (const std::string& name, asIScriptFunction* func);
        std::map<std::string, asIScriptFunction*>& GetChoiceFunctions () { return m_ChoiceFunctions; }

        void SortActors ();

        void Reset ();

        void SetName (const std::string& name);
        std::string GetName () const;

        std::string GetPath () const { return m_Path; }

        void SetBackgroundColor (ALLEGRO_COLOR color);
        void SetBackgroundColor (float r, float g, float b, float a);
        ALLEGRO_COLOR GetBackgroundColor () const;

        QuadTreeNode& GetQuadTree ();

        Rect GetRenderBounds (Entity* entity, bool drawOOBBox = false);
        Rect GetRenderBounds (Rect b);

        void AddOnEnterCallback (
            const std::string& triggerName, std::function<void(std::string areaName, float dx, float dy)> func);
        void AddOnEnterCallback (const std::string& triggerName, asIScriptFunction* func);

        void RemoveOnEnterCallback (const std::string& triggerName);

        void AddOnLeaveCallback (
            const std::string& triggerName, std::function<void(std::string areaName, float dx, float dy)> func);
        void AddOnLeaveCallback (const std::string& triggerName, asIScriptFunction* func);

        void RemoveOnLeaveCallback (const std::string& triggerName);

        Actor* GetCurrentlyProcessedActor ();

        void SetPlayerStartLocation (Point location) { m_PlayerStartLocation = location; }
        Point GetPlayerStartLocation () { return m_PlayerStartLocation; }

        AudioStream* SetSceneAudioStream (const std::string& path);
        AudioStream* GetSceneAudioStream () { return m_SceneAudioStream; }

        void SetSuppressSceneInfo (bool suppress) { m_SuppressSceneInfo = suppress; }
        bool IsSuppressSceneInfo () const { return m_SuppressSceneInfo; }

        void ResetAllActorsPositions ();

        void DrawTriggerAreas ();

        void AddSceneTransition (const std::string& triggerAreaName, const std::string& newSceneName);
        std::string GetSceneTransition (const std::string& triggerAreaName);

    private:
        static void UpdateMaxTileID (Scene* scene);
        void DrawQuadTree (QuadTreeNode* node);
        void CleanUpTriggerAreas ();
        void CleanUpSceneAudio ();

        void UpdateCameraBounds ();

    private:
        std::string m_Name;
        std::string m_Path;
        Rect m_Size;
        ALLEGRO_COLOR m_BackgroundColor;

        std::map<std::string, FlagPoint> m_FlagPoints;
        std::map<std::string, TriggerArea> m_TriggerAreas;
        std::map<int, SpeechData> m_Speeches;
        std::vector<Actor*> m_Actors;
        SceneManager* m_SceneManager;

        std::map<std::string, std::string> m_ScenesTransitions;

        std::map<std::string, asIScriptFunction*> m_ChoiceFunctions;

        Actor* m_CurrentActor;

        QuadTreeNode m_QuadTree;
        Point m_VisibleLastCameraPos;
        std::vector<Entity*> m_VisibleEntities;
        bool m_ActorsTreeChanged;

        Point m_PlayerStartLocation;
        AudioStream* m_SceneAudioStream;

        bool m_SuppressSceneInfo;
    };
}

#endif //   __SCENE_H__
