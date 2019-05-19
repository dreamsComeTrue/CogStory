// Copyright 2017-2019 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "AnimPresetComponent.h"

namespace aga
{
	//--------------------------------------------------------------------------------------------------

	const int DEFAULT_MAX_HEIGHT = 10;
	const float DEFAULT_JUMP_SPEED = 40;

	std::string AnimPresetComponent::TypeName = "AnimPresetComponent";

	//--------------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------------

	AnimPresetComponent::AnimPresetComponent (Actor* owner)
		: Component (owner)
		, m_AnimPresetType (NoAnim)
		, m_StartPos (owner->Bounds.Pos)
		, m_EndPos (owner->Bounds.Pos)
		, m_GoingUp (true)
		, m_MaxHeight (DEFAULT_MAX_HEIGHT)
		, m_JumpSpeed (DEFAULT_JUMP_SPEED)
		, m_CurrentJumpTimes (0)
		, m_MaxJumpTimes (1)
	{
	}

	//--------------------------------------------------------------------------------------------------

	AnimPresetComponent::AnimPresetComponent (const AnimPresetComponent& rhs)
		: Component (rhs.m_Actor)
	{
		this->m_AnimPresetType = rhs.m_AnimPresetType;
		this->m_StartPos = rhs.m_StartPos;
	}

	//--------------------------------------------------------------------------------------------------

	AnimPresetComponent* AnimPresetComponent::Clone () const { return new AnimPresetComponent (*this); }

	//--------------------------------------------------------------------------------------------------

	void AnimPresetComponent::SetAnimPresetType (AnimPresetType type) { m_AnimPresetType = type; }

	//--------------------------------------------------------------------------------------------------

	AnimPresetType AnimPresetComponent::GetAnimPresetType () const { return m_AnimPresetType; }

	//--------------------------------------------------------------------------------------------------

	std::string AnimPresetComponent::GetTypeName () { return TypeName; }

	//--------------------------------------------------------------------------------------------------

	bool AnimPresetComponent::Update (float deltaTime)
	{
		if (!m_IsEnabled)
		{
			return true;
		}

		switch (m_AnimPresetType)
		{
		case JumpInPlace:
		{
			if (m_GoingUp)
			{
				m_Actor->Bounds.Pos.Y -= m_JumpSpeed * 1.5f * deltaTime;

				if (m_Actor->Bounds.Pos.Y < m_EndPos.Y)
				{
					m_GoingUp = false;
				}
			}
			else
			{
				m_Actor->Bounds.Pos.Y += m_JumpSpeed * deltaTime;

				if (m_Actor->Bounds.Pos.Y > m_StartPos.Y)
				{
					m_CurrentJumpTimes++;
					m_GoingUp = true;
				}
			}

			if (m_CurrentJumpTimes >= m_MaxJumpTimes)
			{
				m_AnimPresetType = NoAnim;
			}

			break;
		}
		}

		return true;
	}

	//--------------------------------------------------------------------------------------------------

	bool AnimPresetComponent::Render (float)
	{
		if (!m_IsEnabled)
		{
			return true;
		}

		return true;
	}

	//--------------------------------------------------------------------------------------------------

	void AnimPresetComponent::SetJumpInPlaceMaxHeight (int heightInPixels) { m_MaxHeight = heightInPixels; }

	//--------------------------------------------------------------------------------------------------

	void AnimPresetComponent::SetJumpInPlaceSpeed (float speed) { m_JumpSpeed = speed; }

	//--------------------------------------------------------------------------------------------------

	void AnimPresetComponent::SetMaxJumpTimes (int times) { m_MaxJumpTimes = times; }

	//--------------------------------------------------------------------------------------------------

	void AnimPresetComponent::Play ()
	{
		m_StartPos = m_Actor->Bounds.Pos;

		switch (m_AnimPresetType)
		{
		case JumpInPlace:
		{
			m_EndPos.X = m_StartPos.X;
			m_EndPos.Y = m_StartPos.Y - m_MaxHeight;
			m_GoingUp = true;
			m_CurrentJumpTimes = 0;

			break;
		}
		}
	}

	//--------------------------------------------------------------------------------------------------
}
