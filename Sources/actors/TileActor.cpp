// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "TileActor.h"
#include "Atlas.h"
#include "MainLoop.h"
#include "Screen.h"

namespace aga
{
	//--------------------------------------------------------------------------------------------------

	std::string TileActor::TypeName = "Tile";

	//--------------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------------

	TileActor::TileActor (SceneManager* sceneManager)
		: Actor (sceneManager)
	{
	}

	//--------------------------------------------------------------------------------------------------

	TileActor::TileActor (const TileActor& rhs)
		: Actor (rhs)
	{
	}

	//--------------------------------------------------------------------------------------------------

	TileActor* TileActor::Clone () const { return new TileActor (*this); }

	//--------------------------------------------------------------------------------------------------

	bool TileActor::Initialize () { return Actor::Initialize (); }

	//--------------------------------------------------------------------------------------------------

	bool TileActor::Update (float deltaTime) { return Actor::Update (deltaTime); }

	//--------------------------------------------------------------------------------------------------

	void TileActor::Render (float deltaTime)
	{
		if (m_Atlas)
		{
			Point pos = Bounds.GetPos ();
			m_Atlas->DrawRegion (m_AtlasRegionName, pos.X, pos.Y, 1.0f, 1.0f, DegressToRadians (Rotation));
		}

		RenderComponents (deltaTime);
	}

	//--------------------------------------------------------------------------------------------------

	void TileActor::DrawName ()
	{
		if (m_Atlas)
		{
			Point translate = m_SceneManager->GetCamera ().GetTranslate ();
			Point scale = m_SceneManager->GetCamera ().GetScale ();

			Rect regionBounds = m_Atlas->GetRegion (m_AtlasRegionName).Bounds;
			Point pos = {Bounds.GetPos ().X + regionBounds.GetHalfSize ().Width,
				Bounds.GetPos ().Y + regionBounds.GetSize ().Height};
				
			float xPoint = pos.X * scale.X - translate.X;
			float yPoint = pos.Y * scale.Y - translate.Y;

			Font& font = m_SceneManager->GetMainLoop ()->GetScreen ()->GetFont ();
			std::string str = Name + "[" + std::to_string (ID) + "]";
			font.DrawText (FONT_NAME_SMALL, str, al_map_rgb (0, 255, 0), xPoint, yPoint, 1.0f, ALLEGRO_ALIGN_CENTER);
		}
	}

	//--------------------------------------------------------------------------------------------------
}
