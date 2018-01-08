// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __MAIN_MENU_STATE_H__
#define __MAIN_MENU_STATE_H__

#include "State.h"

struct ALLEGRO_BITMAP;
typedef struct nine_patch_bitmap_tag NINE_PATCH_BITMAP;

namespace aga
{
    const std::string MAIN_MENU_STATE_NAME = "MAIN_MENU_STATE";
    const int MENU_ITEM_NEW_STORY = 0;
    const int MENU_ITEM_CONTINUE = 1;
    const int MENU_ITEM_EXIT = 2;

    class AudioSample;
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
        void HandleSelection ();

    private:
        AudioSample* m_SelectSample;
        ALLEGRO_BITMAP* m_SelectImage;
        float m_SelectionAngle;
        int m_Selection;
    };
}

#endif //   __MAIN_MENU_STATE_H__
