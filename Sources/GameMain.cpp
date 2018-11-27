// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "MainLoop.h"
#include "Screen.h"

using namespace aga;

// The attributes of the screen
const char* GAME_TITLE = "COG STORY";
const int SCREEN_WIDTH = 1200;
const int SCREEN_HEIGHT = 700;

//--------------------------------------------------------------------------------------------------

int main (int /*argc*/, char** /*argv*/)
{
    Screen mainScreen (SCREEN_WIDTH, SCREEN_HEIGHT, GAME_TITLE, true);

    if (mainScreen.Initialize ())
    {
        MainLoop mainLoop (&mainScreen);

        mainLoop.Initialize ();
        mainLoop.Start ();
        mainLoop.Destroy ();
    }

    return 0;
}

//--------------------------------------------------------------------------------------------------
