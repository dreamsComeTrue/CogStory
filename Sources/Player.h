// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "Animation.h"
#include "Common.h"

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

        bool Update ();
        void ProcessEvent (ALLEGRO_EVENT* event);
        void Render ();

    private:
        void InitializeAnimations ();

    private:
        ALLEGRO_BITMAP* m_Image;
        Point m_Size;
        Screen* m_Screen;
        Animation m_Animation;
    };
}

#endif //   __PLAYER_H__
