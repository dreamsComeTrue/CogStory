// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "Animable.h"
#include "Atlas.h"
#include "AtlasManager.h"
#include "Transformable.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    Animable::Animable (AtlasManager* atlasManager)
        : m_AtlasManager (atlasManager)
        , m_Atlas (nullptr)
    {
    }

    //--------------------------------------------------------------------------------------------------

    Animable::~Animable () { Destroy (); }

    //--------------------------------------------------------------------------------------------------

    bool Animable::Initialize (const std::string& atlasName, const std::string& atlasRegionName)
    {
        m_Atlas = m_AtlasManager->GetAtlas (atlasName);
        m_AtlasRegionName = atlasRegionName;

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

            Rect bounds = m_Atlas->GetRegion (m_AtlasRegionName).Bounds;
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
                AnimationFrames& frames = m_Animation.GetCurrentAnimation ();
                Rect& frame = frames.GetFrame (m_Animation.GetCurrentFrame ());

                sourceX = frame.GetPos ().X;
                sourceY = frame.GetPos ().Y;
                sourceWidth = frame.GetSize ().Width;
                sourceHeight = frame.GetSize ().Height;

                m_Atlas->DrawRegion (
                    sourceX, sourceY, sourceWidth, sourceHeight, pos.X, pos.Y, 1, 1, transformable->Rotation, true);
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Animable::SetCurrentAnimation (const std::string& name) { m_Animation.SetCurrentAnimation (name); }

    //--------------------------------------------------------------------------------------------------

    std::map<std::string, AnimationFrames>& Animable::GetAnimations () { return m_Animation.GetAnimations (); }

    //--------------------------------------------------------------------------------------------------
}
