// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __EDITOR_H__
#define __EDITOR_H__

#include "Common.h"
#include "EditorActorMode.h"
#include "EditorFlagPointMode.h"
#include "EditorPhysMode.h"
#include "EditorSpeechMode.h"
#include "EditorTileMode.h"
#include "EditorTriggerAreaMode.h"
#include "Scene.h"

#include <Gwork/Controls.h>

namespace aga
{
    extern const int TILES_COUNT;

    class EditorOpenSceneWindow;
    class EditorSaveSceneWindow;
    class EditorFlagPointWindow;
    class EditorTriggerAreaWindow;
    class EditorSpeechWindow;
    class EditorActorWindow;
    class EditorInfoWindow;
    class EditorQuestionWindow;

    class MainLoop;
    struct Tile;

    enum CursorMode
    {
        TileSelectMode,
        TileEditMode,
        EditPhysBodyMode,
        EditFlagPointsMode,
        EditTriggerAreaMode,
        SpeechMode,
        ActorMode
    };

    class Editor : public Lifecycle, public Gwk::Event::Handler
    {
        friend class EditorOpenSceneWindow;
        friend class EditorSaveSceneWindow;
        friend class EditorFlagPointWindow;
        friend class EditorTriggerAreaWindow;
        friend class EditorSpeechWindow;
        friend class EditorActorWindow;

        friend class EditorTileMode;
        friend class EditorPhysMode;
        friend class EditorFlagPointMode;
        friend class EditorTriggerAreaMode;
        friend class EditorSpeechMode;
        friend class EditorActorMode;

    public:
        Editor (MainLoop* mainLoop);
        virtual ~Editor ();
        bool Initialize ();
        bool Destroy ();

        bool Update (float deltaTime);
        void ProcessEvent (ALLEGRO_EVENT* event, float deltaTime);
        void Render (float deltaTime);

        void OnResetTranslate ();
        void OnResetScale ();

        void SetDrawUITiles (bool draw);

    private:
        void LoadConfig ();
        void SaveConfig ();

        void DrawGrid ();

        void ChangeGridSize (bool clockwise);

        void HandleCameraMovement (const ALLEGRO_MOUSE_EVENT& event);

        void OnNewScene (Gwk::Controls::Base* control);
        void OnOpenScene (Gwk::Controls::Base* control);
        void OnSaveScene (Gwk::Controls::Base* control);

        void LoadScene (const std::string& filePath);
        void SaveScene (const std::string& filePath);

        void OnPlay ();
        void MenuItemPlay ();

        void OnExit ();

        void OnShowGrid ();
        void OnGridIncrease ();
        void OnGridDecrease ();

        void OnFlagPoint ();
        void OnTriggerArea ();

        void OnTileMode ();
        void OnNewPoly ();

        void OnSpeech ();
        void OnActor ();

        void UpdateScriptsBox ();
        void OnReloadScript ();

        Point CalculateCursorPoint (int mouseX, int mouseY);

        bool IsMouseWithinPointRect (int mouseX, int mouseY, Point point, int outsets);

        void RenderUI ();

        void ResetSettings ();
        void ScreenResize ();

    private:
        MainLoop* m_MainLoop;
        EditorTileMode m_EditorTileMode;
        EditorPhysMode m_EditorPhysMode;
        EditorFlagPointMode m_EditorFlagPointMode;
        EditorTriggerAreaMode m_EditorTriggerAreaMode;
        EditorSpeechMode m_EditorSpeechMode;
        EditorActorMode m_EditorActorMode;

        CursorMode m_CursorMode;

        bool m_IsSnapToGrid;
        int m_BaseGridSize;
        int m_GridSize;

        bool m_IsMousePan;
        bool m_IsMouseWheel;

        EditorOpenSceneWindow* m_OpenSceneWindow;
        EditorSaveSceneWindow* m_SaveSceneWindow;
        EditorFlagPointWindow* m_FlagPointWindow;
        EditorTriggerAreaWindow* m_TriggerAreaWindow;
        EditorSpeechWindow* m_SpeechWindow;
        EditorActorWindow* m_ActorWindow;
        EditorInfoWindow* m_InfoWindow;
        EditorQuestionWindow* m_QuestionWindow;
    };
}

#endif //   __EDITOR_H__
