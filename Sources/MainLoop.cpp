// Copyright 2017-2019 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "MainLoop.h"
#include "Screen.h"

#include "states/EditorState.h"
#include "states/GamePlayState.h"
#include "states/MainMenuState.h"

using json = nlohmann::json;

namespace aga
{
	//--------------------------------------------------------------------------------------------------

	std::string gameConfigFileName = "game_settings.cog";

	//--------------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------------

	MainLoop::MainLoop (Screen* screen)
		: m_Screen (screen)
		, m_AudioManager (this)
		, m_StateManager (this)
		, m_SceneManager (this)
		, m_ScriptManager (this)
		, m_PhysicsManager (this)
		, m_TweenManager (this)
		, m_MainMenuState (nullptr)
		, m_GamePlayState (nullptr)
		, m_EditorState (nullptr)
		, m_IsRunning (true)
	{
	}

	//--------------------------------------------------------------------------------------------------

	MainLoop::~MainLoop ()
	{
		if (!IsDestroyed ())
		{
			Destroy ();
		}
	}

	//--------------------------------------------------------------------------------------------------

	bool MainLoop::Initialize ()
	{
		Lifecycle::Initialize ();

		m_AtlasManager.Initialize ();
		m_AudioManager.Initialize ();
		m_SceneManager.Initialize ();
		m_PhysicsManager.Initialize ();
		m_StateManager.Initialize ();
		m_TweenManager.Initialize ();
		m_ScriptManager.Initialize ();

		m_AudioManager.SetMasterVolume (1.0f);

		m_Screen->ProcessEventFunction = [&](ALLEGRO_EVENT* event) {
			bool ret = m_StateManager.ProcessEvent (event, m_Screen->GetDeltaTime ());

			if (!ret)
			{
				m_SceneManager.ProcessEvent (event, m_Screen->GetDeltaTime ());
			}
		};

		m_Screen->RenderFunction = [&]() { m_StateManager.Render (m_Screen->GetDeltaTime ()); };
		m_Screen->UpdateFunction = [&](float deltaTime) {
			if (m_StateManager.GetActiveStateName () != EDITOR_STATE_NAME)
			{
				m_TweenManager.Update (deltaTime);
			}

			m_AudioManager.Update (deltaTime);
			m_ScriptManager.Update (deltaTime);
			m_SceneManager.Update (deltaTime);
			m_StateManager.Update (deltaTime);
		};

		InitializeStates ();
#ifdef EDITOR_ENABLED
		m_StateManager.SetActiveState (m_EditorState);
#else
		m_Screen->SetWindowSize (m_Screen->GetGameWindowSize ());
		m_Screen->CenterOnScreen ();
		m_Screen->SetDrawFilled (true);
		m_StateManager.SetActiveState (m_MainMenuState);
#endif

		return true;
	}

	//--------------------------------------------------------------------------------------------------

	bool MainLoop::Destroy ()
	{
		DestroyStates ();

		m_AtlasManager.Destroy ();
		m_PhysicsManager.Destroy ();
		m_TweenManager.Destroy ();
		m_StateManager.Destroy ();
		m_SceneManager.Destroy ();
		m_AudioManager.Destroy ();
		m_ScriptManager.Destroy ();

		return Lifecycle::Destroy ();
	}

	//--------------------------------------------------------------------------------------------------

	void MainLoop::InitializeStates ()
	{
		m_MainMenuState = new MainMenuState (this);
		m_MainMenuState->Initialize ();
		m_StateManager.RegisterState (m_MainMenuState);

		m_GamePlayState = new GamePlayState (this);
		m_GamePlayState->Initialize ();
		m_StateManager.RegisterState (m_GamePlayState);

		m_EditorState = new EditorState (this);
		m_EditorState->Initialize ();
		m_StateManager.RegisterState (m_EditorState);
	}

	//--------------------------------------------------------------------------------------------------

	void MainLoop::DestroyStates ()
	{
		SAFE_DELETE (m_MainMenuState);
		SAFE_DELETE (m_EditorState);
		SAFE_DELETE (m_GamePlayState);
	}

	//--------------------------------------------------------------------------------------------------

	void MainLoop::Start ()
	{
		double oldTime = al_get_time ();
        
        LoadConfig ();

		while (m_IsRunning)
		{
			double newTime = al_get_time ();
			double deltaTime = (newTime - oldTime);
			oldTime = newTime;

			if (!m_Screen->Update (static_cast<float> (deltaTime)))
			{
				break;
			}
		}
        
        SaveConfig ();
	}

	//--------------------------------------------------------------------------------------------------

	void MainLoop::LoadConfig ()
	{
		try
		{
			std::ifstream file (gameConfigFileName.c_str ());
			json j;
			file >> j;
			file.close ();

			GetAudioManager ().SetStreamsEnabled (j["music_enabled"]);
			GetAudioManager ().SetSamplesEnabled (j["sounds_enabled"]);
		}
		catch (const std::exception&)
		{
		}
	}

	//--------------------------------------------------------------------------------------------------

	void MainLoop::SaveConfig ()
	{
		try
		{
			json j;

			j["music_enabled"] = GetAudioManager ().IsStreamsEnabled ();
			j["sounds_enabled"] = GetAudioManager ().IsSamplesEnabled ();

			// write prettified JSON to another file
			std::ofstream out (gameConfigFileName.c_str ());
			out << std::setw (4) << j.dump (4, ' ') << "\n";
		}
		catch (const std::exception&)
		{
		}
	}

	//--------------------------------------------------------------------------------------------------

	void MainLoop::Exit () { m_IsRunning = false; }

	//--------------------------------------------------------------------------------------------------

	Screen* MainLoop::GetScreen () { return m_Screen; }

	//--------------------------------------------------------------------------------------------------

	AudioManager& MainLoop::GetAudioManager () { return m_AudioManager; }

	//--------------------------------------------------------------------------------------------------

	StateManager& MainLoop::GetStateManager () { return m_StateManager; }

	//--------------------------------------------------------------------------------------------------

	SceneManager& MainLoop::GetSceneManager () { return m_SceneManager; }

	//--------------------------------------------------------------------------------------------------

	ScriptManager& MainLoop::GetScriptManager () { return m_ScriptManager; }

	//--------------------------------------------------------------------------------------------------

	PhysicsManager& MainLoop::GetPhysicsManager () { return m_PhysicsManager; }

	//--------------------------------------------------------------------------------------------------

	TweenManager& MainLoop::GetTweenManager () { return m_TweenManager; }

	//--------------------------------------------------------------------------------------------------

	AtlasManager& MainLoop::GetAtlasManager () { return m_AtlasManager; }

	//--------------------------------------------------------------------------------------------------
}
