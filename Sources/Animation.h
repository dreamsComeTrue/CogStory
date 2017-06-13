// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __ANIMATION_H__
#define __ANIMATION_H__

#include "Common.h"

namespace aga
{
    class AnimationFrames
    {
    public:
        AnimationFrames (unsigned howManyFrames = 0);

        void AddFrame (unsigned index, const Rect& rect);
        Rect& GetFrame (unsigned index);

    private:
        std::vector<Rect> m_Frames;
    };

    class Animation
    {
    public:
        Animation ();

        void AddAnimationFrames (const std::string& name, const AnimationFrames& frames);
        AnimationFrames& GetAnimation (const std::string& name);

    private:
        std::map<std::string, AnimationFrames> m_Animations;
    };
}

#endif //   __ANIMATION_H__
