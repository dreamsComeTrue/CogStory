// Copyright 2017-2019 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __PLAYER_INVENTORY_H__
#define __PLAYER_INVENTORY_H__

#include "Common.h"

namespace aga
{
	class Player;
	class AudioSample;
	class Atlas;

	class PlayerInventory : public Lifecycle
	{
	public:
		PlayerInventory (Player* player);
		virtual ~PlayerInventory ();
		bool Initialize () override;
		bool Destroy () override;

		bool Update (float deltaTime);
		void Render (float deltaTime);

	private:
		void HandleAction ();

	private:
		Player* m_Player;
		Atlas* m_Atlas;
		AudioSample* m_SelectItemSample;
	};
}

#endif //   __PLAYER_INVENTORY_H__
