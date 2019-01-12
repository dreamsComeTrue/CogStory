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
    const Point SCREEN_SIZE = {1200, 700};
    const Point GAME_WINDOW_SIZE = {800, 600};

    Screen mainScreen (SCREEN_SIZE.Width, SCREEN_SIZE.Height, GAME_WINDOW_SIZE.Width, GAME_WINDOW_SIZE.Height, GAME_TITLE, true);

    if (mainScreen.Initialize ())
    {
#ifdef _WIN32
        FreeConsole ();
#endif

        MainLoop mainLoop (&mainScreen);

        mainLoop.Initialize ();
        mainLoop.Start ();
        mainLoop.Destroy ();
    }

    return 0;
}

//--------------------------------------------------------------------------------------------------
