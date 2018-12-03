// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __MAIN_MENU_STATE_H__
#define __MAIN_MENU_STATE_H__

#include "State.h"

struct ALLEGRO_BITMAP;
typedef struct nine_patch_bitmap_tag NINE_PATCH_BITMAP;

namespace aga
{
    const std::string MAIN_MENU_STATE_NAME = "MAIN_MENU_STATE";
    const int MENU_ITEM_NEW_JOURNEY = 0;
    const int MENU_ITEM_CONTINUE = 1;
    const int MENU_ITEM_EXIT = 2;
    const int MENU_ITEM_EXIT_YES = 3;
    const int MENU_ITEM_EXIT_NO = 4;

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

        bool ProcessEvent (ALLEGRO_EVENT* event, float deltaTime);
        void Update (float deltaTime);
        void Render (float deltaTime);

    private:
        void HandleSelection ();

        void RenderMenuItems ();
        void RenderExitItems ();

    private:
        ALLEGRO_BITMAP* m_Image;
        AudioSample* m_SelectSample;
        class Atlas* m_SelectItemAtlas;
        float m_SelectionAngle;
        int m_Selection;

        int m_AnimationStage;
        float m_AnimationTimer;
        float m_SelectionTimer;
        bool m_AnimationUp;

        bool m_ExitSelected;
        bool m_Closing;
    };
}

#endif //   __MAIN_MENU_STATE_H__
