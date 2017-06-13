// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __MAIN_MENU_STATE_H__
#define __MAIN_MENU_STATE_H__

#include "State.h"

struct ALLEGRO_BITMAP;
typedef struct nine_patch_bitmap_tag NINE_PATCH_BITMAP;

namespace aga
{
    class StateManager;

    class MainMenuState : public State
    {
    public:
        MainMenuState (StateManager* stateManager);

        virtual ~MainMenuState ();
        bool Initialize ();
        bool Destroy ();

        void ProcessEvent (ALLEGRO_EVENT* event);
        void Update ();
        void Render ();

    private:
        StateManager* m_StateManager;
        NINE_PATCH_BITMAP* m_Background;
    };
}

#endif //   __MAIN_MENU_STATE_H__
