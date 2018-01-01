// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __ACTOR_H__
#define __ACTOR_H__

#include "Animation.h"
#include "Collidable.h"
#include "Common.h"
#include "Entity.h"
#include "Scriptable.h"

namespace aga
{
    const std::string ANIM_IDLE_NAME = "ANIM_IDLE";
    const std::string ANIM_MOVE_UP_NAME = "ANIM_MOVE_UP";
    const std::string ANIM_MOVE_LEFT_NAME = "ANIM_MOVE_LEFT";
    const std::string ANIM_MOVE_RIGHT_NAME = "ANIM_MOVE_RIGHT";

    //  Pixels Per Second
    const float MOVE_SPEED = 110.0;

    class SceneManager;
    class Scene;

    class Actor : public Entity, public Lifecycle, public Scriptable, public Collidable
    {
    public:
        Actor (SceneManager* sceneManager);
        virtual ~Actor ();
        virtual bool Initialize ();
        bool Destroy ();

        virtual void BeforeEnter ();
        virtual void AfterLeave ();

        virtual bool Update (float deltaTime);
        void Render (float deltaTime);

        virtual void Move (float dx, float dy);
        virtual void SetPosition (float x, float y);
        void SetPosition (const Point& pos);
        Point GetPosition ();
        Point GetSize ();

        void SetCurrentAnimation (const std::string& name);

        std::function<void(float dx, float dy)> MoveCallback;

    protected:
        void ChooseAnimation (float angleDeg);

    protected:
        ALLEGRO_BITMAP* m_Image;
        Animation m_Animation;

        Point m_OldPosition;
        SceneManager* m_SceneManager;
    };
}

#endif //   __ACTOR_H__
