// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "Animation.h"
#include "Common.h"
#include "Scriptable.h"

namespace aga
{
    class Screen;

    class Player : public Lifecycle, public Scriptable
    {
    public:
        Player (Screen* screen);
        virtual ~Player ();
        bool Initialize ();
        bool Destroy ();

        bool Update (double deltaTime);
        void HandleInput (double deltaTime);
        void ProcessEvent (SDL_Event* event, double deltaTime);
        void Render (double deltaTime);

        void Move (double dx, double dy);
        void SetPosition (const Point& pos);
        void SetPosition (double x, double y);
        Point GetPosition ();
        Point GetSize ();

        std::function<void(double dx, double dy)> MoveCallback;

    private:
        void InitializeAnimations ();

    private:
        SDL_Texture* m_Image;
        Point m_Position, m_OldPosition;
        Point m_Size;
        Screen* m_Screen;
        Animation m_Animation;
    };
}

#endif //   __PLAYER_H__
