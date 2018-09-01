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

    std::vector<AnimationData> g_AnimationData
        = {{"ANIMATION_PLAYER", "player.anim"}, {"ANIMATION_NPC_1", "npc_1.anim"}};

    //--------------------------------------------------------------------------------------------------
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

    void ActorFactory::RegisterAnimations ()
    {
        const std::string animPath = GetDataPath () + "/animations/";

        for (AnimationData& data : g_AnimationData)
        {
            s_Animations[data.Name] = LoadAnimationFromFile (animPath + "/" + data.FilePath);
        }
    }

    //--------------------------------------------------------------------------------------------------

    Animation ActorFactory::LoadAnimationFromFile (const std::string& path)
    {
        std::ifstream animFile (path);
        std::string line = "";

        Animation animation;

        AnimationFrames* currentFrames = nullptr;

        while (!animFile.eof ())
        {
            getline (animFile, line);

            if (line == "" || line == "\n")
            {
                continue;
            }

            //  We can parse - we have a name
            if (!StartsWith (line, "\t"))
            {
                std::vector<std::string> lineData = SplitString (line, ' ');
                std::string framesName = lineData[0];
                int framesCount = atoi (lineData[1].c_str ());
                int cellWidth = atoi (lineData[2].c_str ());
                int cellHeight = atoi (lineData[3].c_str ());
                int framesSpeed = atoi (lineData[4].c_str ());

                AnimationFrames frames (framesCount, {cellWidth, cellHeight});
                frames.SetPlaySpeed (framesSpeed);

                animation.AddFrames (framesName, frames);
                currentFrames = &animation.GetAnimationFrames (framesName);
            }
            else
            {
                std::vector<std::string> lineData = SplitString (TrimString (line), ' ');

                if (currentFrames)
                {
                    int x = atoi (lineData[0].c_str ());
                    int y = atoi (lineData[1].c_str ());
                    currentFrames->AddFrame (x, y);
                }
            }
        }

        animFile.close ();

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
