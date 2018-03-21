// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __EDITOR_H__
#define __EDITOR_H__

#include "Common.h"
#include "EditorActorMode.h"
#include "EditorFlagPointMode.h"
#include "EditorPhysMode.h"
#include "EditorSpeechMode.h"
#include "EditorTriggerAreaMode.h"
#include "Scene.h"

#include <Gwork/Controls.h>
#include <Gwork/Input/Allegro5.h>
#include <Gwork/Platform.h>
#include <Gwork/Renderers/Allegro5.h>
#include <Gwork/Skins/TexturedBase.h>

namespace aga
{
    extern const int TILES_COUNT;

    class EditorOpenSceneWindow;
    class EditorSaveSceneWindow;
    class EditorFlagPointWindow;
    class EditorTriggerAreaWindow;
    class EditorSpeechWindow;
    class EditorActorWindow;
    class EditorActorScriptWindow;
    class EditorInfoWindow;
    class EditorInputWindow;
    class EditorSceneWindow;
    class EditorScriptWindow;

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
    public:
        Editor (MainLoop* mainLoop);
        virtual ~Editor ();
        bool Initialize ();
        bool Destroy ();

        bool Update (float deltaTime);
        void ProcessEvent (ALLEGRO_EVENT* event, float deltaTime);
        void Render (float deltaTime);

        void LoadScene (const std::string& filePath);
        void SaveScene (const std::string& filePath);

        void OnResetTranslate ();
        void OnResetScale ();

        void SetDrawUITiles (bool draw);

        CursorMode GetCursorMode () const { return m_CursorMode; }
        void SetCursorMode (CursorMode mode) { m_CursorMode = mode; }
        void SwitchCursorMode ();

        Point CalculateCursorPoint (int mouseX, int mouseY);
        bool IsMouseWithinPointRect (int mouseX, int mouseY, Point point, int outsets);

        MainLoop* GetMainLoop () { return m_MainLoop; }

        EditorPhysMode& GetEditorPhysMode () { return m_EditorPhysMode; }
        EditorActorMode& GetEditorActorMode () { return m_EditorActorMode; }
        EditorFlagPointMode& GetEditorFlagPointMode () { return m_EditorFlagPointMode; }
        EditorSpeechMode& GetEditorSpeechMode () { return m_EditorSpeechMode; }
        EditorTriggerAreaMode& GetEditorTriggerAreaMode () { return m_EditorTriggerAreaMode; }

        EditorScriptWindow* GetScriptWindow () { return m_ScriptWindow; }
        EditorInfoWindow* GetEditorInfoWindow () { return m_InfoWindow; }

        void UpdateSceneNameLabel (const std::string& name);

    private:
        void LoadConfig ();
        void SaveConfig ();

        void DrawGrid ();

        void ChangeGridSize (bool clockwise);
        bool IsEditorCanvasNotCovered ();

        void HandleCameraMovement (const ALLEGRO_MOUSE_EVENT& event);
        void HandleCameraPan (float deltaTime);

        void OnNewScene (Gwk::Controls::Base* control);
        void OnOpenScene (Gwk::Controls::Base* control);
        void OnSaveScene (Gwk::Controls::Base* control);

        void OnPlay ();
        void OnExit ();

        void OnShowGrid ();
        void OnGridIncrease ();
        void OnGridDecrease ();

        void OnSceneEdit ();
        void OnFlagPoint ();
        void OnTriggerArea ();

        void OnRemoveBody ();
        void OnNewPoly ();

        void OnSpeech ();
        void OnActor ();

        void MarkPlayerPosition ();

        void OnTilesetSelected (Gwk::Controls::Base* control);

        void RenderUI ();
        void RenderPhysBodyMode (float deltaTime);

        void ResetSettings ();
        void ScreenResize ();

        void ProcessMouseButtonDoubleClick (ALLEGRO_MOUSE_EVENT& event);
        void ProcessMouseButtonDown (ALLEGRO_MOUSE_EVENT& event);
        void ProcessMouseButtonUp (ALLEGRO_MOUSE_EVENT& event);
        void ProcessMouseAxes (ALLEGRO_MOUSE_EVENT& event);

        void TryToCloseWindows ();

    private:
        MainLoop* m_MainLoop;
        EditorPhysMode m_EditorPhysMode;
        EditorFlagPointMode m_EditorFlagPointMode;
        EditorTriggerAreaMode m_EditorTriggerAreaMode;
        EditorSpeechMode m_EditorSpeechMode;
        EditorActorMode m_EditorActorMode;

        CursorMode m_CursorMode;

        bool m_IsSnapToGrid;
        float m_BaseGridSize;
        float m_GridSize;

        bool m_IsMousePan;

        long m_LastTimeClicked;

        EditorSceneWindow* m_EditorSceneWindow;
        EditorOpenSceneWindow* m_OpenSceneWindow;
        EditorSaveSceneWindow* m_SaveSceneWindow;
        EditorFlagPointWindow* m_FlagPointWindow;
        EditorTriggerAreaWindow* m_TriggerAreaWindow;
        EditorSpeechWindow* m_SpeechWindow;
        EditorActorWindow* m_ActorWindow;
        EditorInfoWindow* m_InfoWindow;
        EditorInputWindow* m_InputWindow;
        EditorScriptWindow* m_ScriptWindow;

        Gwk::Renderer::AllegroResourceLoader* m_ResourceLoader;
        Gwk::Renderer::Allegro* m_GUIRenderer;
        Gwk::Controls::Canvas* m_MainCanvas;
        Gwk::Skin::TexturedBase* m_GuiSkin;

        Gwk::Input::Allegro m_GUIInput;

        std::string m_LastScenePath;
    };
}

#endif //   __EDITOR_H__
