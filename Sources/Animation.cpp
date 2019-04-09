// Copyright 2017-2019 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "Animation.h"

namespace aga
{
	//--------------------------------------------------------------------------------------------------

	AnimationData::AnimationData (unsigned howManyFrames, Point cellSize)
		: m_CellSize (cellSize)
		, m_SpeedMS (1000)
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

	void AnimationData::SetFrames (std::vector<AnimationFrameEntry> frames) { m_Frames = frames; }

	//--------------------------------------------------------------------------------------------------

	AnimationFrameEntry& AnimationData::GetFrame (unsigned index) { return m_Frames[index]; }

	//--------------------------------------------------------------------------------------------------

	std::vector<AnimationFrameEntry>& AnimationData::GetFrames () { return m_Frames; }

	//--------------------------------------------------------------------------------------------------

	size_t AnimationData::GetFramesCount () const { return m_Frames.size (); }

	//--------------------------------------------------------------------------------------------------

	void AnimationData::SetName (const std::string& name) { m_Name = name; }

	//--------------------------------------------------------------------------------------------------

	std::string AnimationData::GetName () { return m_Name; }

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

	void Animation::SetAnimationData (const std::string& name, const AnimationData& frames)
	{
		m_Animations[name] = frames;
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

	void Animation::RemoveAnimation (const std::string& name) { m_Animations.erase (name); }

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

	void Animation::SetName (const std::string& name) { m_Name = name; }

	//--------------------------------------------------------------------------------------------------

	std::string Animation::GetName () { return m_Name; }

	//--------------------------------------------------------------------------------------------------
}
