// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __STATE_MANAGER_H__
#define __STATE_MANAGER_H__

#include "Common.h"

namespace aga
{
    class Screen;
    class State;
    class MainLoop;
    struct TweenData;

    class StateManager : public Lifecycle
    {
    public:
        StateManager (MainLoop* mainLoop);
        virtual ~StateManager ();
        bool Initialize ();
        bool Destroy ();

        void SetActiveState (State* state);
        void SetActiveState (const std::string& name);
        std::string GetActiveStateName ();
        void RegisterState (State* state);

        bool Update (float deltaTime);
        void ProcessEvent (ALLEGRO_EVENT* event, float deltaTime);
        void Render (float deltaTime);

        MainLoop* GetMainLoop ();

        void StateFadeInOut (const std::string& enteringStateName);

    private:
        std::map<std::string, State*> m_States;
        State* m_ActiveState;

        MainLoop* m_MainLoop;

        int m_BlendOp, m_BlendSrc, m_BlendDst;
        TweenData* m_TweenFade;
        bool m_Transitioning;
        ALLEGRO_COLOR m_FadeColor;
        std::function<void(void)> m_TransitionFunc;
        bool m_TransitionFuncFired;
    };
}

#endif //   __STATE_MANAGER_H__
