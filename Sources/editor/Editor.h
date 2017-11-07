// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __EDITOR_H__
#define __EDITOR_H__

#include "Common.h"
#include "EditorFlagPointMode.h"
#include "EditorPhysMode.h"
#include "EditorTileMode.h"
#include "EditorTriggerAreaMode.h"
#include "Scene.h"

namespace aga
{
    extern const int TILES_COUNT;

    class MainLoop;
    struct Tile;

    enum CursorMode
    {
        TileSelectMode,
        TileEditMode,
        EditPhysBodyMode,
        EditFlagPointsMode,
        EditTriggerAreaMode
    };

    class Editor : public Lifecycle
    {
        friend class EditorTileMode;
        friend class EditorPhysMode;
        friend class EditorFlagPointMode;
        friend class EditorTriggerAreaMode;

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
        void DrawGrid ();

        void ChangeGridSize (bool clockwise);

        void HandleCameraMovement (const ALLEGRO_MOUSE_EVENT& event);

        void OnNewScene ();
        void OnLoadScene (const std::string& filePath);
        void OnSaveScene (const std::string& filePath);

        void OnPlay ();
        void MenuItemPlay ();

        void OnExit ();

        void OnShowGrid ();
        void OnGridIncrease ();
        void OnGridDecrease ();

        Point CalculateCursorPoint (int mouseX, int mouseY);

        bool IsMouseWithinPointRect (int mouseX, int mouseY, Point point, int outsets);

        void RenderUI ();

        void ResetSettings ();

    private:
        MainLoop* m_MainLoop;
        EditorTileMode m_EditorTileMode;
        EditorPhysMode m_EditorPhysMode;
        EditorFlagPointMode m_EditorFlagPointMode;
        EditorTriggerAreaMode m_EditorTriggerAreaMode;

        CursorMode m_CursorMode;

        bool m_IsSnapToGrid;
        int m_BaseGridSize;
        int m_GridSize;

        bool m_IsMousePan;
        bool m_IsMouseWheel;
    };
}

#endif //   __EDITOR_H__
