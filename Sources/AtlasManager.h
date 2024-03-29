// Copyright 2017-2019 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

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
        virtual ~AtlasManager ();

        bool Initialize ();
        bool Destroy ();

        Atlas* LoadFromFile (const std::string& path);
        Atlas* GetAtlas (const std::string& name);
        std::map<std::string, Atlas*>& GetAtlases ();

    private:
        std::map<std::string, Atlas*> m_Atlases;
    };
}

#endif //   __ATLAS_MANAGER_H__
