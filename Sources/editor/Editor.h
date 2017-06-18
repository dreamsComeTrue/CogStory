// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __EDITOR_H__
#define __EDITOR_H__

#include "Common.h"
#include "UIManager.h"

namespace aga
{
    class Screen;

    class Editor : public Lifecycle
    {
    public:
        Editor (Screen* screen);
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
        Screen* m_Screen;
        UIManager m_UIManager;
        bool m_DrawTiles;
    };
}

#endif //   __EDITOR_H__
