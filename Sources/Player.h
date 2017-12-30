// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "Actor.h"

#define PLAYER_Z_ORDER 10

namespace aga
{
    class SceneManager;
    class Scene;

    class Player : public Actor
    {
    public:
        Player (SceneManager* sceneManager);
        virtual ~Player ();
        bool Initialize () override;

        void BeforeEnter () override;

        void SetPreventInput (bool prevent = false);
        bool IsPreventInput () const;
        void HandleInput (float deltaTime);
        void ProcessEvent (ALLEGRO_EVENT* event, float deltaTime);

        bool Update (float deltaTime) override;

        void Move (float dx, float dy) override;
        void SetPosition (float x, float y) override;

        void SetFollowCamera (bool follow);

        static std::string GetTypeName ();

    private:
        void InitializeAnimations ();

    private:
        bool m_FollowCamera;
        bool m_PreventInput;
    };
}

#endif //   __PLAYER_H__
