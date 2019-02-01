// Copyright 2017-2019 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "AtlasManager.h"
#include "Atlas.h"
#include "Resources.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    AtlasManager::AtlasManager () {}

    //--------------------------------------------------------------------------------------------------

    AtlasManager::~AtlasManager ()
    {
        if (!IsDestroyed ())
        {
            Destroy ();
        }
    }

    //--------------------------------------------------------------------------------------------------

    bool AtlasManager::Initialize ()
    {
        Lifecycle::Initialize ();

        for (ResourceID res : GetGfxPacks ())
        {
            LoadFromFile (GetResourcePath (res));
        }

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    bool AtlasManager::Destroy ()
    {
        for (std::map<std::string, Atlas*>::iterator it = m_Atlases.begin (); it != m_Atlases.end (); ++it)
        {
            SAFE_DELETE (it->second);
        };

        return Lifecycle::Destroy ();
    }

    //--------------------------------------------------------------------------------------------------

    Atlas* AtlasManager::LoadFromFile (const std::string& path)
    {
        std::string fileName = GetBaseName (path);
        Atlas* atlas = GetAtlas (fileName);

        if (!atlas)
        {
            atlas = new Atlas (path);

            m_Atlases.insert (std::make_pair (fileName, atlas));
        }

        return atlas;
    }

    //--------------------------------------------------------------------------------------------------

    Atlas* AtlasManager::GetAtlas (const std::string& name)
    {
        if (m_Atlases.find (name) != m_Atlases.end ())
        {
            return m_Atlases[name];
        }

        return nullptr;
    }

    //--------------------------------------------------------------------------------------------------

    std::map<std::string, Atlas*>& AtlasManager::GetAtlases () { return m_Atlases; }

    //--------------------------------------------------------------------------------------------------
}
