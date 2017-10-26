// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "Animation.h"
#include "Collidable.h"
#include "Common.h"
#include "Scriptable.h"

#define PLAYER_Z_ORDER 10

namespace aga
{
    class SceneManager;
    class Scene;

    class Player : public Lifecycle, public Scriptable, public Collidable
    {
    public:
        Player (SceneManager* sceneManager);
        virtual ~Player ();
        bool Initialize ();
        bool Destroy ();

        void CreatePhysics (Scene* currentScene);
        void DestroyPhysics (Scene* currentScene);

        bool Update (float deltaTime);
        void HandleInput (float deltaTime);
        void ProcessEvent (ALLEGRO_EVENT* event, float deltaTime);
        void Render (float deltaTime);

        void Move (double dx, double dy);
        void SetPosition (const Point& pos);
        void SetPosition (double x, double y);
        Point GetPosition ();
        Point GetSize ();

        std::function<void(double dx, double dy)> MoveCallback;

    private:
        void InitializeAnimations ();

    private:
        ALLEGRO_BITMAP* m_Image;
        Point m_Position, m_OldPosition;
        Point m_Size;
        SceneManager* m_SceneManager;
        Animation m_Animation;
    };
}

#endif //   __PLAYER_H__
