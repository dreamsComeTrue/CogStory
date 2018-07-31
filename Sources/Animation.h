// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __ANIMATION_H__
#define __ANIMATION_H__

#include "Common.h"

namespace aga
{
    class AnimationFrames
    {
    public:
        AnimationFrames (unsigned howManyFrames = 0, Point cellSize = Point (32, 32));

        void SetPlaySpeed (unsigned milliseconds);
        unsigned GetPlaySpeed () const;

        void AddFrame (unsigned index, const Rect& rect);
        void AddFrame (unsigned index, int row, int col);

        Rect& GetFrame (unsigned index);
        size_t GetFramesCount () const;

        void SetCellSize (Point p);

    private:
        Point m_CellSize;
        std::vector<Rect> m_Frames;
        unsigned m_SpeedMS;
    };

    class Animation
    {
    public:
        Animation ();

        void AddFrames (const std::string& name, const AnimationFrames& frames);
        AnimationFrames& GetAnimation (const std::string& name);
        AnimationFrames& GetCurrentAnimation ();
        std::string GetCurrentAnimationName ();
        std::map<std::string, AnimationFrames>& GetAnimations ();
        void SetCurrentAnimation (const std::string& name);

        void Update (float deltaTime);
        unsigned GetCurrentFrame () const;

    private:
        std::map<std::string, AnimationFrames> m_Animations;
        std::string m_CurrentAnimationName;
        unsigned m_CurrentFrame;
        float m_TimeTaken;
    };
}

#endif //   __ANIMATION_H__
