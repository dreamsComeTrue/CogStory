// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __EDITOR_H__
#define __EDITOR_H__

#include "Common.h"
#include "UIManager.h"

namespace aga
{
    class MainLoop;
    class Frame;
    class TextBox;

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

    private:
        MainLoop* m_MainLoop;
        UIManager m_UIManager;
        bool m_DrawTiles;

        bool m_MousePan;
        bool m_MouseWheel;

        Frame* m_TilesFrames[8];
        TextBox* m_NewSceneTitle;
    };
}

#endif //   __EDITOR_H__
