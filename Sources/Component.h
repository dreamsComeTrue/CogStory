// Copyright 2017-2019 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __COMPONENT_H__
#define __COMPONENT_H__

#include "Actor.h"
#include "Lifecycle.h"

namespace aga
{
	class Component : public Lifecycle
	{
	public:
		Component (Actor* owner);
		Component (const Component& rhs);
		virtual ~Component () {}

		virtual Component* Clone () const = 0;

		virtual bool Update (float deltaTime) = 0;

		virtual bool Render (float deltaTime) = 0;

		void SetEnabled (bool enabled);
		bool IsEnabled () const;

		void SetActor (Actor* actor);
		Actor* GetActor ();

		virtual std::string GetTypeName () = 0;

	protected:
		Actor* m_Actor;
		bool m_IsEnabled;
	};
}

#endif //   __COMPONENT_H__
