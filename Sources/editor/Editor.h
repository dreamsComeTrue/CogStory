// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __EDITOR_H__
#define __EDITOR_H__

#include "Atlas.h"
#include "Common.h"

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
        EditFlagPointsMode
    };

    class Editor : public Lifecycle
    {
    public:
        Editor (MainLoop* mainLoop);
        virtual ~Editor ();
        bool Initialize ();
        bool Destroy ();

        bool Update (float deltaTime);
        void ProcessEvent (ALLEGRO_EVENT* event, float deltaTime);
        void Render (float deltaTime);

        void SetDrawUITiles (bool draw);
        void OnResetTranslate ();
        void OnResetScale ();

    private:
        void InitializeUI ();
        void DrawTiles ();
        void DrawGrid ();
        void DrawPhysBody (float mouseX, float mouseY);
        void DrawFlagPoints ();

        bool ChooseTile (int mouseX, int mouseY);
        Tile* AddTile (int mouseX, int mouseY);

        void ChangeRotation (bool clockwise);
        void ChangeZOrder (bool clockwise);
        void ChangeGridSize (bool clockwise);
        void MoveSelectedTile ();
        void MoveSelectedPhysPoint ();
        void RemoveSelectedTile ();
        void CopySelectedTile ();

        Tile* GetTileUnderCursor (int mouseX, int mouseY, Rect&& outRect);
        Rect GetRenderBounds (Tile* tile);

        void HandleCameraMovement (const ALLEGRO_MOUSE_EVENT& event);

        void OnNewScene ();
        void OnLoadScene (const std::string& filePath);
        void OnSaveScene (const std::string& filePath);

        void OnPlay ();
        void MenuItemPlay ();

        void OnExit ();
        void OnExitYesButton ();
        void OnExitNoButton ();

        void OnTileSelected ();

        void OnShowGrid ();
        void OnGridIncrease ();
        void OnGridDecrease ();

        void InsertFlagPointAtCursor (int mouseX, int mouseY);

        void InsertPhysPointAtCursor (int mouseX, int mouseY);
        Point* GetPhysPointUnderCursor (int mouseX, int mouseY);
        void RemovePhysPointUnderCursor (int mouseX, int mouseY);

        bool RemoveFlagPointUnderCursor (int mouseX, int mouseY);

        Point CalculateCursorPoint (int mouseX, int mouseY);

        bool IsMouseWithinPointRect (int mouseX, int mouseY, Point point, int outsets);

        void RenderUI ();

        void Resize ();
        void ResetSettings ();

    private:
        MainLoop* m_MainLoop;

        Atlas* m_Atlas;
        AtlasRegion m_SelectedAtlasRegion;

        float m_Rotation;

        CursorMode m_CursorMode;

        Tile* m_TileUnderCursor;
        Tile* m_SelectedTile;

        bool m_IsDrawTiles;

        bool m_IsSnapToGrid;
        int m_BaseGridSize;
        int m_GridSize;

        bool m_IsMousePan;
        bool m_IsMouseWheel;

        Point* m_PhysPoint;
        int m_PhysPointIndex;
        std::vector<Point>* m_PhysPoly;
    };
}

#endif //   __EDITOR_H__
