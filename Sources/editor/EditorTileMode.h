// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __EDITOR_TILE_MODE_H__
#define __EDITOR_TILE_MODE_H__

#include "Atlas.h"
#include "Common.h"
#include "Scene.h"

namespace aga
{
    extern const int TILES_COUNT;

    class MainLoop;
    class Editor;
    class EditorPhysMode;
    struct Tile;

    class EditorTileMode
    {
        friend class Editor;
        friend class EditorPhysMode;

    public:
        EditorTileMode (Editor* editor);
        virtual ~EditorTileMode ();

    private:
        void SetCurrentID (int id);

        void InitializeUI ();
        void DrawTiles ();

        bool ChooseTile (int mouseX, int mouseY);
        Tile* AddTile (int mouseX, int mouseY);

        void ChangeRotation (bool clockwise);
        void ChangeZOrder (bool clockwise);
        void ChangeGridSize (bool clockwise);
        bool MoveSelectedTile ();
        void RemoveSelectedTile ();
        void CopySelectedTile ();

        Tile* GetTileUnderCursor (int mouseX, int mouseY, Rect&& outRect);
        Rect GetRenderBounds (Tile* tile);

        void OnTileSelected ();

        void ResetSettings ();

    private:
        Editor* m_Editor;

        Atlas* m_Atlas;
        AtlasRegion m_SelectedAtlasRegion;

        float m_Rotation;

        Tile* m_TileUnderCursor;
        Tile* m_SelectedTile;

        bool m_IsDrawTiles;
    };
}

#endif //   __EDITOR_TILE_MODE_H__
