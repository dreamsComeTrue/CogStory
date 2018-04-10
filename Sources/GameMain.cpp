// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "Common.h"
#include "MainLoop.h"
#include "Resources.h"
#include "Screen.h"

using namespace aga;

// The attributes of the screen
const int SCREEN_WIDTH = 1000;
const int SCREEN_HEIGHT = 600;

//--------------------------------------------------------------------------------------------------

int main (int argc, char** argv)
{
    Screen mainScreen (SCREEN_WIDTH, SCREEN_HEIGHT, false);

    if (mainScreen.Initialize ())
    {
        //        ALLEGRO_BITMAP* img = al_load_bitmap
        //        ("C:/Users/squall/GoogleDrive/RobotTales/Data/gfx/ui/menu_ui.png"); ALLEGRO_BITMAP* bmp =
        //        al_create_sub_bitmap (img, 2, 2, 20, 16);

        //        al_draw_bitmap (bmp, 0, 0, 0);
        //        NINE_PATCH_BITMAP* m_FrameBitmap = create_nine_patch_bitmap (bmp, true);

        //        while (true)
        //        {
        //            if (m_FrameBitmap)
        //                draw_nine_patch_bitmap (m_FrameBitmap, 100, 100, 700, 700);
        //            al_flip_display ();
        //        }

        MainLoop mainLoop (&mainScreen);

        mainLoop.Initialize ();
        mainLoop.Start ();
        mainLoop.Destroy ();
    }

    return 0;
}

//--------------------------------------------------------------------------------------------------
