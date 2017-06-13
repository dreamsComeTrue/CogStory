// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __SCREEN_H__
#define __SCREEN_H__

#include "Common.h"
#include "Font.h"

#include <functional>

struct ALLEGRO_DISPLAY;
struct ALLEGRO_EVENT_QUEUE;
struct ALLEGRO_TIMER;
union ALLEGRO_EVENT;

namespace aga
{
    class Screen : public Lifecycle
    {
    public:
        Screen (unsigned width, unsigned height);
        virtual ~Screen ();
        bool Initialize ();
        bool Destroy ();

        bool Update (double deltaTime);

        void SetMouseCursor (const char* path);

        const Point GetScreenSize ();
        Font& GetFont ();

        double GetDeltaTime () const;
        double GetFPS () const;

        std::function<void(ALLEGRO_EVENT*)> ProcessEventFunction;
        std::function<void()> RenderFunction;

    private:
        unsigned m_Width, m_Height;
        int m_RealWidth, m_RealHeight;
        bool m_Redraw;
        double m_DeltaTime;

        Font m_Font;

        ALLEGRO_DISPLAY* m_Display;
        ALLEGRO_EVENT_QUEUE* m_EventQueue;
        ALLEGRO_TIMER* m_DisplayTimer;
    };
}

#endif //   __SCREEN_H__
