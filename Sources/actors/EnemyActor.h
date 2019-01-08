// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __ENEMY_ACTOR_H__
#define __ENEMY_ACTOR_H__

#include "Actor.h"

namespace aga
{
	class EnemyActor : public Actor
	{
	public:
		static std::string TypeName;

	public:
		EnemyActor (SceneManager* sceneManager);
		EnemyActor (const EnemyActor& rhs);
		virtual EnemyActor* Clone () const override;

		virtual bool Initialize () override;

		virtual bool Update (float deltaTime) override;

		virtual std::string GetTypeName () override { return TypeName; }
	};
}

#endif //   __ENEMY_ACTOR_H__
