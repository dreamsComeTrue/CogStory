// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __ATLAS_H__
#define __ATLAS_H__

#include "Common.h"

namespace aga
{
    struct AtlasRegion
    {
        Rect Bounds;
        std::string Name;
    };

    class AtlasManager;

    class Atlas
    {
        friend class AtlasManager;

    public:
        Atlas ();

        void DrawRegion (const std::string& name, int x, int y);
        void DrawRegion (const std::string& name, int x, int y, float scaleX, float scaleY, float rotation);
        ALLEGRO_BITMAP* GetImage ();
        std::vector<AtlasRegion>& GetRegions ();

        std::string GetName ();

    private:
        void LoadFromFile (const std::string& path);

    private:
        ALLEGRO_BITMAP* m_Image;
        std::string m_Path;
        std::string m_Name;
        std::map<std::string, AtlasRegion> m_Regions;
        std::vector<AtlasRegion> m_RegionsVector;
    };
}

#endif //   __ATLAS_H__
