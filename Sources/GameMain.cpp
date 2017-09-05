// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "Common.h"
#include "MainLoop.h"
#include "Resources.h"
#include "Screen.h"

using namespace aga;

// The attributes of the screen
const int SCREEN_WIDTH = 1000;
const int SCREEN_HEIGHT = 600;

int main (int argc, char* argv[])
{
    Screen mainScreen (SCREEN_WIDTH, SCREEN_HEIGHT);
    MainLoop mainLoop (&mainScreen);

    if (!mainScreen.Initialize ())
    {
        printf ("Failed to initialize!\n");
    }

    mainLoop.Initialize ();
    mainLoop.Start ();

    return 0;
}
