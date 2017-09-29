// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __ATLAS_MANAGER_H__
#define __ATLAS_MANAGER_H__

#include "Common.h"

namespace aga
{
    class Atlas;
    class Screen;

    class AtlasManager : public Lifecycle
    {
    public:
        AtlasManager ();
        ~AtlasManager ();

        bool Initialize ();
        bool Destroy ();

        Atlas* LoadFromFile (const std::string& path);
        Atlas* GetAtlas (const std::string& name);

    private:
        std::map<std::string, Atlas*> m_Atlases;
    };
}

#endif //   __ATLAS_MANAGER_H__
