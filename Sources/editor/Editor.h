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
        TileInsertMode,
        TileSelectMode,
        TileEditMode
    };

    class Editor : public Lifecycle
    {
    public:
        Editor (MainLoop* mainLoop);
        virtual ~Editor ();
        bool Initialize ();
        bool Destroy ();

        bool Update (double deltaTime);
        void ProcessEvent (ALLEGRO_EVENT* event, double deltaTime);
        void Render (double deltaTime);

    private:
        void InitializeUI ();
        void DrawTiles ();
        void DrawGrid ();

        bool ChooseTile (int mouseX, int mouseY);
        void AddTile (int mouseX, int mouseY);

        Tile* GetTileUnderCursor (int mouseX, int mouseY, Rect&& outRect);
        Rect GetRenderBounds (Tile& tile);

        void OnNewScene ();
        void OnNewSceneYesButton ();
        void OnNewSceneNoButton ();

        void OnLoadScene ();
        void OnSaveScene ();

        void OnPlay ();
        void MenuItemPlay ();

        void OnExit ();
        void OnExitYesButton ();
        void OnExitNoButton ();

        void OnTileSelected ();

        void OnResetTranslate ();
        void OnResetScale ();
        void OnShowGrid ();
        void OnGridIncrease ();
        void OnGridDecrease ();

        Point CalculateCursorPoint (int mouseX, int mouseY);
        void RenderUI ();

        void Resize ();

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
    };
}

#endif //   __EDITOR_H__
