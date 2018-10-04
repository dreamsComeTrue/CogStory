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

#include "Gwork/Controls.h"
#include "Gwork/Input/Allegro5.h"
#include "Gwork/Platform.h"
#include "Gwork/Renderers/Allegro5.h"
#include "Gwork/Skins/TexturedBase.h"

//--------------------------------------------------------------------------------------------------

namespace ImGui
{
    bool Combo (const char* label, int* currIndex, std::vector<std::string>& values);
}

//--------------------------------------------------------------------------------------------------

namespace aga
{
    extern const int TILES_COUNT;

    class EditorSpeechWindow;
    class EditorActorWindow;
    class EditorActorScriptWindow;
    class EditorQuestionWindow;
    class EditorSceneWindow;
    class EditorScriptWindow;
    class EditorComponentWindow;

    class MainLoop;
    struct Tile;

    enum CursorMode
    {
        ActorSelectMode,
        EditPhysBodyMode,
        EditFlagPointsMode,
        EditTriggerAreaMode,
        EditSpriteSheetMode
    };

    class Editor : public Lifecycle, public Gwk::Event::Handler
    {
    public:
        Editor (MainLoop* mainLoop);
        virtual ~Editor ();
        bool Initialize ();
        bool Destroy ();

        void BeforeEnter ();

        bool Update (float deltaTime);
        void ProcessEvent (ALLEGRO_EVENT* event, float deltaTime);
        void Render (float deltaTime);

        void LoadScene (const std::string& filePath);
        void SaveScene (const std::string& filePath);

        CursorMode GetCursorMode () const { return m_CursorMode; }
        void SetCursorMode (CursorMode mode) { m_CursorMode = mode; }
        void SwitchCursorMode ();

        Point CalculateWorldPoint (int mouseX, int mouseY);
        bool IsMouseWithinRect (int mouseX, int mouseY, Rect rect);
        bool IsMouseWithinPointRect (int mouseX, int mouseY, Point point, int outsets = 0);

        MainLoop* GetMainLoop () { return m_MainLoop; }

        EditorPhysMode& GetEditorPhysMode () { return m_EditorPhysMode; }
        EditorActorMode& GetEditorActorMode () { return m_EditorActorMode; }
        EditorFlagPointMode& GetEditorFlagPointMode () { return m_EditorFlagPointMode; }
        EditorSpeechMode& GetEditorSpeechMode () { return m_EditorSpeechMode; }
        EditorTriggerAreaMode& GetEditorTriggerAreaMode () { return m_EditorTriggerAreaMode; }

        EditorScriptWindow* GetScriptWindow () { return m_ScriptWindow; }
        EditorComponentWindow* GetComponentWindow () { return m_ComponentWindow; }
        EditorQuestionWindow* GetEditorQuestionWindow () { return m_QuestionWindow; }

        ALLEGRO_TRANSFORM& GetWorldTransform () { return m_WorldTransform; }

        Point GetLastMousePos () { return m_LastMousePos; }

        bool IsSnapToGrid () { return m_IsSnapToGrid; }
        float GetGridSize () { return m_GridSize; }

        Rect GetSelectionRect () { return m_SelectionRect; }

        bool IsCloseCurrentPopup () { return m_CloseCurrentPopup; }
        void SetCloseCurrentPopup (bool close) { m_CloseCurrentPopup = close; }

    private:
        void LoadConfig ();
        void SaveConfig ();

        void DrawGrid ();
        void DrawSelectionRect (Rect rect);

        void ChangeGridSize (bool clockwise);
        bool IsEditorCanvasNotCovered ();

        void HandleCameraMovement (const ALLEGRO_MOUSE_EVENT& event);
        void HandleCameraPan (float deltaTime);

        void OnResetTranslate ();
        void OnResetScale ();

        void SetDrawUITiles (bool draw);

        void OnPlay ();

        void OnScrollPrevTiles ();
        void OnScrollNextTiles ();
        void OnBigScrollPrevTiles ();
        void OnBigScrollNextTiles ();
        void OnSpriteSheetEdit ();
        void OnCloseSpriteSheetEdit ();

        void OnShowGrid ();
        void OnGridIncrease ();
        void OnGridDecrease ();

        void OutlineBody ();
        void OnRemoveBody ();
        void OnNewPoly ();

        void OnSpeech ();
        void OnActorSelected ();

        void MarkPlayerPosition ();

        void OnTilesetSelected (const std::string& path);

        void RenderUI ();
        void RenderActorMode (float deltaTime);
        void RenderPhysBodyMode (float deltaTime);

        void RenderUINewScene ();
        void RenderUIOpenScene ();
        void RenderUISaveScene ();
        void RenderFlagPointWindow ();
        void RenderTriggerAreaWindow ();

        void ResetSettings ();
        void ScreenResize ();

        void ProcessMouseButtonDoubleClick (ALLEGRO_MOUSE_EVENT& event);
        void ProcessMouseButtonDown (ALLEGRO_MOUSE_EVENT& event);
        void ProcessMouseButtonUp (ALLEGRO_MOUSE_EVENT& event);
        void ProcessMouseAxes (ALLEGRO_MOUSE_EVENT& event);

        void TryToCloseWindows ();

        void AddActorsFromSpritesheet (float x, float y);
        void SelectActorsWithinSelectionRect ();

        void SelectPhysPoint ();
        void SelectTriggerAreaPoint ();

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

        Point m_LastMousePos;
        long m_LastTimeClicked;

        EditorSceneWindow* m_EditorSceneWindow;
        EditorSpeechWindow* m_SpeechWindow;
        EditorActorWindow* m_ActorWindow;
        EditorQuestionWindow* m_QuestionWindow;
        EditorScriptWindow* m_ScriptWindow;
        EditorComponentWindow* m_ComponentWindow;

        Gwk::Renderer::AllegroResourceLoader* m_ResourceLoader;
        Gwk::Renderer::Allegro* m_GUIRenderer;
        Gwk::Controls::Canvas* m_MainCanvas;
        Gwk::Skin::TexturedBase* m_GuiSkin;

        Gwk::Input::Allegro m_GUIInput;

        std::string m_LastScenePath;

        ALLEGRO_TRANSFORM m_WorldTransform;
        ALLEGRO_TRANSFORM m_NewTransform;
        bool m_OldSnapToGrid;

        Rect m_SelectionRect;
        bool m_IsRectSelection;

        bool m_CloseCurrentPopup;
        bool m_OpenPopupOpenScene;
        bool m_OpenPopupSaveScene;
        bool m_OpenPopupActorEditor;

        std::vector<std::string> m_RecentFileNames;
    };
}

#endif //   __EDITOR_H__
