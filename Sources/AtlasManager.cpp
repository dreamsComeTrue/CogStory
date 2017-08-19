// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "AtlasManager.h"
#include "Atlas.h"

#include <experimental/filesystem>

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

        Lifecycle::Destroy ();
    }

    //--------------------------------------------------------------------------------------------------

    bool AtlasManager::Initialize () { Lifecycle::Initialize (); }

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
        std::experimental::filesystem::path p{ path };
        std::string fileName = p.stem ().c_str ();

        Atlas* atlas = GetAtlas (fileName);

        if (!atlas)
        {
            atlas = new Atlas ();
            atlas->LoadFromFile (path);

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
}
