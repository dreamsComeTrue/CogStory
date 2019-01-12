// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "Screen.h"

namespace aga
{
	//--------------------------------------------------------------------------------------------------

	const double TARGET_FPS = 60.0;

	//--------------------------------------------------------------------------------------------------

	Screen* Screen::m_Singleton = nullptr;

	//--------------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------------

	Screen::Screen (unsigned screenWidth, unsigned screenHeight, unsigned gameWindowWidth, unsigned gameWindowHeight,
		const char* gameTitle, bool centerOnScreen)
		: m_GameTitle (gameTitle)
		, m_Width (screenWidth)
		, m_Height (screenHeight)
		, m_GameWindowWidth (gameWindowWidth)
		, m_GameWindowHeight (gameWindowHeight)
		, m_RealSize (static_cast<int> (screenWidth), static_cast<int> (screenHeight))
		, m_CenterOnScreen (centerOnScreen)
		, m_Redraw (false)
		, m_BackgroundColor (al_map_rgb (0, 0, 0))
		, m_DrawFilled (false)
		, m_BackBuffer (nullptr)
	{
		m_Singleton = this;
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
		Lifecycle::Initialize ();

		// use current time as seed for random generator
		std::srand (static_cast<unsigned> (std::time (nullptr)));

		if (!al_init ())
		{
			Log ("Failed to initialize allegro!\n");
			return false;
		}

		if (!al_install_mouse ())
		{
			return false;
		}

		if (!al_install_keyboard ())
		{
			return false;
		}

		if (!al_install_joystick ())
		{
			return false;
		}

		al_reconfigure_joysticks ();

		al_set_new_display_flags (ALLEGRO_RESIZABLE);
		m_Display = al_create_display (static_cast<int> (m_Width), static_cast<int> (m_Height));

		if (!m_Display)
		{
			Log ("Failed to create display!\n");
			return false;
		}

		if (m_CenterOnScreen)
		{
			CenterOnScreen ();
		}
		else
		{
#ifdef __linux__
			int winX = 0; //    px - frame border;
			int winY = 0; //    px - frame border;
#else
			int winX = 2; //    px - frame border;
			int winY = 32; //    px - frame border;
#endif

			al_set_window_position (m_Display, winX, winY);
		}

		if (!al_init_image_addon ())
		{
			return false;
		}

		if (!al_install_audio ())
		{
			Log ("failed to initialize audio!\n");
			return false;
		}

		if (!al_init_acodec_addon ())
		{
			Log ("failed to initialize audio codecs!\n");
			return false;
		}

		if (!al_reserve_samples (3))
		{
			Log ("failed to reserve samples!\n");
			return false;
		}

		if (!al_init_font_addon ())
		{
			return false;
		}

		if (!al_init_ttf_addon ())
		{
			return false;
		}

		if (!al_init_primitives_addon ())
		{
			return false;
		}

		m_DisplayTimer = al_create_timer (1.0 / TARGET_FPS);

		if (!m_DisplayTimer)
		{
			Log ("Failed to create timer!\n");
			return false;
		}

		m_EventQueue = al_create_event_queue ();

		if (!m_EventQueue)
		{
			Log ("Failed to create event_queue!\n");
			al_destroy_display (m_Display);
			return false;
		}

		al_register_event_source (m_EventQueue, al_get_display_event_source (m_Display));
		al_register_event_source (m_EventQueue, al_get_timer_event_source (m_DisplayTimer));
		al_register_event_source (m_EventQueue, al_get_joystick_event_source ());
		al_register_event_source (m_EventQueue, al_get_mouse_event_source ());
		al_register_event_source (m_EventQueue, al_get_keyboard_event_source ());

		al_set_window_title (m_Display, m_GameTitle.c_str ());

		al_start_timer (m_DisplayTimer);

		m_Font.Initialize ();

		al_set_new_bitmap_flags (ALLEGRO_VIDEO_BITMAP);

		ALLEGRO_BITMAP* windowIcon = al_load_bitmap ((GetDataPath () + "/gfx/icon.png").c_str ());
		al_set_display_icon (m_Display, windowIcon);

		InvalidateBackBuffer (m_Width, m_Height);

		return true;
	}

	//--------------------------------------------------------------------------------------------------

	bool Screen::Destroy ()
	{
		m_Font.Destroy ();

		if (m_DisplayTimer != nullptr)
		{
			al_stop_timer (m_DisplayTimer);
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

		al_destroy_bitmap (m_BackBuffer);

		al_shutdown_primitives_addon ();
		al_shutdown_font_addon ();

		al_uninstall_keyboard ();
		al_uninstall_mouse ();

		return Lifecycle::Destroy ();
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
		else if (ev.type == ALLEGRO_EVENT_JOYSTICK_CONFIGURATION)
		{
			al_reconfigure_joysticks ();
		}
		else if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
		{
			return false;
		}
		else if (ev.type == ALLEGRO_EVENT_DISPLAY_RESIZE)
		{
			OnResize ();

			if (ProcessEventFunction != nullptr)
			{
				ProcessEventFunction (&ev);
			}
		}
		else if ((ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) || (ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP)
			|| (ev.type == ALLEGRO_EVENT_MOUSE_AXES) || (ev.type == ALLEGRO_EVENT_KEY_DOWN)
			|| (ev.type == ALLEGRO_EVENT_KEY_UP) || (ev.type == ALLEGRO_EVENT_KEY_CHAR)
			|| (ev.type == ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN) || (ev.type == ALLEGRO_EVENT_JOYSTICK_BUTTON_UP)
			|| (ev.type == ALLEGRO_EVENT_JOYSTICK_AXIS))
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
			al_set_target_bitmap (m_BackBuffer);
			al_clear_to_color (m_BackgroundColor);

			if (RenderFunction != nullptr)
			{
				RenderFunction ();
			}

			DrawDebugMessages ();

			al_set_target_bitmap (al_get_backbuffer (m_Display));
			al_clear_to_color (COLOR_BLACK);

			const Point size = GetRealWindowSize ();

			if (m_DrawFilled)
			{
				al_draw_scaled_bitmap (m_BackBuffer, 0, 0, size.Width, size.Height, 0, 0, size.Width, size.Height, 0);
			}
			else
			{
				//  Calculate scaling ratio
				Point backBufferSize = GetBackBufferSize ();

				float sx = size.Width / backBufferSize.Width;
				float sy = size.Height / backBufferSize.Height;
				float scale = std::min (sx, sy);

				// calculate how much the buffer should be scaled
				float scaleW = backBufferSize.Width * scale;
				float scaleH = backBufferSize.Height * scale;

				m_BackBufferOffset.X = (size.Width - scaleW) / 2;
				m_BackBufferOffset.Y = (size.Height - scaleH) / 2;

				al_draw_scaled_bitmap (m_BackBuffer, 0, 0, m_Width, m_Height, m_BackBufferOffset.X,
					m_BackBufferOffset.Y, scaleW, scaleH, 0);
			}

			al_flip_display ();

			m_Redraw = false;
		}

		return true;
	}

	//--------------------------------------------------------------------------------------------------

	void Screen::CenterOnScreen ()
	{
		ALLEGRO_MONITOR_INFO aminfo;
		al_get_monitor_info (0, &aminfo);
		int desktopWidth = aminfo.x2 - aminfo.x1 + 1;
		int desktopHeight = aminfo.y2 - aminfo.y1 + 1;

		int winX = static_cast<int> (desktopWidth / 2 - m_RealSize.Width / 2);
		int winY = static_cast<int> (desktopHeight / 2 - m_RealSize.Height / 2);

		al_set_window_position (m_Display, winX, winY);
	}

	//--------------------------------------------------------------------------------------------------

	void Screen::DrawDebugMessages ()
	{
		for (size_t i = 0; i < m_DebugMessages.size (); ++i)
		{
			DebugMessage& msg = m_DebugMessages.at (i);
			msg.ActualDuration += (m_DeltaTime * 1000.0f);

			if (msg.ActualDuration >= msg.MaxDuration)
			{
				m_DebugMessages.erase (m_DebugMessages.begin () + i);
			}
			else
			{
				GetFont ().DrawText (
					FONT_NAME_SMALL, msg.Message, msg.Color, 10, 10 + i * 10, 1.0f, ALLEGRO_ALIGN_LEFT);
			}
		}
	}

	//--------------------------------------------------------------------------------------------------

	void Screen::SetBackgroundColor (ALLEGRO_COLOR color) { m_BackgroundColor = color; }

	//--------------------------------------------------------------------------------------------------

	void Screen::SetBackgroundColor (float r, float g, float b, float a)
	{
		m_BackgroundColor = al_map_rgba_f (r, g, b, a);
	}

	//--------------------------------------------------------------------------------------------------

	void Screen::SetMouseCursor (const char* path)
	{
		ALLEGRO_BITMAP* bitmap = al_load_bitmap (path);
		ALLEGRO_MOUSE_CURSOR* cursor = al_create_mouse_cursor (bitmap, 0, 0);

		al_set_mouse_cursor (m_Display, cursor);
	}

	//--------------------------------------------------------------------------------------------------

	void Screen::SetWindowSize (Point size)
	{
		m_RealSize = size;

		al_resize_display (m_Display, static_cast<int> (m_RealSize.Width), static_cast<int> (m_RealSize.Height));
		al_acknowledge_resize (m_Display);
	}

	//--------------------------------------------------------------------------------------------------

	Point& Screen::GetRealWindowSize ()
	{
		int width = al_get_display_width (m_Display);
		int height = al_get_display_height (m_Display);

		if (width != static_cast<int> (m_RealSize.Width) || height != static_cast<int> (m_RealSize.Height))
		{
			m_RealSize = {width, height};
		}

		return m_RealSize;
	}

	//--------------------------------------------------------------------------------------------------

	Point Screen::GetGameWindowSize ()
	{
		return Point (static_cast<float> (m_GameWindowWidth), static_cast<float> (m_GameWindowHeight));
	}

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

	void Screen::AddDebugMessage (const std::string& text, float duration, ALLEGRO_COLOR color)
	{
		DebugMessage msg;
		msg.Message = text;
		msg.MaxDuration = duration;
		msg.Color = color;

		m_DebugMessages.push_back (msg);
	}

	//--------------------------------------------------------------------------------------------------

	Point Screen::GetBackBufferSize () { return m_BackBufferSize; }

	//--------------------------------------------------------------------------------------------------

	Point Screen::GetBackBufferOffset () { return m_BackBufferOffset; }

	//--------------------------------------------------------------------------------------------------

	void Screen::OnResize ()
	{
		GetRealWindowSize ();

		if (m_DrawFilled)
		{
			InvalidateBackBuffer (m_RealSize.Width, m_RealSize.Height);
		}

		al_acknowledge_resize (m_Display);
	}

	//--------------------------------------------------------------------------------------------------

	void Screen::SetDrawFilled (bool filled)
	{
		m_DrawFilled = filled;

		if (!m_DrawFilled)
		{
			m_Width = m_GameWindowWidth;
			m_Height = m_GameWindowHeight;

			InvalidateBackBuffer (m_Width, m_Height);
		}

		OnResize ();
	}

	//--------------------------------------------------------------------------------------------------

	void Screen::InvalidateBackBuffer (int newWidth, int newHeight)
	{
		if (m_BackBuffer)
		{
			al_destroy_bitmap (m_BackBuffer);
		}

		m_BackBuffer = al_create_bitmap (newWidth, newHeight);
		m_BackBufferSize = Point (al_get_bitmap_width (m_BackBuffer), al_get_bitmap_height (m_BackBuffer));
	}

	//--------------------------------------------------------------------------------------------------
}
