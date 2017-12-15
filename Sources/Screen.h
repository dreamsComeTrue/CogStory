// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __SCREEN_H__
#define __SCREEN_H__

#include "Common.h"
#include "Font.h"

namespace aga
{
    class Screen : public Lifecycle
    {
    public:
        Screen (unsigned width, unsigned height);
        virtual ~Screen ();
        bool Initialize ();
        bool Destroy ();

        bool Update (float deltaTime);

        void SetBackgroundColor (ALLEGRO_COLOR color);
        void SetBackgroundColor (float r, float g, float b, float a);

        void SetMouseCursor (const char* path);

        const Point& GetWindowSize () const;
        Font& GetFont ();

        float GetDeltaTime () const;
        float GetFPS () const;

        ALLEGRO_DISPLAY* GetDisplay ();
        ALLEGRO_EVENT_QUEUE* GetEventQueue ();

        std::function<void(ALLEGRO_EVENT*)> ProcessEventFunction;
        std::function<void(float deltaTime)> UpdateFunction;
        std::function<void()> RenderFunction;

    private:
        Point m_RealSize;
        unsigned m_Width, m_Height;
        bool m_Redraw;
        float m_DeltaTime;

        Font m_Font;

        ALLEGRO_COLOR m_BackgroundColor;
        ALLEGRO_DISPLAY* m_Display;
        ALLEGRO_EVENT_QUEUE* m_EventQueue;
        ALLEGRO_TIMER* m_DisplayTimer;
    };
}

#endif //   __SCREEN_H__
