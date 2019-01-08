// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __TRANSFORMABLE_H__
#define __TRANSFORMABLE_H__

#include "Rect.h"

namespace aga
{
	class Transformable
	{
	public:
		Transformable () {}

		Transformable (const Transformable& rhs)
		{
			this->Bounds = rhs.Bounds;
			this->Rotation = rhs.Rotation;
		}

		Rect Bounds;
		float Rotation = 0;
	};
}

#endif //   __TRANSFORMABLE_H__
