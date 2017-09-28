// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "Screen.h"
#include "Common.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    const float TARGET_FPS = 60;
    const char* GAME_TITLE = "Robot Tale";

    //--------------------------------------------------------------------------------------------------

    Screen::Screen (unsigned width, unsigned height)
      : m_Width (width)
      , m_Height (height)
      , m_Renderer (nullptr)
      , m_RealWidth (width)
      , m_RealHeight (height)
      , m_Redraw (false)
      , m_BackgroundColor (SDL_Color{ 0, 0, 0, 0 })
    {
    }

    //--------------------------------------------------------------------------------------------------

    Screen::~Screen ()
    {
        if (!IsDestroyed ())
        {
            Destroy ();
        }

        Lifecycle::Destroy ();
    }

    //--------------------------------------------------------------------------------------------------

    bool Screen::Initialize ()
    {
        if (SDL_Init (SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS | SDL_INIT_AUDIO) != 0)
        {
            SDL_Log ("Unable to initialize SDL: %s", SDL_GetError ());
            return false;
        }

        // Initialize PNG loading
        int imgFlags = IMG_INIT_PNG | IMG_INIT_JPG;
        if (!(IMG_Init (imgFlags) & imgFlags))
        {
            SDL_Log ("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError ());
            return false;
        }

        // Initialize SDL_ttf
        if (TTF_Init () == -1)
        {
            SDL_Log ("SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError ());
            return false;
        }

        m_Display = SDL_CreateWindow (GAME_TITLE,                              // window title
                                      SDL_WINDOWPOS_UNDEFINED,                 // initial x position
                                      SDL_WINDOWPOS_UNDEFINED,                 // initial y position
                                      m_Width,                                 // width, in pixels
                                      m_Height,                                // height, in pixels
                                      SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE // flags - see below
                                      );

        if (!m_Display)
        {
            fprintf (stderr, "Failed to create display!\n");
            return false;
        }

        m_Renderer = SDL_CreateRenderer (m_Display, -1, SDL_RENDERER_ACCELERATED);

        m_Font = new Font (this);
        m_Font->Initialize ();

        Lifecycle::Initialize ();

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    bool Screen::Destroy ()
    {
        SAFE_DELETE (m_Font);

        if (m_Renderer != nullptr)
        {
            SDL_DestroyRenderer (m_Renderer);
            m_Renderer = nullptr;
        }

        if (m_Display != nullptr)
        {
            SDL_DestroyWindow (m_Display);
            m_Display = nullptr;
        }

        TTF_Quit ();
        IMG_Quit ();
        SDL_Quit ();

        Lifecycle::Destroy ();
    }

    //--------------------------------------------------------------------------------------------------

    bool Screen::Update (double deltaTime)
    {
        m_DeltaTime = deltaTime;

        SDL_Event e;
        while (SDL_PollEvent (&e) != 0)
        {
            if (e.type == SDL_QUIT)
            {
                return false;
            }

            if (e.type == SDL_WINDOWEVENT)
            {
                switch (e.window.type)
                {
                    case SDL_WINDOWEVENT_SIZE_CHANGED:
                    {
                        m_RealWidth = e.window.data1;
                        m_RealHeight = e.window.data2;
                        SDL_RenderPresent (m_Renderer);

                        if (ProcessEventFunction != nullptr)
                        {
                            ProcessEventFunction (&e);
                        }

                        break;
                    }

                    // Repaint on exposure
                    case SDL_WINDOWEVENT_EXPOSED:
                    {
                        SDL_RenderPresent (m_Renderer);
                        break;
                    }
                }
            }
            else if (e.type == SDL_KEYDOWN || e.type == SDL_KEYUP || e.type == SDL_MOUSEMOTION || e.type == SDL_MOUSEBUTTONDOWN ||
                     e.type == SDL_MOUSEBUTTONUP || e.type == SDL_MOUSEWHEEL)
            {
                if (ProcessEventFunction != nullptr)
                {
                    ProcessEventFunction (&e);
                }
            }
        }

        SDL_SetRenderDrawColor (m_Renderer, m_BackgroundColor.r, m_BackgroundColor.g, m_BackgroundColor.b, m_BackgroundColor.a);
        SDL_RenderClear (m_Renderer);

        if (RenderFunction != nullptr)
        {
            RenderFunction ();
        }

        SDL_RenderPresent (m_Renderer);

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    void Screen::SetBackgroundColor (SDL_Color color) { m_BackgroundColor = color; }

    //--------------------------------------------------------------------------------------------------

    void Screen::SetMouseCursor (const char* path)
    {
        //        ALLEGRO_BITMAP* bitmap = al_load_bitmap (path);
        //        ALLEGRO_MOUSE_CURSOR* cursor = al_create_mouse_cursor (bitmap, 0, 0);

        //        al_set_mouse_cursor (m_Display, cursor);
    }

    //--------------------------------------------------------------------------------------------------

    Point Screen::GetScreenSize () { return Point{ m_RealWidth, m_RealHeight }; }

    //--------------------------------------------------------------------------------------------------

    Font* Screen::GetFont () { return m_Font; }

    //--------------------------------------------------------------------------------------------------

    double Screen::GetDeltaTime () const { return m_DeltaTime; }

    //--------------------------------------------------------------------------------------------------

    double Screen::GetFPS () const { return 1 / m_DeltaTime * 1000; }

    //--------------------------------------------------------------------------------------------------

    SDL_Window* Screen::GetDisplay () { return m_Display; }

    //--------------------------------------------------------------------------------------------------

    SDL_Renderer* Screen::GetRenderer () { return m_Renderer; }

    //--------------------------------------------------------------------------------------------------

    SDL_Texture* Screen::LoadTexture (const std::string& path)
    {
        SDL_Texture* newTexture = nullptr;

        // Load image at specified path
        SDL_Surface* loadedSurface = IMG_Load (path.c_str ());
        if (loadedSurface == nullptr)
        {
            printf ("Unable to load image %s! SDL_image Error: %s\n", path.c_str (), IMG_GetError ());
        }
        else
        {
            // Create texture from surface pixels
            newTexture = SDL_CreateTextureFromSurface (m_Renderer, loadedSurface);

            if (newTexture == nullptr)
            {
                printf ("Unable to create texture from %s! SDL Error: %s\n", path.c_str (), SDL_GetError ());
            }

            // Get rid of old loaded surface
            SDL_FreeSurface (loadedSurface);
        }

        return newTexture;
    }

    //--------------------------------------------------------------------------------------------------
}
