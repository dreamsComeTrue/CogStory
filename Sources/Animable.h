// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __ANIMABLE_H__
#define __ANIMABLE_H__

#include "Animation.h"
#include "Common.h"

namespace aga
{
    class PhysicsManager;
    class Transformable;
    class AtlasManager;
    class Atlas;

    class Animable
    {
    public:
        Animable (AtlasManager* atlasManager);
        virtual ~Animable ();

        bool Initialize (const std::string& atlasName, const std::string& atlasRegionName);
        bool Destroy ();

        bool Update (float deltaTime);
        void Render (Transformable* transformable);

        void SetCurrentAnimation (const std::string& name);

        Animation& GetAnimation () { return m_Animation; }
        void SetAnimation (Animation animation) { m_Animation = animation; }
        std::map<std::string, AnimationFrames>& GetAnimations ();

        void SetAtlasRegionName (const std::string& name) { m_AtlasRegionName = name; }
        void SetAtlas (Atlas* atlas) { m_Atlas = atlas; }

        std::string GetAtlasRegionName () const { return m_AtlasRegionName; }
        Atlas* GetAtlas () { return m_Atlas; }

    private:
        AtlasManager* m_AtlasManager;
        Atlas* m_Atlas;
        std::string m_AtlasRegionName;

        Animation m_Animation;
    };
}

#endif //   __ANIMABLE_H__
