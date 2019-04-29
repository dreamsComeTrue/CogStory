// Copyright 2017-2019 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __EDITOR_H__
#define __EDITOR_H__

#include "Common.h"
#include "EditorActorMode.h"
#include "EditorFlagPointMode.h"
#include "EditorPhysMode.h"
#include "EditorSpeechMode.h"
#include "EditorTriggerAreaMode.h"
#include "EditorUndoRedo.h"

//--------------------------------------------------------------------------------------------------

namespace ImGui
{
    bool Combo (const char* label, int* currIndex, std::vector<std::string>& values);
}

//--------------------------------------------------------------------------------------------------

namespace aga
{
    extern const int TILES_COUNT;

    class EditorSaveSceneWindow;
    class EditorOpenSceneWindow;
    class EditorSpeechWindow;
    class EditorActorWindow;
    class EditorActorScriptWindow;
    class EditorSceneWindow;
    class EditorScriptSelectWindow;
    class EditorScriptWindow;
    class EditorComponentWindow;
    class EditorAnimationWindow;
    class EditorTilesWindow;

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

    class Editor : public Lifecycle
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

        CursorMode GetCursorMode () const;
        void SetCursorMode (CursorMode mode);
        void SwitchCursorMode ();

        Point CalculateWorldPoint (int mouseX, int mouseY);
        bool IsMouseWithinRect (int mouseX, int mouseY, Rect rect);
        bool IsMouseWithinPointRect (int mouseX, int mouseY, Point point, int outsets = 0);

        MainLoop* GetMainLoop ();

        EditorPhysMode& GetEditorPhysMode ();
        EditorActorMode& GetEditorActorMode ();
        EditorFlagPointMode& GetEditorFlagPointMode ();
        EditorSpeechMode& GetEditorSpeechMode ();
        EditorTriggerAreaMode& GetEditorTriggerAreaMode ();

        EditorScriptSelectWindow* GetScriptSelectWindow ();
        EditorComponentWindow* GetComponentWindow ();
        EditorAnimationWindow* GetAnimationWindow ();
        EditorSpeechWindow* GetSpeechWindow ();
        EditorActorWindow* GetActorWindow ();
        EditorScriptWindow* GetScriptWindow ();

        Point GetLastMousePos ();

        bool IsSnapToGrid ();
        float GetGridSize ();

        Rect GetSelectionRect ();

        bool IsCloseCurrentPopup ();
        void SetCloseCurrentPopup (bool close);
        
        EditorUndoRedo* GetUndoRedo ();

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

        void OnOpenScene ();
        void OnSaveScene ();
        void OnSpeech ();
        void OnActorSelected ();
        void OnAnimation ();
        void OnTilesEditor ();
        void OnScriptEditor ();

        void MarkPlayerPosition ();

        void OnTilesetSelected (const std::string& path);

        void RenderUI ();
        void RenderActorMode (float deltaTime);
        void RenderPhysBodyMode (float deltaTime);

        void RenderUINewScene ();

        void ResetSettings ();
        void ScreenResize ();

        void ProcessMouseButtonDoubleClick (ALLEGRO_MOUSE_EVENT& event);
        void ProcessMouseButtonDown (ALLEGRO_MOUSE_EVENT& event);
        void ProcessMouseButtonUp (ALLEGRO_MOUSE_EVENT& event);
        void ProcessMouseAxes (ALLEGRO_MOUSE_EVENT& event);

        void TryToCloseWindows ();

        void AddActorsFromSpritesheet (int x, int y);
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
        EditorUndoRedo m_EditorUndoRedo;

        CursorMode m_CursorMode;

        bool m_IsSnapToGrid;
        float m_BaseGridSize;
        float m_GridSize;

        bool m_IsMousePan;
        bool m_IsMouseDrag;

        Point m_LastMousePos;
        long m_LastTimeClicked;

        EditorOpenSceneWindow* m_OpenSceneWindow;
        EditorSaveSceneWindow* m_SaveSceneWindow;
        EditorSceneWindow* m_EditorSceneWindow;
        EditorSpeechWindow* m_SpeechWindow;
        EditorActorWindow* m_ActorWindow;
        EditorScriptSelectWindow* m_ScriptSelectWindow;
        EditorComponentWindow* m_ComponentWindow;
        EditorAnimationWindow* m_AnimationWindow;
        EditorTilesWindow* m_TilesWindow;
        EditorScriptWindow* m_ScriptWindow;

        std::string m_LastScenePath;

        ALLEGRO_TRANSFORM m_WorldTransform;
        ALLEGRO_TRANSFORM m_NewTransform;
        bool m_OldSnapToGrid;
        float m_OldGridSize;

        Rect m_SelectionRect;
        bool m_IsRectSelection;

        bool m_CloseCurrentPopup;
        bool m_OpenPopupOpenScene;
        bool m_OpenPopupSaveScene;
        bool m_OpenPopupActorEditor;
        bool m_OpenPopupFlagPointEditor;
        bool m_OpenPopupTriggerAreaEditor;
        bool m_OpenPopupSpeechEditor;
        bool m_OpenPopupAnimationEditor;
        bool m_OpenPopupTilesEditor;
        bool m_OpenPopupScriptEditor;

        char m_FlagPointName[100];

        bool m_DrawActors;
        bool m_DrawFlagPoints;
        bool m_DrawTriggerAreas;
        bool m_DrawCameraBounds;
        
        bool m_NoSceneSelected;
    };
}

#endif //   __EDITOR_H__
