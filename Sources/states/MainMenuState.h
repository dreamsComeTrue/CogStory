// Copyright 2017-2019 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __MAIN_MENU_STATE_H__
#define __MAIN_MENU_STATE_H__

#include "State.h"

struct ALLEGRO_BITMAP;
typedef struct nine_patch_bitmap_tag NINE_PATCH_BITMAP;

namespace aga
{
    const std::string MAIN_MENU_STATE_NAME = "MAIN_MENU_STATE";
    
    const int MENU_ITEM_GENERAL_NEW_JOURNEY = 0;
    const int MENU_ITEM_GENERAL_CONTINUE = 1;
    const int MENU_ITEM_GENERAL_EXIT = 2;
    
    const int MENU_ITEM_GAME_CONTINUE = 0;
    const int MENU_ITEM_GAME_OPTIONS = 1;
    const int MENU_ITEM_GAME_EXIT = 2;
    
    const int MENU_ITEM_OPTIONS_MUSIC = 0;
    const int MENU_ITEM_OPTIONS_SOUNDS = 1;
    const int MENU_ITEM_OPTIONS_BACK = 2;    
    
    const int MENU_ITEM_EXIT_YES = 0;
    const int MENU_ITEM_EXIT_NO = 1;

    class AudioSample;
    class AudioStream;
    class StateManager;
    
    enum MenuState
    {
        MENU_GENERAL,
        MENU_IN_GAME,
        MENU_OPTIONS,
        MENU_EXIT        
    };

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
        void HandleGeneralMenu ();
        void HandleInGameMenu ();
        void HandleOptionsMenu ();
        void HandleExitMenu ();
        void HandleMoveUp ();
        void HandleMoveDown ();

        void RenderGeneralMenuItems ();
        void RenderInGameMenuItems ();
        void RenderOptionsMenuItems ();
        void RenderExitItems ();
        
        void RenderMenuItems (std::vector<std::string> options, int offset = 0);

    private:
        ALLEGRO_BITMAP* m_BackgroundImage;
        ALLEGRO_BITMAP* m_TitleImage;
        AudioSample* m_SelectSample;
        AudioStream* m_BackgroundStream;
        class Atlas* m_SelectItemAtlas;
        float m_SelectionAngle;
        int m_Selection;
        int m_QuitQuestion;
        
        MenuState m_MenuState;
        
        int m_AnimationStage;
        float m_AnimationTimer;
        float m_SelectionTimer;
        bool m_AnimationUp;

        bool m_Closing;
    };
}

#endif //   __MAIN_MENU_STATE_H__
