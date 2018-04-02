// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __ANIMABLE_H__
#define __ANIMABLE_H__

#include "Animation.h"
#include "Common.h"

namespace aga
{
    class PhysicsManager;
    class Transformable;

    class Animable
    {
    public:
        Animable ();
        virtual ~Animable ();

        bool Initialize (const std::string& filePath);
        bool Destroy ();

        bool Update (float deltaTime);
        void Render (Transformable* transformable);

        void SetCurrentAnimation (const std::string& name);

        ALLEGRO_BITMAP* GetImage ();
        Animation& GetAnimation () { return m_Animation; }
        void SetAnimation (Animation animation) { m_Animation = animation; }
        std::map<std::string, AnimationFrames>& GetAnimations ();

        std::string GetImagePath () const { return m_ImagePath; }

    private:
        ALLEGRO_BITMAP* m_Image;
        std::string m_ImagePath;
        int m_ImageWidth;
        int m_ImageHeight;

        Animation m_Animation;
    };
}

#endif //   __ANIMABLE_H__
