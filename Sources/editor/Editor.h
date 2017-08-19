// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __EDITOR_H__
#define __EDITOR_H__

#include "Atlas.h"
#include "Common.h"

namespace aga
{
    extern const int TILES_COUNT;

    class MainLoop;

    class Editor : public Lifecycle, public Gwk::Event::Handler
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

        void SelectTile (int mouseX, int mouseY);
        void AddTile (int mouseX, int mouseY);

        void OnMenuItemPlay (Gwk::Event::Info info);
        void MenuItemPlay ();
        void OnMenuItemExit (Gwk::Event::Info info);
        void OnTileSelected (Gwk::Event::Info info);

    private:
        MainLoop* m_MainLoop;

        Gwk::Input::Allegro m_GworkInput;
        Gwk::Renderer::Allegro* m_Renderer;
        Gwk::Skin::TexturedBase* m_Skin;
        Gwk::Controls::Canvas* m_MainCanvas;

        Atlas m_Atlas;
        AtlasRegion m_SelectedAtlasRegion;
        bool m_IsAtlasRegionSelected;

        float m_Rotation;

        std::vector<Gwk::Controls::ImagePanel*> m_ImagePanels;

        bool m_IsDrawTiles;

        bool m_IsSnapToGrid;
        int m_GridSize;

        bool m_IsMousePan;
        bool m_IsMouseWheel;
    };
}

#endif //   __EDITOR_H__
