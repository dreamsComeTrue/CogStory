// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __MAIN_LOOP_H__
#define __MAIN_LOOP_H__

#include "Lifecycle.h"
#include "StateManager.h"

namespace aga
{
    class Screen;
    class MainMenuState;
    class GamePlayState;
    class EditorState;

    class MainLoop : public Lifecycle
    {
    public:
        MainLoop (Screen* screen);
        virtual ~MainLoop ();
        bool Initialize ();
        bool Destroy ();

        void Start ();

    private:
        void InitializeStates ();
        void DestroyStates ();

    private:
        Screen* m_Screen;
        StateManager m_StateManager;

        MainMenuState* m_MainMenuState;
        GamePlayState* m_GamePlayState;
        EditorState* m_EditorState;
    };
}

#endif //   __MAIN_LOOP_H__
