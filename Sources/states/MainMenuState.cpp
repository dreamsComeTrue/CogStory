// Copyright 2017-2019 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "MainMenuState.h"
#include "Atlas.h"
#include "AudioSample.h"
#include "AudioStream.h"
#include "GamePlayState.h"
#include "MainLoop.h"
#include "Resources.h"
#include "Screen.h"

namespace aga
{
	//--------------------------------------------------------------------------------------------------

	const float MENU_SELECTION_ROTATION_SPEED = 0.5;
	const float MENU_ANIMATION_TIME = 500.f;

	std::string quitQuestions[] = {"REALLY?", "SURE?", "WANT TO LEAVE?"};

	//--------------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------------

	MainMenuState::MainMenuState (MainLoop* mainLoop)
		: State (mainLoop, MAIN_MENU_STATE_NAME)
		, m_BackgroundImage (nullptr)
		, m_TitleImage (nullptr)
		, m_SelectionAngle (0.0f)
		, m_Selection (0)
		, m_AnimationStage (0)
		, m_AnimationTimer (0.f)
		, m_SelectionTimer (0.f)
		, m_AnimationUp (true)
		, m_ExitSelected (false)
		, m_Closing (false)
	{
	}

	//--------------------------------------------------------------------------------------------------

	MainMenuState::~MainMenuState ()
	{
		if (!IsDestroyed ())
		{
			Destroy ();
		}
	}

	//--------------------------------------------------------------------------------------------------

	bool MainMenuState::Initialize ()
	{
		Lifecycle::Initialize ();

		m_SelectSample = m_MainLoop->GetSceneManager ().GetMainLoop ()->GetAudioManager ().LoadSampleFromFile (
			"SELECT_MENU", GetResource (SOUND_SPEECH_SELECT).Dir + GetResource (SOUND_SPEECH_SELECT).Name);

		m_BackgroundStream = m_MainLoop->GetSceneManager ().GetMainLoop ()->GetAudioManager ().LoadStreamFromFile (
			"MUSIC_MENU_BACKGROUND",
			GetResource (MUSIC_MENU_BACKGROUND).Dir + GetResource (MUSIC_MENU_BACKGROUND).Name);

		m_SelectItemAtlas = m_MainLoop->GetAtlasManager ().GetAtlas ("menu_ui");

		m_BackgroundImage = al_load_bitmap ((GetDataPath () + "/gfx/ui/night-city3.jpg").c_str ());
		m_TitleImage = al_load_bitmap ((GetDataPath () + "/gfx/ui/main_menu.png").c_str ());

		m_AnimationStage = 0;
		m_AnimationTimer = 0.f;

		return true;
	}

	//--------------------------------------------------------------------------------------------------

	bool MainMenuState::Destroy ()
	{
		al_destroy_bitmap (m_BackgroundImage);
		al_destroy_bitmap (m_TitleImage);

		return Lifecycle::Destroy ();
	}

	//--------------------------------------------------------------------------------------------------

	void MainMenuState::BeforeEnter ()
	{
		Screen* screen = m_MainLoop->GetScreen ();

		al_hide_mouse_cursor (screen->GetDisplay ());
		screen->SetBackgroundColor (COLOR_BLACK);

		m_BackgroundStream->SetFadeIn (400);

		m_ExitSelected = false;
		m_Closing = false;

		m_AnimationStage = 0;
		m_AnimationTimer = 0.f;
		m_SelectionTimer = 0.f;

		m_QuitQuestion = RandInRange (0, ARRAY_SIZE (quitQuestions));
	}

	//--------------------------------------------------------------------------------------------------

	void MainMenuState::AfterLeave () { m_BackgroundStream->SetFadeOut (400, true); }

	//--------------------------------------------------------------------------------------------------

	bool MainMenuState::ProcessEvent (ALLEGRO_EVENT* event, float)
	{
		if (event->type == ALLEGRO_EVENT_JOYSTICK_BUTTON_UP)
		{
			if (event->joystick.button == 1)
			{
				m_AnimationTimer = 0.f;
				m_Closing = true;
			}
			else if (event->joystick.button == 2)
			{
				HandleSelection ();
			}
		}

		if (event->type == ALLEGRO_EVENT_JOYSTICK_AXIS)
		{
			if (event->joystick.pos < -0.7f)
			{
				HandleMoveUp ();
			}
			else if (event->joystick.pos > 0.7f)
			{
				HandleMoveDown ();
			}
		}

		if (event->type == ALLEGRO_EVENT_KEY_CHAR)
		{
			switch (event->keyboard.keycode)
			{
			case ALLEGRO_KEY_ESCAPE:
				m_AnimationTimer = 0.f;
				m_Closing = true;
				break;
			}
		}

		if (event->type == ALLEGRO_EVENT_KEY_DOWN)
		{
			switch (event->keyboard.keycode)
			{
			case ALLEGRO_KEY_UP:
			case ALLEGRO_KEY_W:
			{
				HandleMoveUp ();

				break;
			}

			case ALLEGRO_KEY_DOWN:
			case ALLEGRO_KEY_S:
			{
				HandleMoveDown ();

				break;
			}

			case ALLEGRO_KEY_ENTER:
			case ALLEGRO_KEY_X:
			case ALLEGRO_KEY_SPACE:
			{
				HandleSelection ();

				break;
			}
			}
		}

		return true;
	}

	//--------------------------------------------------------------------------------------------------

	void MainMenuState::HandleMoveUp ()
	{
		m_SelectSample->Play ();

		--m_Selection;

		if (m_ExitSelected)
		{
			if (m_Selection < MENU_ITEM_EXIT_YES)
			{
				m_Selection = MENU_ITEM_EXIT_NO;
			}
		}
		else
		{
			if (m_Selection < MENU_ITEM_NEW_JOURNEY)
			{
				m_Selection = MENU_ITEM_EXIT;
			}
		}
	}

	//--------------------------------------------------------------------------------------------------

	void MainMenuState::HandleMoveDown ()
	{
		m_SelectSample->Play ();

		++m_Selection;

		if (m_ExitSelected)
		{
			if (m_Selection > MENU_ITEM_EXIT_NO)
			{
				m_Selection = MENU_ITEM_EXIT_YES;
			}
		}
		else
		{
			if (m_Selection > MENU_ITEM_EXIT)
			{
				m_Selection = MENU_ITEM_NEW_JOURNEY;
			}
		}
	}

	//--------------------------------------------------------------------------------------------------

	void MainMenuState::HandleSelection ()
	{
		m_SelectSample->Play ();

		switch (m_Selection)
		{
		case MENU_ITEM_NEW_JOURNEY:
		{
			m_AnimationTimer = 0.f;
			m_Closing = true;
			m_MainLoop->GetStateManager ().StateFadeInOut (GAMEPLAY_STATE_NAME);
			break;
		}

		case MENU_ITEM_CONTINUE:
		{
			m_AnimationTimer = 0.f;
			m_Closing = true;
			m_MainLoop->GetStateManager ().StateFadeInOut (GAMEPLAY_STATE_NAME);
			break;
		}

		case MENU_ITEM_EXIT:
		{
			m_ExitSelected = true;
			m_Selection = MENU_ITEM_EXIT_YES;
			break;
		}

		case MENU_ITEM_EXIT_YES:
		{
			m_MainLoop->Exit ();
			break;
		}

		case MENU_ITEM_EXIT_NO:
		{
			m_ExitSelected = false;
			m_Selection = MENU_ITEM_EXIT;
			break;
		}
		}
	}

	//--------------------------------------------------------------------------------------------------

	void MainMenuState::Update (float) {}

	//--------------------------------------------------------------------------------------------------

	void MainMenuState::Render (float deltaTime)
	{
		const Point winSize = m_MainLoop->GetScreen ()->GetBackBufferSize ();
		Font& font = m_MainLoop->GetScreen ()->GetFont ();

		if (!m_Closing)
		{
			if (m_AnimationStage < 2)
			{
				m_AnimationTimer += deltaTime * 1000.f;

				if (m_AnimationTimer > MENU_ANIMATION_TIME)
				{
					++m_AnimationStage;
					m_AnimationTimer = 0.f;
				}
			}
		}
		else
		{
			m_AnimationTimer += deltaTime * 1000.f;

			if (m_AnimationTimer > MENU_ANIMATION_TIME)
			{
				++m_AnimationStage;
				m_AnimationTimer = 0.f;
			}
		}

		float percent = m_AnimationTimer / MENU_ANIMATION_TIME;
		float currentPercent;

		if (!m_Closing)
		{
			currentPercent = m_AnimationStage > 0 ? 1.0f : clamp (percent, 0.f, 1.f);
		}
		else
		{
			currentPercent = 1.0f - clamp (percent, 0.f, 1.f);
		}

		float imgHeight = al_get_bitmap_height (m_TitleImage);
		float targetYPos = winSize.Height * 0.5f - imgHeight * 0.5f;
		float yPos = -imgHeight * 0.5f + std::abs (-imgHeight * 0.5f - targetYPos) * currentPercent;

		al_draw_scaled_bitmap (m_BackgroundImage, 0, 0, al_get_bitmap_width (m_BackgroundImage),
			al_get_bitmap_height (m_BackgroundImage), 0, 0, winSize.Width, winSize.Height, 0);
		al_draw_bitmap (m_TitleImage, winSize.Width * 0.5f - al_get_bitmap_width (m_TitleImage) * 0.5f, yPos, 0);

		if (m_ExitSelected)
		{
			RenderExitItems ();
		}
		else
		{
			RenderMenuItems ();
		}

		//  Draw center cog
		float scale = 1.3f * currentPercent;

		m_SelectionAngle += MENU_SELECTION_ROTATION_SPEED * deltaTime;
		m_SelectItemAtlas->DrawRegion ("cog", winSize.Width * 0.5f, winSize.Height * 0.5f - 70, scale, scale,
			m_SelectionAngle, false, COLOR_WHITE);

		if (m_AnimationUp)
		{
			m_SelectionTimer += deltaTime * 1000.f;

			if (m_SelectionTimer > MENU_ANIMATION_TIME)
			{
				m_AnimationUp = false;
			}
		}
		else
		{
			m_SelectionTimer -= deltaTime * 1000.f;

			if (m_SelectionTimer <= 0.f)
			{
				m_SelectionTimer = 0.f;
				m_AnimationUp = true;
			}
		}

		font.DrawText (FONT_NAME_SMALL, "v 0.1 (C) Agado Studio", al_map_rgb (130, 130, 130), 5, winSize.Height - 20,
			1.0f, ALLEGRO_ALIGN_LEFT);
	}

	//--------------------------------------------------------------------------------------------------

#define MENU_ITEM_COLOR al_map_rgb (200, 200, 200)

	void MainMenuState::RenderMenuItems ()
	{
		const Point winSize = m_MainLoop->GetScreen ()->GetBackBufferSize ();
		std::string menuItems[3] = {"NEW JOURNEY", "CONTINUE", "EXIT"};

		Font& font = m_MainLoop->GetScreen ()->GetFont ();
		Point textSize = font.GetTextDimensions (FONT_NAME_MENU_ITEM_NORMAL, menuItems[m_Selection]);

		int offset = 10;
		int menuItemSpacing = static_cast<int> (textSize.Height + offset);
		int menuItemStartY = static_cast<int> (winSize.Height * 0.6f + menuItemSpacing);

		float percent = m_AnimationTimer / MENU_ANIMATION_TIME;
		float currentPercent;

		if (!m_Closing)
		{
			currentPercent = m_AnimationStage > 0 ? 1.0f : clamp (percent, 0.f, 1.f);
		}
		else
		{
			currentPercent = 1.0f - clamp (percent, 0.f, 1.f);
		}

		for (int i = 0; i < 3; ++i)
		{
			float scale = i == m_Selection ? 1.0f + (m_SelectionTimer / MENU_ANIMATION_TIME) * 0.2f : 1.0f;
			float targetXPos = winSize.Width * 0.5f + 250.f;
			float xPos = winSize.Width - (winSize.Width - targetXPos) * currentPercent;

			font.DrawText (FONT_NAME_MENU_ITEM_SMALL, menuItems[i], MENU_ITEM_COLOR, xPos,
				m_Selection == i ? menuItemStartY + i * menuItemSpacing - offset * 0.5f
								 : menuItemStartY + i * menuItemSpacing,
				scale, ALLEGRO_ALIGN_CENTER);
		}
	}

	//--------------------------------------------------------------------------------------------------

	void MainMenuState::RenderExitItems ()
	{
		const Point winSize = m_MainLoop->GetScreen ()->GetBackBufferSize ();

		std::string menuItems[3] = {quitQuestions[m_QuitQuestion], "YES", "NO"};

		Font& font = m_MainLoop->GetScreen ()->GetFont ();
		Point textSize = font.GetTextDimensions (FONT_NAME_MENU_ITEM_NORMAL, menuItems[m_Selection - MENU_ITEM_EXIT]);

		int offset = 10;
		int menuItemSpacing = static_cast<int> (textSize.Height + offset);
		int menuItemStartY = static_cast<int> (winSize.Height * 0.6f + menuItemSpacing);

		for (int i = 0; i < 3; ++i)
		{
			float currentPercent = m_SelectionTimer / MENU_ANIMATION_TIME;
			float scale = i == (m_Selection - MENU_ITEM_EXIT) ? 1.0f + currentPercent * 0.2f : 1.0f;

			font.DrawText (FONT_NAME_MENU_ITEM_SMALL, menuItems[i], MENU_ITEM_COLOR, winSize.Width * 0.5f + 250.f,
				i == (m_Selection - MENU_ITEM_EXIT) ? menuItemStartY + i * menuItemSpacing - offset * 0.5f
													: menuItemStartY + i * menuItemSpacing,
				scale, ALLEGRO_ALIGN_CENTER);
		}
	}

	//--------------------------------------------------------------------------------------------------
}
