// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __EDITOR_TILES_WINDOW_H__
#define __EDITOR_TILES_WINDOW_H__

#include "Animation.h"
#include "Common.h"

namespace aga
{
    class Editor;

    class EditorTilesWindow
    {
    public:
        EditorTilesWindow (Editor* editor);
        virtual ~EditorTilesWindow ();

        void Show (std::function<void(std::string, std::string)> OnAcceptFunc,
            std::function<void(std::string, std::string)> OnCancelFunc);

        void Render ();
        void RenderSpritesheet ();

        bool IsVisible ();

    private:
        void OnSave ();
        void OnSavePack ();
        void OnAccept ();
        void OnCancel ();

    private:
        Editor* m_Editor;
        ALLEGRO_BITMAP* m_Image;

        char m_Name[100];
        char m_Path[300];
        int m_TilesX;
        int m_TilesY;
        bool m_DrawGrid;

        bool m_IsVisible;
        bool m_SaveTilesButtonPressed;
        bool m_SavePackButtonPressed;
        bool m_OpenImageButtonPressed;

        std::function<void(std::string, std::string)> m_OnAcceptFunc;
        std::function<void(std::string, std::string)> m_OnCancelFunc;

        int m_Width, m_Height;
    };
}

#endif //   __EDITOR_TILES_WINDOW_H__
