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
            atlas->DrawRegion (Name, Bounds.GetPos ().X, Bounds.GetPos ().Y, 1, 1, DegressToRadians (Rotation));
        }
    }

    //--------------------------------------------------------------------------------------------------

    std::string TileActor::GetTypeName () { return TypeName; }

    //--------------------------------------------------------------------------------------------------
}
