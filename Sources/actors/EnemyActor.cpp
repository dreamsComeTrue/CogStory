// Copyright 2017-2019 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "EnemyActor.h"

namespace aga
{
	//--------------------------------------------------------------------------------------------------

	std::string EnemyActor::TypeName = "Enemy";

	//--------------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------------

	EnemyActor::EnemyActor (SceneManager* sceneManager)
		: Actor (sceneManager)
	{
	}

	//--------------------------------------------------------------------------------------------------

	EnemyActor::EnemyActor (const EnemyActor& rhs)
		: Actor (rhs)
	{
	}

	//--------------------------------------------------------------------------------------------------

	EnemyActor* EnemyActor::Clone () const { return new EnemyActor (*this); }

	//--------------------------------------------------------------------------------------------------

	bool EnemyActor::Initialize () { return Actor::Initialize (); }

	//--------------------------------------------------------------------------------------------------

	bool EnemyActor::Update (float deltaTime) { return Actor::Update (deltaTime); }

	//--------------------------------------------------------------------------------------------------
}
