// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "Animation.h"
#include "Common.h"

#include "addons/tweeny/tweeny.h"

union ALLEGRO_EVENT;
struct ALLEGRO_BITMAP;

namespace aga
{
    class Screen;

    class Player : public Lifecycle
    {
    public:
        Player (Screen* screen);
        virtual ~Player ();
        bool Initialize ();
        bool Destroy ();

        bool Update (double deltaTime);
        void ProcessEvent (ALLEGRO_EVENT* event, double deltaTime);
        void Render (double deltaTime);

    private:
        void InitializeAnimations ();
        void HandleInput (double deltaTime);
        void Move (double dx, double dy);

    private:
        ALLEGRO_BITMAP* m_Image;
        Point m_Position;
        Point m_Size;
        Screen* m_Screen;
        Animation m_Animation;
        tweeny::tween<int> tween;
    };
}

#endif //   __PLAYER_H__
