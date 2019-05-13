// Copyright 2017-2019 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "GamePlayState.h"
#include "AudioStream.h"
#include "EditorState.h"
#include "MainLoop.h"
#include "MainMenuState.h"
#include "Player.h"
#include "Scene.h"
#include "Screen.h"
#include "Script.h"
#include "SpeechFrameManager.h"

namespace aga
{
	//--------------------------------------------------------------------------------------------------

	const float GAME_WINDOW_SCALE = 2.0f;

	//--------------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------------

	GamePlayState::GamePlayState (MainLoop* mainLoop)
		: State (mainLoop, GAMEPLAY_STATE_NAME)
		, m_AudioWasEnabled (false)
		, m_MasterScript (nullptr)
	{
	}

	//--------------------------------------------------------------------------------------------------

	GamePlayState::~GamePlayState ()
	{
		if (!IsDestroyed ())
		{
			Destroy ();
		}
	}

	//--------------------------------------------------------------------------------------------------

	bool GamePlayState::Initialize ()
	{
		Lifecycle::Initialize ();

		m_MasterScript = m_MainLoop->GetScriptManager ().LoadScriptFromFile ("Master.script", "master");

		return true;
	}

	//--------------------------------------------------------------------------------------------------

	bool GamePlayState::Destroy () { return Lifecycle::Destroy (); }

	//--------------------------------------------------------------------------------------------------

	void GamePlayState::BeforeEnter ()
	{
		Screen* screen = m_MainLoop->GetScreen ();
		SceneManager& sceneManager = m_MainLoop->GetSceneManager ();
		State* prevState = m_MainLoop->GetStateManager ().GetPreviousState ();

		screen->HideMouseCursor ();

		if (prevState != nullptr && prevState->GetName () == EDITOR_STATE_NAME)
		{
			screen->SetWindowSize (screen->GetGameWindowSize ());
			screen->CenterOnScreen ();
		}

		sceneManager.BeforeEnter ();

		ResizeWindow ();

		m_MasterScript->Run ("void Start ()");
	}

	//--------------------------------------------------------------------------------------------------

	void GamePlayState::AfterLeave () { m_MainLoop->GetSceneManager ().AfterLeave (); }

	//--------------------------------------------------------------------------------------------------

	bool GamePlayState::ProcessEvent (ALLEGRO_EVENT* event, float deltaTime)
	{
		if (event->type == ALLEGRO_EVENT_KEY_UP)
		{
			AudioManager& audioManager = m_MainLoop->GetAudioManager ();

			if (event->keyboard.keycode == ALLEGRO_KEY_M)
			{
				audioManager.SetEnabled (!audioManager.IsEnabled ());
				m_AudioWasEnabled = audioManager.IsEnabled ();
			}

			if (audioManager.IsEnabled ())
			{
				AudioStream* audioStream = m_MainLoop->GetSceneManager ().GetActiveScene ()->GetSceneAudioStream ();

				if (audioStream)
				{
					audioStream->Play ();
				}
			}
		}

		if (event->type == ALLEGRO_EVENT_DISPLAY_RESIZE)
		{
			ResizeWindow ();
		}

		return m_MainLoop->GetSceneManager ().GetPlayer ()->ProcessEvent (event, deltaTime);
	}

	//--------------------------------------------------------------------------------------------------

	void GamePlayState::ResizeWindow ()
	{
		//  Initial scale in gameplay twice big as normal
		const Point winSize = m_MainLoop->GetScreen ()->GetBackBufferSize ();

		float finalScale = (winSize.Width * GAME_WINDOW_SCALE) / m_MainLoop->GetScreen ()->GetGameWindowSize ().Width;

		Camera& camera = m_MainLoop->GetSceneManager ().GetCamera ();
		Point translate = camera.GetTranslate ();

		camera.ClearTransformations ();
		camera.Scale (finalScale, finalScale, winSize.Width * 0.5f, winSize.Height * 0.5f);
		camera.SetTranslate (translate);

		if (camera.GetFollowActor ())
		{
			camera.GetFollowActor ()->MoveCallback (0, 0);
		}
	}

	//--------------------------------------------------------------------------------------------------

	void GamePlayState::Update (float deltaTime)
	{
		SceneManager& sceneManager = m_MainLoop->GetSceneManager ();

		if (!sceneManager.IsTransitioning () && !sceneManager.GetPlayer ()->IsPreventInput ())
		{
			sceneManager.GetPlayer ()->HandleInput (deltaTime);
		}
	}

	//--------------------------------------------------------------------------------------------------

	void GamePlayState::Render (float deltaTime) { m_MainLoop->GetSceneManager ().Render (deltaTime); }

	//--------------------------------------------------------------------------------------------------
}
