// Copyright 2017-2019 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.
#include "ActorFactory.h"
#include "actors/EnemyActor.h"
#include "actors/NPCActor.h"
#include "actors/TileActor.h"

#include "actors/components/AudioSampleComponent.h"
#include "actors/components/MovementComponent.h"
#include "actors/components/ParticleEmitterComponent.h"
#include "actors/components/AnimPresetComponent.h"

using json = nlohmann::json;

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    std::vector<std::string> ActorFactory::s_ActorTypes;
    std::vector<std::string> ActorFactory::s_ActorComponents;
    std::map<std::string, Animation> ActorFactory::s_Animations;
    Animation ActorFactory::s_DummyAnimation;

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
        s_ActorComponents.push_back (AnimPresetComponent::TypeName);
    }

    //--------------------------------------------------------------------------------------------------

    void ActorFactory::RegisterAnimations ()
    {
        const std::string animPath = GetDataPath () + "/animations/";
        std::vector<std::string> files = GetFilesInDirectory (animPath);

        for (std::string& fileName : files)
        {
            Animation animation = LoadAnimationFromFile (animPath + "/" + fileName);

            if (animation.GetName () != "")
            {
                s_Animations[animation.GetName ()] = animation;
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    std::vector<std::string>& ActorFactory::GetActorTypes () { return s_ActorTypes; }

    //--------------------------------------------------------------------------------------------------

    std::vector<std::string>& ActorFactory::GetActorComponents () { return s_ActorComponents; }

    //--------------------------------------------------------------------------------------------------

    Animation ActorFactory::LoadAnimationFromFile (const std::string& path)
    {
        std::ifstream animFile (path);
        json j;
        animFile >> j;
        animFile.close ();

        Animation animation;
        std::string name = GetBaseName (j["animation"]);

        if (EndsWith (name, ".anim"))
        {
            name = name.substr (0, name.find (".anim"));
        }

        animation.SetName (name);

        auto& animations = j["animations"];

        for (auto& animationEntry : animations)
        {
            AnimationData animData;

            animData.SetName (animationEntry["name"]);
            animData.SetPlaySpeed (animationEntry["speed"]);

            auto& frames = animationEntry["frames"];

            for (auto& frame : frames)
            {
                AnimationFrameEntry frameEntry;

                frameEntry.Atlas = frame["atlas"];
                frameEntry.AtlasRegion = frame["region"];

                std::vector<Point> points = StringToVectorPoints (frame["bounds"]);
                frameEntry.Bounds.Pos = points[0];
                frameEntry.Bounds.Size = points[1];

                animData.AddFrame (frameEntry);
            }

            animation.AddAnimationData (animData.GetName (), animData);
        }

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
        else if (type == AnimPresetComponent::TypeName)
        {
            newComponent = new AnimPresetComponent (actor);
        }

        return newComponent;
    }

    //--------------------------------------------------------------------------------------------------

    Animation& ActorFactory::GetAnimation (const std::string& name)
    {
        for (auto& kv : s_Animations)
        {
            if (kv.first == name)
            {
                return s_Animations[name];
            }
        }

        return s_DummyAnimation;
    }

    //--------------------------------------------------------------------------------------------------

    Animation& ActorFactory::GetDummyAnimation () { return s_DummyAnimation; }

    //--------------------------------------------------------------------------------------------------

    std::map<std::string, Animation>& ActorFactory::GetAnimations () { return s_Animations; }

    //--------------------------------------------------------------------------------------------------
}
