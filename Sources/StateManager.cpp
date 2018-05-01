// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "StateManager.h"
#include "Common.h"
#include "MainLoop.h"
#include "Screen.h"
#include "State.h"
#include "TweenManager.h"

#include "states/EditorState.h"
#include "states/GamePlayState.h"
#include "states/MainMenuState.h"

#include <algorithm>

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    const int FADE_MAX_TIME = 300;

    //--------------------------------------------------------------------------------------------------

    StateManager::StateManager (MainLoop* mainLoop)
        : m_ActiveState (nullptr)
        , m_PreviousState (nullptr)
        , m_MainLoop (mainLoop)
        , m_Transitioning (false)
        , m_FadeColor (COLOR_BLACK)
        , m_TweenFade (nullptr)
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
        if (m_ActiveState != nullptr)
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

    void StateManager::ProcessEvent (ALLEGRO_EVENT* event, float deltaTime)
    {
        std::string before;
        std::string after;

        if (m_ActiveState != nullptr)
        {
            before = GetActiveStateName ();
            m_ActiveState->ProcessEvent (event, deltaTime);
            after = GetActiveStateName ();
        }

        if (before == after)
        {
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
            const Point size = m_MainLoop->GetScreen ()->GetWindowSize ();
            al_draw_filled_rectangle (0, 0, size.Width, size.Height, m_FadeColor);
        }
    }

    //--------------------------------------------------------------------------------------------------

    MainLoop* StateManager::GetMainLoop () { return m_MainLoop; }

    //--------------------------------------------------------------------------------------------------

    void StateManager::StateFadeInOut (const std::string& enteringStateName)
    {
        std::function<void(void)> transitionFunc = [&]() {
            std::map<std::string, State*>::iterator it = m_States.find (enteringStateName);

            if (it != m_States.end ())
            {
                if (m_ActiveState != nullptr)
                {
                    m_ActiveState->AfterLeave ();
                }

                m_MainLoop->GetSceneManager ().Reset ();

                m_ActiveState = (*it).second;
                m_ActiveState->BeforeEnter ();
            }
        };

        m_FadeColor = al_map_rgb (60, 60, 70);
        m_FadeColor.a = 0.0f;
        m_Transitioning = true;
        m_TransitionFunc = transitionFunc;
        m_TransitionFuncFired = false;

        auto fadeFunc = [&](float v) {
            m_FadeColor.a = v;
            if (m_TweenFade->TweenF.progress () < 1.0f)
            {
            }
            else
            {
                al_set_blender (m_BlendOp, m_BlendSrc, m_BlendDst);
                m_Transitioning = false;
            }

            if (m_TweenFade->TweenF.progress () > 0.5f)
            {
                if (m_TransitionFunc && !m_TransitionFuncFired)
                {
                    m_TransitionFuncFired = true;
                    m_TransitionFunc ();
                }
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

        m_TweenFade = &m_MainLoop->GetTweenManager ().AddTween (9999, tween);

        al_get_blender (&m_BlendOp, &m_BlendSrc, &m_BlendDst);
        al_set_blender (ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA);
    }

    //--------------------------------------------------------------------------------------------------
}
