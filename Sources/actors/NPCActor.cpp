// Copyright 2017-2019 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "NPCActor.h"

namespace aga
{
	//--------------------------------------------------------------------------------------------------

	std::string NPCActor::TypeName = "NPC";

	//--------------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------------

	NPCActor::NPCActor (SceneManager* sceneManager)
		: Actor (sceneManager)
	{
	}

	//--------------------------------------------------------------------------------------------------

	NPCActor::NPCActor (const NPCActor& rhs)
		: Actor (rhs)
	{
	}

	//--------------------------------------------------------------------------------------------------

	NPCActor* NPCActor::Clone () const { return new NPCActor (*this); }

	//--------------------------------------------------------------------------------------------------

	bool NPCActor::Initialize ()
	{
		Actor::Initialize ();

		if (GetAnimation ().GetName () != "")
		{
			SetCurrentAnimation (ANIM_IDLE_NAME);
		}

		return true;
	}

	bool NPCActor::Update (float deltaTime) { return Actor::Update (deltaTime); }

	//--------------------------------------------------------------------------------------------------
}
