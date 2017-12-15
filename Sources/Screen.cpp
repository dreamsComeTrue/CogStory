// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "Screen.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    const float TARGET_FPS = 60;
    const char* GAME_TITLE = "Robot Tale";

    //--------------------------------------------------------------------------------------------------

    Screen::Screen (unsigned width, unsigned height)
        : m_Width (width)
        , m_Height (height)
        , m_RealSize ((int)width, (int)height)
        , m_Redraw (false)
        , m_BackgroundColor (al_map_rgb (0, 0, 0))
    {
    }

    //--------------------------------------------------------------------------------------------------

    Screen::~Screen ()
    {
        if (!IsDestroyed ())
        {
            Destroy ();
        }
    }

    //--------------------------------------------------------------------------------------------------

    bool Screen::Initialize ()
    {
        if (!al_init ())
        {
            fprintf (stderr, "Failed to initialize allegro!\n");
            return false;
        }

        al_install_mouse ();
        al_install_keyboard ();

        al_set_new_display_flags (ALLEGRO_RESIZABLE);
        m_Display = al_create_display (m_Width, m_Height);

        if (!m_Display)
        {
            fprintf (stderr, "Failed to create display!\n");
            return false;
        }

        if (!al_init_image_addon ())
        {
            return false;
        }

        if (!al_install_audio ())
        {
            fprintf (stderr, "failed to initialize audio!\n");
            return false;
        }

        if (!al_init_acodec_addon ())
        {
            fprintf (stderr, "failed to initialize audio codecs!\n");
            return false;
        }

        if (!al_reserve_samples (3))
        {
            fprintf (stderr, "failed to reserve samples!\n");
            return false;
        }

        al_init_font_addon ();
        al_init_ttf_addon ();
        al_init_primitives_addon ();

        m_DisplayTimer = al_create_timer (1.0 / TARGET_FPS);

        if (!m_DisplayTimer)
        {
            fprintf (stderr, "Failed to create timer!\n");
            return false;
        }

        m_EventQueue = al_create_event_queue ();

        if (!m_EventQueue)
        {
            fprintf (stderr, "Failed to create event_queue!\n");
            al_destroy_display (m_Display);
            return false;
        }

        al_register_event_source (m_EventQueue, al_get_display_event_source (m_Display));
        al_register_event_source (m_EventQueue, al_get_timer_event_source (m_DisplayTimer));
        al_register_event_source (m_EventQueue, al_get_mouse_event_source ());
        al_register_event_source (m_EventQueue, al_get_keyboard_event_source ());

        al_set_window_title (m_Display, GAME_TITLE);
        al_set_window_position (m_Display, 0, 0);

        al_start_timer (m_DisplayTimer);

        m_Font.Initialize ();

        Lifecycle::Initialize ();

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    bool Screen::Destroy ()
    {
        al_uninstall_keyboard ();
        al_uninstall_mouse ();

        if (m_DisplayTimer != nullptr)
        {
            al_destroy_timer (m_DisplayTimer);
            m_DisplayTimer = nullptr;
        }

        if (m_EventQueue != nullptr)
        {
            al_destroy_event_queue (m_EventQueue);
            m_EventQueue = nullptr;
        }

        if (m_Display != nullptr)
        {
            al_destroy_display (m_Display);
            m_Display = nullptr;
        }

        m_Font.Destroy ();

        Lifecycle::Destroy ();
    }

    //--------------------------------------------------------------------------------------------------

    bool Screen::Update (float deltaTime)
    {
        m_DeltaTime = deltaTime;

        ALLEGRO_EVENT ev;
        al_wait_for_event (m_EventQueue, &ev);

        if (ev.type == ALLEGRO_EVENT_TIMER)
        {
            m_Redraw = true;
        }
        else if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
        {
            return false;
        }
        else if (ev.type == ALLEGRO_EVENT_DISPLAY_RESIZE)
        {
            m_RealSize = { ev.display.width, ev.display.height };

            al_acknowledge_resize (m_Display);

            if (ProcessEventFunction != nullptr)
            {
                ProcessEventFunction (&ev);
            }
        }
        else if ((ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) || (ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP)
            || (ev.type == ALLEGRO_EVENT_MOUSE_AXES) || (ev.type == ALLEGRO_EVENT_KEY_DOWN)
            || (ev.type == ALLEGRO_EVENT_KEY_UP) || (ev.type == ALLEGRO_EVENT_KEY_CHAR))
        {
            if (ProcessEventFunction != nullptr)
            {
                ProcessEventFunction (&ev);
            }
        }

        if (UpdateFunction != nullptr)
        {
            UpdateFunction (deltaTime);
        }

        if (m_Redraw && al_is_event_queue_empty (m_EventQueue))
        {
            if (RenderFunction != nullptr)
            {
                RenderFunction ();
            }

            al_flip_display ();
            al_clear_to_color (m_BackgroundColor);
            m_Redraw = false;
        }

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    void Screen::SetBackgroundColor (ALLEGRO_COLOR color) { m_BackgroundColor = color; }

    //--------------------------------------------------------------------------------------------------

    void Screen::SetBackgroundColor (float r, float g, float b, float a)
    {
        m_BackgroundColor = al_map_rgba (r, g, b, a);
    }

    //--------------------------------------------------------------------------------------------------

    void Screen::SetMouseCursor (const char* path)
    {
        ALLEGRO_BITMAP* bitmap = al_load_bitmap (path);
        ALLEGRO_MOUSE_CURSOR* cursor = al_create_mouse_cursor (bitmap, 0, 0);

        al_set_mouse_cursor (m_Display, cursor);
    }

    //--------------------------------------------------------------------------------------------------

    const Point& Screen::GetWindowSize () const { return m_RealSize; }

    //--------------------------------------------------------------------------------------------------

    Font& Screen::GetFont () { return m_Font; }

    //--------------------------------------------------------------------------------------------------

    ALLEGRO_DISPLAY* Screen::GetDisplay () { return m_Display; }

    //--------------------------------------------------------------------------------------------------

    ALLEGRO_EVENT_QUEUE* Screen::GetEventQueue () { return m_EventQueue; }

    //--------------------------------------------------------------------------------------------------

    float Screen::GetDeltaTime () const { return m_DeltaTime; }

    //--------------------------------------------------------------------------------------------------

    float Screen::GetFPS () const { return 1 / m_DeltaTime; }

    //--------------------------------------------------------------------------------------------------
}
