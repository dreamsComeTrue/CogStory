// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "TileActor.h"
#include "Atlas.h"
#include "AtlasManager.h"
#include "Font.h"
#include "MainLoop.h"
#include "SceneManager.h"
#include "Screen.h"
#include "actors/components/MovementComponent.h"

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

    bool TileActor::Initialize ()
    {
        Actor::Initialize ();

        RemoveComponent (m_MovementComponent);
        m_MovementComponent = nullptr;

        return true;
    }

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
    }

    //--------------------------------------------------------------------------------------------------

    void TileActor::DrawName ()
    {
        if (m_Atlas)
        {
            Rect regionBounds = m_Atlas->GetRegion (m_AtlasRegionName).Bounds;

            Font& font = m_SceneManager->GetMainLoop ()->GetScreen ()->GetFont ();
            Point pos = {Bounds.GetPos ().X + regionBounds.GetHalfSize ().Width,
                Bounds.GetPos ().Y + regionBounds.GetSize ().Height};
            std::string str = Name + "[" + ToString (ID) + "]";
            font.DrawText (FONT_NAME_SMALL, al_map_rgb (0, 255, 0), pos.X, pos.Y, str, ALLEGRO_ALIGN_CENTER);
        }
    }

    //--------------------------------------------------------------------------------------------------
}
