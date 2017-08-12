// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __EDITOR_H__
#define __EDITOR_H__

#include "Common.h"

namespace aga
{
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

        void OnMenuItemPlay (Gwk::Event::Info info);
        void MenuItemPlay ();
        void OnMenuItemExit (Gwk::Event::Info info);

    private:
        MainLoop* m_MainLoop;

        Gwk::Input::Allegro m_GworkInput;
        Gwk::Renderer::Allegro* m_Renderer;
        Gwk::Skin::TexturedBase* m_Skin;
        Gwk::Controls::Canvas* m_MainCanvas;

        bool m_DrawTiles;

        bool m_MousePan;
        bool m_MouseWheel;
    };
}

#endif //   __EDITOR_H__
