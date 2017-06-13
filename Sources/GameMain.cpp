// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "Common.h"
#include "MainLoop.h"
#include "Resources.h"
#include "Screen.h"

using namespace aga;

// The attributes of the screen
const int SCREEN_WIDTH = 1024;
const int SCREEN_HEIGHT = 768;

MainLoop* g_MainLoop;
Screen* g_Screen;

int main (int argc, char* argv[])
{
    g_Screen = new Screen (SCREEN_WIDTH, SCREEN_HEIGHT);
    g_MainLoop = new MainLoop (g_Screen);

    if (!g_Screen->Initialize ())
    {
        printf ("Failed to initialize!\n");
    }

    g_MainLoop->Initialize ();
    g_MainLoop->Start ();

    SAFE_DELETE (g_MainLoop);
    SAFE_DELETE (g_Screen);

    return 0;
}
