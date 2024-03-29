// Copyright 2017-2019 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "StateManager.h"
#include "MainLoop.h"
#include "Screen.h"
#include "State.h"

#include "states/EditorState.h"
#include "states/GamePlayState.h"
#include "states/MainMenuState.h"

namespace aga
{
	//--------------------------------------------------------------------------------------------------

	const int FADE_MAX_TIME = 200;
	const int STATE_MANAGER_TWEEN_ID = 0;

	//--------------------------------------------------------------------------------------------------

	StateManager::StateManager (MainLoop* mainLoop)
		: m_PreviousState (nullptr)
		, m_ActiveState (nullptr)
		, m_MainLoop (mainLoop)
		, m_TweenFade (nullptr)
		, m_Transitioning (false)
		, m_FadeColor (COLOR_BLACK)
	{
	}

	//--------------------------------------------------------------------------------------------------

	StateManager::~StateManager ()
	{
		if (!IsDestroyed ())
		{
			Destroy ();
		}
	}

	//--------------------------------------------------------------------------------------------------

	bool StateManager::Initialize () { return Lifecycle::Initialize (); }

	//--------------------------------------------------------------------------------------------------

	bool StateManager::Destroy () { return Lifecycle::Destroy (); }

	//--------------------------------------------------------------------------------------------------

	void StateManager::SetActiveState (State* state)
	{
		m_PreviousState = m_ActiveState;

		if (m_ActiveState != nullptr)
		{
			m_ActiveState->AfterLeave ();
		}

		m_MainLoop->GetTweenManager ().Clear ();
		m_MainLoop->GetSceneManager ().Reset ();

		m_ActiveState = state;
		m_ActiveState->BeforeEnter ();
	}

	//--------------------------------------------------------------------------------------------------

	void StateManager::SetActiveState (const std::string& name)
	{
		std::map<std::string, State*>::iterator it = m_States.find (name);

		if (it != m_States.end ())
		{
			SetActiveState ((*it).second);
		}
	}

	//--------------------------------------------------------------------------------------------------

	std::string StateManager::GetActiveStateName ()
	{
		if (m_ActiveState)
		{
			return m_ActiveState->GetName ();
		}

		return "";
	}

	//--------------------------------------------------------------------------------------------------

	void StateManager::RegisterState (State* state)
	{
		std::map<std::string, State*>::iterator it = m_States.find (state->GetName ());

		if (it == m_States.end ())
		{
			m_States.insert (std::make_pair (state->GetName (), state));
		}
	}

	//--------------------------------------------------------------------------------------------------

	bool StateManager::Update (float deltaTime)
	{
		if (m_ActiveState != nullptr)
		{
			m_ActiveState->Update (deltaTime);
		}

		return true;
	}

	//--------------------------------------------------------------------------------------------------

	bool StateManager::ProcessEvent (ALLEGRO_EVENT* event, float deltaTime)
	{
		std::string before;
		std::string after;
		bool ret = false;

		if (m_ActiveState != nullptr)
		{
			before = GetActiveStateName ();
			ret = m_ActiveState->ProcessEvent (event, deltaTime);
			after = GetActiveStateName ();
		}

		if (before == after)
		{
			if (event->type == ALLEGRO_EVENT_JOYSTICK_BUTTON_UP)
			{
				if (event->joystick.button == 9)
				{
					if (GetActiveStateName () == GAMEPLAY_STATE_NAME)
					{
						SetActiveState (EDITOR_STATE_NAME);
					}
					else if (GetActiveStateName () == EDITOR_STATE_NAME)
					{
						SetActiveState (GAMEPLAY_STATE_NAME);
					}
				}

				if (event->joystick.button == 0)
				{
					if (GetActiveStateName () == GAMEPLAY_STATE_NAME)
					{
						StateFadeInOut (MAIN_MENU_STATE_NAME);
					}
					else if (GetActiveStateName () == MAIN_MENU_STATE_NAME)
					{
						StateFadeInOut (GAMEPLAY_STATE_NAME);
					}
				}
			}

			if (event->type == ALLEGRO_EVENT_KEY_UP)
			{
				switch (event->keyboard.keycode)
				{
				case ALLEGRO_KEY_F1:
				{
					if (GetActiveStateName () == GAMEPLAY_STATE_NAME)
					{
						SetActiveState (EDITOR_STATE_NAME);
					}
					else if (GetActiveStateName () == EDITOR_STATE_NAME)
					{
						SetActiveState (GAMEPLAY_STATE_NAME);
					}

					break;
				}

				case ALLEGRO_KEY_ESCAPE:
				{
					if (GetActiveStateName () == GAMEPLAY_STATE_NAME)
					{
						StateFadeInOut (MAIN_MENU_STATE_NAME);
					}
					else if (GetActiveStateName () == MAIN_MENU_STATE_NAME)
					{
						StateFadeInOut (GAMEPLAY_STATE_NAME);
					}

					break;
				}
				}
			}
		}

		return ret;
	}

	//--------------------------------------------------------------------------------------------------

	void StateManager::Render (float deltaTime)
	{
		if (m_ActiveState != nullptr)
		{
			m_ActiveState->Render (deltaTime);
		}

		if (m_Transitioning)
		{
			al_get_blender (&m_BlendOp, &m_BlendSrc, &m_BlendDst);
			al_set_blender (ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA);

			const Point size = m_MainLoop->GetScreen ()->GetBackBufferSize ();
			al_draw_filled_rectangle (0, 0, size.Width, size.Height, m_FadeColor);

			al_set_blender (m_BlendOp, m_BlendSrc, m_BlendDst);
		}
	}

	//--------------------------------------------------------------------------------------------------

	MainLoop* StateManager::GetMainLoop () { return m_MainLoop; }

	//--------------------------------------------------------------------------------------------------

	State* StateManager::GetPreviousState () { return m_PreviousState; }

	//--------------------------------------------------------------------------------------------------

	void StateManager::StateFadeInOut (const std::string& enteringStateName)
	{
		std::function<void(void)> transitionFunc = [&]() {
			std::map<std::string, State*>::iterator it = m_States.find (enteringStateName);

			if (it != m_States.end ())
			{
				m_PreviousState = m_ActiveState;

				if (m_ActiveState != nullptr)
				{
					m_ActiveState->AfterLeave ();
				}

				m_MainLoop->GetSceneManager ().Reset ();

				m_ActiveState = (*it).second;
				m_ActiveState->BeforeEnter ();
			}
		};

		m_FadeColor = COLOR_BLACK;//al_map_rgba (45, 55, 145, 255);
		m_Transitioning = true;
		m_TransitionFunc = transitionFunc;
		m_TransitionFuncFired = false;

		auto fadeFunc = [&](float v) {
			m_FadeColor.a = v;

			if (m_TweenFade->TweenF.progress () >= 0.99f)
			{
				m_Transitioning = false;
			}

			if (m_TransitionFunc && !m_TransitionFuncFired && m_TweenFade->TweenF.progress () >= 0.5f)
			{
				m_TransitionFuncFired = true;
				m_TransitionFunc ();
			}

			return false;
		};

		tweeny::tween<float> tween = tweeny::from (0.0f)
										 .to (1.0f)
										 .during (FADE_MAX_TIME)
										 .onStep (fadeFunc)
										 .to (0.0f)
										 .during (FADE_MAX_TIME)
										 .onStep (fadeFunc);

		m_TweenFade = &m_MainLoop->GetTweenManager ().AddTween (STATE_MANAGER_TWEEN_ID, tween);
	}

	//--------------------------------------------------------------------------------------------------
}
