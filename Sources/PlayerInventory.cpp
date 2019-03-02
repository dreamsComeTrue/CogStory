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
		, m_Atlas (nullptr)
	{
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

		m_Atlas = m_Player->GetSceneManager ()->GetMainLoop ()->GetAtlasManager ().GetAtlas (
			GetBaseName (GetResourcePath (PACK_PLAYER)));

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
		AtlasRegion& region = m_Atlas->GetRegion ("inventory");
		Point halfSize = region.Bounds.GetHalfSize ();

		m_Atlas->DrawRegion ("inventory", winSize.Width * 0.5f - halfSize.Width,
			winSize.Height * 0.5f - halfSize.Height, 1.0f, 1.0f, 0.f);
	}

	//--------------------------------------------------------------------------------------------------

	void PlayerInventory::HandleAction () {}

	//--------------------------------------------------------------------------------------------------
}
