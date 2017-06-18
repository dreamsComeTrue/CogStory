// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __WIDGET_H__
#define __WIDGET_H__

#include "Common.h"

namespace aga
{
    class Screen;

    class Widget : public Lifecycle
    {
    public:
        Widget (Screen* screen);
        virtual ~Widget ();
        bool Initialize ();
        bool Destroy ();

        virtual bool Update (double deltaTime);
        virtual void ProcessEvent (ALLEGRO_EVENT* event, double deltaTime);
        virtual void Render (double deltaTime);

        void SetPosition (int x, int y);
        void SetPosition (Point pos);
        Point GetPosition ();

    protected:
        Screen* m_Screen;
        Point m_Position;
    };
}

#endif //   __WIDGET_H__
