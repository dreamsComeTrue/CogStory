// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "Animation.h"
#include "Collidable.h"
#include "Common.h"
#include "Entity.h"
#include "Scriptable.h"

#define PLAYER_Z_ORDER 10

namespace aga
{
    class SceneManager;
    class Scene;

    class Player : public Entity, public Lifecycle, public Scriptable, public Collidable
    {
    public:
        Player (SceneManager* sceneManager);
        virtual ~Player ();
        bool Initialize ();
        bool Destroy ();

        void BeforeEnter ();
        void AfterLeave ();

        bool Update (float deltaTime);
        void HandleInput (float deltaTime);
        void ProcessEvent (ALLEGRO_EVENT* event, float deltaTime);
        void Render (float deltaTime);

        void Move (float dx, float dy);
        void SetPosition (const Point& pos);
        void SetPosition (float x, float y);
        Point GetPosition ();
        Point GetSize ();

        void SetFollowCamera (bool follow);

        std::function<void(float dx, float dy)> MoveCallback;

    private:
        void InitializeAnimations ();

    private:
        ALLEGRO_BITMAP* m_Image;
        Point m_OldPosition;
        SceneManager* m_SceneManager;
        Animation m_Animation;
        bool m_FollowCamera;
    };
}

#endif //   __PLAYER_H__
