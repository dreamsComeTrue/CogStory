// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "TileActor.h"
#include "Atlas.h"
#include "AtlasManager.h"
#include "SceneManager.h"

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

    bool TileActor::Update (float deltaTime) { return Actor::Update (deltaTime); }

    //--------------------------------------------------------------------------------------------------

    void TileActor::Render (float deltaTime)
    {
        Atlas* atlas = m_SceneManager->GetAtlasManager ()->GetAtlas (Tileset);

        if (atlas)
        {
            Point pos = Bounds.GetPos ();
            atlas->DrawRegion (Name, pos.X, pos.Y, 1.0f, 1.0f, DegressToRadians (Rotation));
        }
    }

    //--------------------------------------------------------------------------------------------------

    std::string TileActor::GetTypeName () { return TypeName; }

    //--------------------------------------------------------------------------------------------------
}
