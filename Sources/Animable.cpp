// Copyright 2017-2019 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "Animable.h"
#include "ActorFactory.h"
#include "Atlas.h"
#include "AtlasManager.h"

namespace aga
{
	//--------------------------------------------------------------------------------------------------

	Animable::Animable ()
		: m_AtlasManager (nullptr)
		, m_Atlas (nullptr)
	{
	}
    
    //--------------------------------------------------------------------------------------------------

	Animable::Animable (AtlasManager* atlasManager)
		: m_AtlasManager (atlasManager)
		, m_Atlas (nullptr)
		, m_Animation (ActorFactory::GetDummyAnimation ())
	{
	}

	//--------------------------------------------------------------------------------------------------

	Animable::Animable (const Animable& rhs)
	{
		this->m_AtlasManager = rhs.m_AtlasManager;
		this->m_Atlas = rhs.m_Atlas;
		this->m_AtlasName = rhs.m_AtlasName;
		this->m_AtlasRegionName = rhs.m_AtlasRegionName;
		this->m_Animation = rhs.m_Animation;
	}

	//--------------------------------------------------------------------------------------------------

	Animable::~Animable () { Destroy (); }

	//--------------------------------------------------------------------------------------------------

	bool Animable::Initialize (const std::string& atlasName, const std::string& atlasRegionName)
	{
		m_AtlasName = atlasName;
		m_AtlasRegionName = atlasRegionName;
		m_Atlas = m_AtlasManager->GetAtlas (atlasName);

		return true;
	}

	//--------------------------------------------------------------------------------------------------

	bool Animable::Destroy () { return true; }

	//--------------------------------------------------------------------------------------------------

	bool Animable::Update (float deltaTime)
	{
		if (!m_Animation.GetAnimations ().empty ())
		{
			m_Animation.Update (deltaTime);
		}

		return true;
	}

	//--------------------------------------------------------------------------------------------------

	void Animable::Render (Transformable* transformable)
	{
		if (m_Atlas)
		{
			float sourceX = 0;
			float sourceY = 0;
			float sourceWidth;
			float sourceHeight;

			Point pos = transformable->Bounds.GetPos ();

			if (m_Animation.GetAnimations ().empty ())
			{
				if (m_AtlasRegionName != "")
				{
					m_Atlas->DrawRegion (m_AtlasRegionName, pos.X, pos.Y, 1.f, 1.f, transformable->Rotation, true);
				}
				else
				{
					sourceWidth = al_get_bitmap_width (m_Atlas->GetImage ());
					sourceHeight = al_get_bitmap_height (m_Atlas->GetImage ());

					m_Atlas->DrawRegion (
						0, 0, sourceWidth, sourceHeight, pos.X, pos.Y, 1.f, 1.f, transformable->Rotation, true);
				}
			}
			else
			{
				AnimationData& frames = m_Animation.GetCurrentAnimation ();
				AnimationFrameEntry& frame = frames.GetFrame (m_Animation.GetCurrentFrame ());
				Rect frameRect = frame.Bounds;

				sourceX = frameRect.GetPos ().X;
				sourceY = frameRect.GetPos ().Y;
				sourceWidth = frameRect.GetSize ().Width;
				sourceHeight = frameRect.GetSize ().Height;

				m_Atlas->DrawRegion (
					sourceX, sourceY, sourceWidth, sourceHeight, pos.X, pos.Y, 1, 1, transformable->Rotation, true);
			}
		}
	}

	//--------------------------------------------------------------------------------------------------

	void Animable::SetCurrentAnimation (const std::string& name) { m_Animation.SetCurrentAnimation (name); }

	//--------------------------------------------------------------------------------------------------

	Animation& Animable::GetAnimation () { return m_Animation; }

	//--------------------------------------------------------------------------------------------------

	void Animable::SetAnimation (Animation& animation) { m_Animation = animation; }

	//--------------------------------------------------------------------------------------------------

	std::map<std::string, AnimationData>& Animable::GetAnimationsData () { return m_Animation.GetAnimations (); }

	//--------------------------------------------------------------------------------------------------

	void Animable::SetAtlasName (const std::string& name) { m_AtlasName = name; }

	//--------------------------------------------------------------------------------------------------

	void Animable::SetAtlasRegionName (const std::string& name) { m_AtlasRegionName = name; }

	//--------------------------------------------------------------------------------------------------

	std::string Animable::GetAtlasRegionName () const { return m_AtlasRegionName; }

	//--------------------------------------------------------------------------------------------------

	Atlas* Animable::GetAtlas () { return m_Atlas; }

	//--------------------------------------------------------------------------------------------------

	void Animable::SetAtlas (Atlas* atlas)
	{
		m_Atlas = atlas;
		m_AtlasRegionName = atlas != nullptr ? atlas->GetName () : "";
	}

	//--------------------------------------------------------------------------------------------------
}
