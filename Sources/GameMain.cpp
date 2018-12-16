// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "MainLoop.h"
#include "Screen.h"

#ifdef _WIN32
    #include <windows.h>
#endif

using namespace aga;

//--------------------------------------------------------------------------------------------------

int main (int /*argc*/, char** /*argv*/)
{
    // The attributes of the screen
    const char* GAME_TITLE = "LITTLE COG STORY";
    const int SCREEN_WIDTH = 1200;
    const int SCREEN_HEIGHT = 700;

    Screen mainScreen (SCREEN_WIDTH, SCREEN_HEIGHT, GAME_TITLE, true);

    if (mainScreen.Initialize ())
    {
        #ifdef _WIN32
            FreeConsole();
        #endif

        MainLoop mainLoop (&mainScreen);

        mainLoop.Initialize ();
        mainLoop.Start ();
        mainLoop.Destroy ();
    }

    return 0;
}

//--------------------------------------------------------------------------------------------------
