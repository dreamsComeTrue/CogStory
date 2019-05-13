// Copyright 2017-2019 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.
#include "PlayerInventory.h"
#include "Atlas.h"
#include "AtlasManager.h"
#include "AudioSample.h"
#include "MainLoop.h"
#include "Player.h"
#include "Resources.h"
#include "SceneManager.h"
#include "Screen.h"

namespace aga
{
	//--------------------------------------------------------------------------------------------------

	PlayerInventory::PlayerInventory (Player* player)
		: m_Player (player)
		, m_UIAtlas (nullptr)
		, m_MenuAtlas (nullptr)
	{
		for (int i = 0; i < 3; ++i)
		{
			m_CogRotations[i] = 0.f;
		}
	}

	//--------------------------------------------------------------------------------------------------

	PlayerInventory::~PlayerInventory ()
	{
		if (!IsDestroyed ())
		{
			Destroy ();
		}
	}

	//--------------------------------------------------------------------------------------------------

	bool PlayerInventory::Initialize ()
	{
		Lifecycle::Initialize ();

		m_UIAtlas = m_Player->GetSceneManager ()->GetMainLoop ()->GetAtlasManager ().GetAtlas (
			GetBaseName (GetResourcePath (PACK_PLAYER)));
		m_MenuAtlas = m_Player->GetSceneManager ()->GetMainLoop ()->GetAtlasManager ().GetAtlas (
			GetBaseName (GetResourcePath (PACK_MENU_UI)));

		return true;
	}

	//--------------------------------------------------------------------------------------------------

	bool PlayerInventory::Destroy () { return Lifecycle::Destroy (); }

	//--------------------------------------------------------------------------------------------------

	bool PlayerInventory::Update (float deltaTime) { return true; }

	//--------------------------------------------------------------------------------------------------

	void PlayerInventory::Render (float deltaTime)
	{
		SceneManager* sceneManager = m_Player->GetSceneManager ();
		Point winSize = sceneManager->GetMainLoop ()->GetScreen ()->GetBackBufferSize ();

		AtlasRegion& backRegion = m_UIAtlas->GetRegion ("inventory-back");
		Point halfSize = backRegion.Bounds.GetHalfSize ();

		m_UIAtlas->DrawRegion ("inventory-back", winSize.Width * 0.5f - halfSize.Width,
			winSize.Height * 0.5f - halfSize.Height, 1.0f, 1.0f, 0.f);

		AtlasRegion& cogRegion = m_UIAtlas->GetRegion ("cog");
		halfSize = cogRegion.Bounds.GetHalfSize ();
		
		float cogSize = 1.7f;
		m_MenuAtlas->DrawRegion ("cog", winSize.Width * 0.5f - halfSize.Width - 116, winSize.Height * 0.5f - halfSize.Height - 22,
			cogSize, cogSize, m_CogRotations[0]);
		m_MenuAtlas->DrawRegion ("cog", winSize.Width * 0.5f - halfSize.Width - 32, winSize.Height * 0.5f - halfSize.Height + 12,
			cogSize, cogSize, m_CogRotations[1]);
		m_MenuAtlas->DrawRegion ("cog", winSize.Width * 0.5f - halfSize.Width + 52, winSize.Height * 0.5f - halfSize.Height - 22,
			cogSize, cogSize, -m_CogRotations[2]);

		for (int i = 0; i < 3; ++i)			
		{
			m_CogRotations[i] += deltaTime;
		}

		 AtlasRegion& frontRegion = m_UIAtlas->GetRegion ("inventory-front");
		 halfSize = frontRegion.Bounds.GetHalfSize ();

		 m_UIAtlas->DrawRegion ("inventory-front", winSize.Width * 0.5f - halfSize.Width,
		 	winSize.Height * 0.5f - halfSize.Height, 1.0f, 1.0f, 0.f);
	}

	//--------------------------------------------------------------------------------------------------

	void PlayerInventory::HandleAction () {}

	//--------------------------------------------------------------------------------------------------
}
