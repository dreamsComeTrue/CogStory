// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __MAIN_MENU_STATE_H__
#define __MAIN_MENU_STATE_H__

#include "State.h"

struct ALLEGRO_BITMAP;
typedef struct nine_patch_bitmap_tag NINE_PATCH_BITMAP;

namespace aga
{
    extern std::string MAIN_MENU_STATE_NAME;

    class StateManager;

    class MainMenuState : public State
    {
    public:
        MainMenuState (MainLoop* mainLoop);

        virtual ~MainMenuState ();
        bool Initialize ();
        bool Destroy ();

        void BeforeEnter ();
        void AfterLeave ();

        void ProcessEvent (ALLEGRO_EVENT* event, float deltaTime);
        void Update (float deltaTime);
        void Render (float deltaTime);

    private:
        NINE_PATCH_BITMAP* m_Background;
    };
}

#endif //   __MAIN_MENU_STATE_H__
