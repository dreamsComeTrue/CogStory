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

        void SetPlaySpeed (unsigned milliseconds);
        unsigned GetPlaySpeed () const;

        void AddFrame (unsigned index, const Rect& rect);
        Rect& GetFrame (unsigned index);
        unsigned GetFramesCount () const;

    private:
        std::vector<Rect> m_Frames;
        unsigned m_SpeedMS;
    };

    class Animation
    {
    public:
        Animation ();

        void AddAnimationFrames (const std::string& name, const AnimationFrames& frames);
        AnimationFrames& GetAnimation (const std::string& name);
        AnimationFrames& GetCurrentAnimation ();
        void SetCurrentAnimation (const std::string& name);

        void Update (float deltaTime);
        unsigned GetCurrentFrame () const;

    private:
        std::map<std::string, AnimationFrames> m_Animations;
        std::string m_CurrentAnimation;
        unsigned m_CurrentFrame;
        float m_TimeTaken;
    };
}

#endif //   __ANIMATION_H__
