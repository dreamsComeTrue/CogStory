// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "Animation.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    AnimationData::AnimationData (unsigned howManyFrames, Point cellSize)
        : m_SpeedMS (1000)
        , m_CellSize (cellSize)
    {
        m_Frames.reserve (howManyFrames);
    }

    //--------------------------------------------------------------------------------------------------

    void AnimationData::SetPlaySpeed (unsigned milliseconds) { m_SpeedMS = milliseconds; }

    //--------------------------------------------------------------------------------------------------

    unsigned AnimationData::GetPlaySpeed () const { return m_SpeedMS; }

    //--------------------------------------------------------------------------------------------------

    void AnimationData::AddFrame (const AnimationFrameEntry& frame, int index)
    {
        if (index >= 0)
        {
            m_Frames.insert (m_Frames.begin () + index, frame);
        }
        else
        {
            m_Frames.push_back (frame);
        }
    }

    //--------------------------------------------------------------------------------------------------

    //    void AnimationData::AddFrame (unsigned index, int row, int col)
    //    {
    //        Rect rect = Rect ({col * m_CellSize.Width, row * m_CellSize.Height},
    //            {col * m_CellSize.Width + m_CellSize.Width, row * m_CellSize.Height + m_CellSize.Height});
    //        m_Frames.insert (m_Frames.begin () + index, rect);
    //    }

    //    //--------------------------------------------------------------------------------------------------

    //    void AnimationData::AddFrame (int row, int col)
    //    {
    //        Rect rect = Rect ({col * m_CellSize.Width, row * m_CellSize.Height},
    //            {col * m_CellSize.Width + m_CellSize.Width, row * m_CellSize.Height + m_CellSize.Height});
    //        m_Frames.push_back (rect);
    //    }

    //--------------------------------------------------------------------------------------------------

    AnimationFrameEntry& AnimationData::GetFrame (unsigned index) { return m_Frames[index]; }

    //--------------------------------------------------------------------------------------------------

    size_t AnimationData::GetFramesCount () const { return m_Frames.size (); }

    //--------------------------------------------------------------------------------------------------
    //--------------------------------------------------------------------------------------------------

    Animation::Animation ()
        : m_CurrentAnimationName ("")
        , m_CurrentFrame (0)
    {
    }

    //--------------------------------------------------------------------------------------------------

    void Animation::AddAnimationData (const std::string& name, const AnimationData& frames)
    {
        m_Animations.insert (std::make_pair (name, frames));
    }

    //--------------------------------------------------------------------------------------------------

    void Animation::ClearAnimationData () { m_Animations.clear (); }

    //--------------------------------------------------------------------------------------------------

    AnimationData& Animation::GetAnimationData (const std::string& name) { return m_Animations[name]; }

    //--------------------------------------------------------------------------------------------------

    AnimationData& Animation::GetCurrentAnimation () { return m_Animations[m_CurrentAnimationName]; }

    //--------------------------------------------------------------------------------------------------

    std::string Animation::GetCurrentAnimationName () { return m_CurrentAnimationName; }

    //--------------------------------------------------------------------------------------------------

    std::map<std::string, AnimationData>& Animation::GetAnimations () { return m_Animations; }

    //--------------------------------------------------------------------------------------------------

    void Animation::SetCurrentAnimation (const std::string& name)
    {
        if (m_CurrentAnimationName != name)
        {
            m_CurrentAnimationName = name;
            m_CurrentFrame = 0;
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Animation::Update (float deltaTime)
    {
        std::map<std::string, AnimationData>::iterator it = m_Animations.find (m_CurrentAnimationName);

        if (it != m_Animations.end ())
        {
            AnimationData& animation = it->second;

            m_TimeTaken += deltaTime * 1000;

            if (m_TimeTaken >= animation.GetPlaySpeed ())
            {
                ++m_CurrentFrame;
                m_TimeTaken = 0;
            }

            if (m_CurrentFrame >= animation.GetFramesCount ())
            {
                m_CurrentFrame = 0;
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    unsigned Animation::GetCurrentFrame () const { return m_CurrentFrame; }

    //--------------------------------------------------------------------------------------------------
}
