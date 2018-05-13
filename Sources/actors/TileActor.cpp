// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "TileActor.h"
#include "Atlas.h"
#include "AtlasManager.h"
#include "Font.h"
#include "MainLoop.h"
#include "SceneManager.h"
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

    bool TileActor::Initialize ()
    {
        Actor::Initialize ();

        if (Tileset != "" && m_AtlasRegionName != "")
        {
            Bounds.Size = m_SceneManager->GetMainLoop ()
                              ->GetAtlasManager ()
                              .GetAtlas (Tileset)
                              ->GetRegion (m_AtlasRegionName)
                              .Bounds.GetSize ();
        }

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    bool TileActor::Update (float deltaTime) { return Actor::Update (deltaTime); }

    //--------------------------------------------------------------------------------------------------

    void TileActor::Render (float deltaTime)
    {
        Atlas* atlas = m_SceneManager->GetMainLoop ()->GetAtlasManager ().GetAtlas (Tileset);

        if (atlas)
        {
            Point pos = Bounds.GetPos ();
            atlas->DrawRegion (m_AtlasRegionName, pos.X, pos.Y, 1.0f, 1.0f, DegressToRadians (Rotation));
        }
    }

    //--------------------------------------------------------------------------------------------------

    void TileActor::DrawName ()
    {
        Atlas* atlas = m_SceneManager->GetMainLoop ()->GetAtlasManager ().GetAtlas (Tileset);

        if (atlas)
        {
            Rect regionBounds = atlas->GetRegion (m_AtlasRegionName).Bounds;

            Font& font = m_SceneManager->GetMainLoop ()->GetScreen ()->GetFont ();
            Point pos = {Bounds.GetPos ().X + regionBounds.GetHalfSize ().Width,
                Bounds.GetPos ().Y + regionBounds.GetSize ().Height};
            std::string str = Name + "[" + ToString (ID) + "]";
            font.DrawText (FONT_NAME_SMALL, al_map_rgb (0, 255, 0), pos.X, pos.Y, str, ALLEGRO_ALIGN_CENTER);
        }
    }

    //--------------------------------------------------------------------------------------------------

    void TileActor::SetAtlasRegionName (const std::string& name) { m_AtlasRegionName = name; }

    //--------------------------------------------------------------------------------------------------

    void TileActor::SetAtlas (Atlas* atlas)
    {
        m_Atlas = atlas;
        Tileset = atlas != nullptr ? atlas->GetName () : "";
    }

    //--------------------------------------------------------------------------------------------------
}
