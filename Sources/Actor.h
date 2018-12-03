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

    const std::string ANIM_STAND_DOWN_NAME = "ANIM_STAND_DOWN";
    const std::string ANIM_STAND_UP_NAME = "ANIM_STAND_UP";
    const std::string ANIM_STAND_LEFT_NAME = "ANIM_STAND_LEFT";
    const std::string ANIM_STAND_RIGHT_NAME = "ANIM_STAND_RIGHT";

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

        virtual void SetCenterPosition (float x, float y);
        virtual void SetCenterPosition (Point pos);

        Point GetPosition ();
        Point GetSize ();

        void AddComponent (const std::string& name, Component* component);
        void RemoveComponent (const std::string& name);
        void RemoveComponent (Component* component);
        Component* FindComponent (const std::string& name, const std::string& typeName);
        class MovementComponent* GetMovementComponent (const std::string& name);
        class ParticleEmitterComponent* GetParticleEmitterComponent (const std::string& name);
        std::map<std::string, Component*>& GetComponents ();

        void OrientTo (Actor* actor);
        void ChooseWalkAnimation (float angleDeg);
        void ChooseStandAnimation (float angleDeg);

        virtual void DrawBounds ();
        virtual void DrawName ();

        void SuspendUpdate ();
        void ResumeUpdate ();

        std::function<void(float dx, float dy)> MoveCallback;

        virtual std::string GetTypeName () = 0;

        //  Helper functions
        void AssignFlagPointsToWalk (const std::string& flagPointName);

        float GetFocusHeight () const;
        void SetFocusHeight (float focusHeight);

        std::string GetActionSpeech ();
        void SetActionSpeech (const std::string& speechID);

        bool IsActionSpeechHandling ();
        void SetActionSpeechHandling (bool handling);

    protected:
        void ProcessTriggerAreas (float dx, float dy, Point&& offset);
        void RenderComponents (float deltaTime);

        void FireMoveCallback ();

    protected:
        bool m_IsUpdating;
        Point m_OldPosition;
        std::map<std::string, Component*> m_Components;

        //  At which percent [0..1] Player should be rendered OVER this actor
        float m_FocusHeight;

        //  Speech triggered when 'action' with player
        std::string m_ActionSpeech;
        bool m_ActionSpeechHandling;

    public:
        Rect TemplateBounds;
    };
}

#endif //   __ACTOR_H__
