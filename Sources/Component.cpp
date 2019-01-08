// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "Component.h"

namespace aga
{
	//--------------------------------------------------------------------------------------------------

	Component::Component (Actor* owner)
		: m_Actor (owner)
		, m_IsEnabled (true)
	{
	}

	//--------------------------------------------------------------------------------------------------

	Component::Component (const Component& rhs)
	{
		this->m_Actor = rhs.m_Actor;
		this->m_IsEnabled = rhs.m_IsEnabled;
	}
	
	//--------------------------------------------------------------------------------------------------

	void Component::SetEnabled (bool enabled) { m_IsEnabled = enabled; }
	
	//--------------------------------------------------------------------------------------------------
	
	bool Component::IsEnabled () const { return m_IsEnabled; }
	
	//--------------------------------------------------------------------------------------------------

	void Component::SetActor (Actor* actor) { m_Actor = actor; }
	
	//--------------------------------------------------------------------------------------------------
	
	Actor* Component::GetActor () { return m_Actor; }

	//--------------------------------------------------------------------------------------------------
}
