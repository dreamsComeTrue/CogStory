// Copyright 2017-2019 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __ANIMATION_H__
#define __ANIMATION_H__

#include "Common.h"

namespace aga
{
    struct AnimationFrameEntry
    {
        std::string Atlas;
        std::string AtlasRegion;
        Rect Bounds;
    };

    class AnimationData
    {
    public:
        AnimationData (unsigned howManyFrames = 0, Point cellSize = Point (32, 32));

        void SetPlaySpeed (unsigned milliseconds);
        unsigned GetPlaySpeed () const;

        void AddFrame (const AnimationFrameEntry& frame, int index = -1);
        void ClearFrames ();

        AnimationFrameEntry& GetFrame (unsigned index);
        std::vector<AnimationFrameEntry>& GetFrames ();

        size_t GetFramesCount () const;

        void SetName (const std::string& name);
        std::string GetName ();

    private:
        std::string m_Name;
        std::string FilePath;

        Point m_CellSize;
        std::vector<AnimationFrameEntry> m_Frames;
        unsigned m_SpeedMS;
    };

    class Animation
    {
    public:
        Animation ();

        void AddAnimationData (const std::string& name, const AnimationData& frames);
        AnimationData& GetAnimationData (const std::string& name);
        AnimationData& GetCurrentAnimation ();
        std::string GetCurrentAnimationName ();
        std::map<std::string, AnimationData>& GetAnimations ();
        void SetCurrentAnimation (const std::string& name);
        void ClearAnimationData ();

        void Update (float deltaTime);
        unsigned GetCurrentFrame () const;

        void SetName (const std::string& name);
        std::string GetName ();

    private:
        std::string m_Name;

        std::map<std::string, AnimationData> m_Animations;
        std::string m_CurrentAnimationName;
        unsigned m_CurrentFrame;
        float m_TimeTaken;
    };
}

#endif //   __ANIMATION_H__
