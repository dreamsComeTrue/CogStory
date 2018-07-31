// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.
#include "ActorFactory.h"
#include "actors/EnemyActor.h"
#include "actors/NPCActor.h"
#include "actors/TileActor.h"

#include "actors/components/AudioSampleComponent.h"
#include "actors/components/MovementComponent.h"
#include "actors/components/ParticleEmitterComponent.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    std::vector<std::string> ActorFactory::s_ActorTypes;
    std::vector<std::string> ActorFactory::s_ActorComponents;
    std::map<std::string, Animation> ActorFactory::s_Animations;

    //--------------------------------------------------------------------------------------------------

    ActorFactory::ActorFactory () {}

    //--------------------------------------------------------------------------------------------------

    void ActorFactory::RegisterActorTypes ()
    {
        s_ActorTypes.push_back (NPCActor::TypeName);
        s_ActorTypes.push_back (EnemyActor::TypeName);
        s_ActorTypes.push_back (TileActor::TypeName);
    }

    //--------------------------------------------------------------------------------------------------

    void ActorFactory::RegisterActorComponents ()
    {
        s_ActorComponents.push_back (AudioSampleComponent::TypeName);
        s_ActorComponents.push_back (MovementComponent::TypeName);
        s_ActorComponents.push_back (ParticleEmitterComponent::TypeName);
    }

    //--------------------------------------------------------------------------------------------------

    void ActorFactory::RegisterAnimations () { s_Animations[ANIMATION_PLAYER] = RegisterAnimation_Player (); }

    //--------------------------------------------------------------------------------------------------

    Animation ActorFactory::RegisterAnimation_Player ()
    {
        Animation animation;
        Point cellSize (64, 64);
        float animSpeed = 80;

        AnimationFrames idleFrames (3, cellSize);
        idleFrames.AddFrame (0, 0, 0);
        idleFrames.AddFrame (1, 0, 1);
        idleFrames.AddFrame (2, 0, 2);
        idleFrames.SetPlaySpeed (500);
        animation.AddFrames (ANIM_IDLE_NAME, idleFrames);

        AnimationFrames moveDownFrames (6, cellSize);
        moveDownFrames.AddFrame (0, 1, 0);
        moveDownFrames.AddFrame (1, 1, 1);
        moveDownFrames.AddFrame (2, 1, 2);
        moveDownFrames.AddFrame (3, 1, 3);
        moveDownFrames.AddFrame (4, 1, 4);
        moveDownFrames.AddFrame (5, 1, 5);
        moveDownFrames.SetPlaySpeed (animSpeed);
        animation.AddFrames (ANIM_MOVE_DOWN_NAME, moveDownFrames);

        AnimationFrames moveLeftFrames (6, cellSize);
        moveLeftFrames.AddFrame (0, 2, 0);
        moveLeftFrames.AddFrame (1, 2, 1);
        moveLeftFrames.AddFrame (2, 2, 2);
        moveLeftFrames.AddFrame (3, 2, 3);
        moveLeftFrames.AddFrame (4, 2, 4);
        moveLeftFrames.AddFrame (5, 2, 5);
        moveLeftFrames.SetPlaySpeed (animSpeed + 30);
        animation.AddFrames (ANIM_MOVE_LEFT_NAME, moveLeftFrames);

        AnimationFrames moveRightFrames (6, cellSize);
        moveRightFrames.AddFrame (0, 3, 0);
        moveRightFrames.AddFrame (1, 3, 1);
        moveRightFrames.AddFrame (2, 3, 2);
        moveRightFrames.AddFrame (3, 3, 3);
        moveRightFrames.AddFrame (4, 3, 4);
        moveRightFrames.AddFrame (5, 3, 5);
        moveRightFrames.SetPlaySpeed (animSpeed + 30);
        animation.AddFrames (ANIM_MOVE_RIGHT_NAME, moveRightFrames);

        AnimationFrames moveUpFrames (6, cellSize);
        moveUpFrames.AddFrame (0, 4, 0);
        moveUpFrames.AddFrame (1, 4, 1);
        moveUpFrames.AddFrame (2, 4, 2);
        moveUpFrames.AddFrame (3, 4, 3);
        moveUpFrames.AddFrame (4, 4, 4);
        moveUpFrames.AddFrame (5, 4, 5);
        moveUpFrames.SetPlaySpeed (animSpeed);
        animation.AddFrames (ANIM_MOVE_UP_NAME, moveUpFrames);

        AnimationFrames moveUpLook (1, cellSize);
        moveUpLook.AddFrame (0, 4, 0);
        moveUpLook.SetPlaySpeed (animSpeed);
        animation.AddFrames (ANIM_MOVE_UP_LOOK_NAME, moveUpLook);

        return animation;
    }

    //--------------------------------------------------------------------------------------------------

    Actor* ActorFactory::GetActor (SceneManager* sceneManager, const std::string& type)
    {
        Actor* newActor = nullptr;

        if (type == NPCActor::TypeName)
        {
            newActor = new NPCActor (sceneManager);
        }
        else if (type == EnemyActor::TypeName)
        {
            newActor = new EnemyActor (sceneManager);
        }
        else if (type == TileActor::TypeName)
        {
            newActor = new TileActor (sceneManager);
        }

        return newActor;
    }

    //--------------------------------------------------------------------------------------------------

    Component* ActorFactory::GetActorComponent (Actor* actor, const std::string& type)
    {
        Component* newComponent = nullptr;

        if (type == AudioSampleComponent::TypeName)
        {
            newComponent = new AudioSampleComponent (actor);
        }
        else if (type == MovementComponent::TypeName)
        {
            newComponent = new MovementComponent (actor);
        }
        else if (type == ParticleEmitterComponent::TypeName)
        {
            newComponent = new ParticleEmitterComponent (actor);
        }

        return newComponent;
    }

    //--------------------------------------------------------------------------------------------------

    Animation& ActorFactory::GetAnimation (const std::string& name) { return s_Animations[name]; }

    //--------------------------------------------------------------------------------------------------
}
