// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "Animation.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    AnimationFrames::AnimationFrames (unsigned howManyFrames)
    {
        m_Frames.reserve (howManyFrames);
    }

    //--------------------------------------------------------------------------------------------------

    void AnimationFrames::AddFrame (unsigned index, const Rect& rect)
    {
        if (index > m_Frames.size () - 1)
        {
            return;
        }

        std::vector<Rect>::iterator it = m_Frames.begin ();
        m_Frames.insert (it + index, rect);
    }

    //--------------------------------------------------------------------------------------------------

    Rect& AnimationFrames::GetFrame (unsigned index)
    {
        return m_Frames[index];
    }

    //--------------------------------------------------------------------------------------------------
    //--------------------------------------------------------------------------------------------------

    Animation::Animation ()
    {
    }

    //--------------------------------------------------------------------------------------------------

    void Animation::AddAnimationFrames (const std::string& name, const AnimationFrames& frames)
    {
        m_Animations.insert (std::make_pair (name, frames));
    }

    //--------------------------------------------------------------------------------------------------

    AnimationFrames& Animation::GetAnimation (const std::string& name)
    {
        return m_Animations[name];
    }

    //--------------------------------------------------------------------------------------------------
}
