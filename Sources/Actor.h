// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __ACTOR_H__
#define __ACTOR_H__

#include "Animable.h"
#include "Collidable.h"
#include "Common.h"
#include "Entity.h"
#include "Scriptable.h"

namespace aga
{
    const std::string ANIM_IDLE_NAME = "ANIM_IDLE";
    const std::string ANIM_MOVE_DOWN_NAME = "ANIM_MOVE_DOWN";
    const std::string ANIM_MOVE_UP_NAME = "ANIM_MOVE_UP";
    const std::string ANIM_MOVE_LEFT_NAME = "ANIM_MOVE_LEFT";
    const std::string ANIM_MOVE_RIGHT_NAME = "ANIM_MOVE_RIGHT";

    //  Pixels Per Second
    const float MOVE_SPEED = 110.0;

    class SceneManager;
    class Scene;
    class Component;

    class Actor : public Entity, public Lifecycle, public Animable, public Scriptable, public Collidable
    {
    public:
        Actor (SceneManager* sceneManager);
        virtual ~Actor ();
        virtual bool Initialize ();
        virtual bool Destroy ();

        virtual void BeforeEnter ();
        virtual void AfterLeave ();

        virtual bool Update (float deltaTime);
        virtual void Render (float deltaTime);

        virtual void Move (float dx, float dy);
        virtual void Move (Point deltaPos);
        virtual void SetPosition (float x, float y);
        virtual void SetPosition (Point pos);
        Point GetPosition ();
        Point GetSize ();

        void AddComponent (const std::string& name, Component* component);
        void RemoveComponent (const std::string& name);
        void RemoveComponent (Component* component);
        std::map<std::string, Component*>& GetComponents () { return m_Components; }

        virtual void DrawBounds ();
        virtual void DrawName ();

        std::function<void(float dx, float dy)> MoveCallback;

        virtual std::string GetTypeName () = 0;

    protected:
        void ProcessTriggerAreas (float dx, float dy);

        void ChooseAnimation (float angleDeg);

        void FireMoveCallback ();

    protected:
        Point m_OldPosition;
        std::map<std::string, Component*> m_Components;

    public:
        Rect TemplateBounds;
    };
}

#endif //   __ACTOR_H__
