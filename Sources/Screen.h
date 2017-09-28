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

        bool Update (double deltaTime);

        void SetBackgroundColor (SDL_Color color);

        void SetMouseCursor (const char* path);

        Point GetScreenSize ();
        Font* GetFont ();

        double GetDeltaTime () const;
        double GetFPS () const;

        SDL_Window* GetDisplay ();
        SDL_Renderer* GetRenderer ();

        SDL_Texture* LoadTexture (const std::string& path);

        std::function<void(SDL_Event*)> ProcessEventFunction;
        std::function<void()> RenderFunction;

    private:
        unsigned m_Width, m_Height;
        int m_RealWidth, m_RealHeight;
        bool m_Redraw;
        double m_DeltaTime;

        Font* m_Font;

        SDL_Color m_BackgroundColor;
        SDL_Window* m_Display;
        SDL_Renderer* m_Renderer;
    };
}

#endif //   __SCREEN_H__
