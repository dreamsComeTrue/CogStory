// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __EDITOR_H__
#define __EDITOR_H__

#include "Common.h"

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
        Screen* m_Screen;
        bool m_DrawTiles;
    };
}

#endif //   __EDITOR_H__
