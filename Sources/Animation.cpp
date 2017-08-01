// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "Animation.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    AnimationFrames::AnimationFrames (unsigned howManyFrames)
        : m_SpeedMS (1000)
    {
        m_Frames.reserve (howManyFrames);
    }

    //--------------------------------------------------------------------------------------------------

    void AnimationFrames::SetPlaySpeed (unsigned milliseconds)
    {
      m_SpeedMS = milliseconds;
    }

    //--------------------------------------------------------------------------------------------------

    unsigned AnimationFrames::GetPlaySpeed () const
    {
        return m_SpeedMS;
    }

    //--------------------------------------------------------------------------------------------------

    void AnimationFrames::AddFrame (unsigned index, const Rect& rect)
    {
        std::vector<Rect>::iterator it = m_Frames.begin ();
        m_Frames.insert (it + index, rect);
    }

    //--------------------------------------------------------------------------------------------------

    Rect& AnimationFrames::GetFrame (unsigned index)
    {
        return m_Frames[index];
    }

    //--------------------------------------------------------------------------------------------------

    unsigned AnimationFrames::GetFramesCount () const
    {
        return m_Frames.size ();
    }

    //--------------------------------------------------------------------------------------------------
    //--------------------------------------------------------------------------------------------------

    Animation::Animation ()
        : m_CurrentAnimation ("")
        , m_CurrentFrame (0)
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

    AnimationFrames& Animation::GetCurrentAnimation ()
    {
        return m_Animations[m_CurrentAnimation];
    }

    //--------------------------------------------------------------------------------------------------

    void Animation::SetCurrentAnimation (const std::string& name)
    {
        m_CurrentAnimation = name;
        m_CurrentFrame = 0;
    }

    //--------------------------------------------------------------------------------------------------

    void Animation::Update (double deltaTime)
    {
        std::map<std::string, AnimationFrames>::iterator it = m_Animations.find (m_CurrentAnimation);

        if (it != m_Animations.end ())
        {
            AnimationFrames& animation = it->second;

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

    unsigned Animation::GetCurrentFrame () const
    {
        return m_CurrentFrame;
    }

    //--------------------------------------------------------------------------------------------------
}
