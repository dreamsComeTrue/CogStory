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

    class Atlas
    {
    public:
        Atlas ();

        void LoadFromFile (const std::string& path);
        void DrawRegion (const std::string& name, int x, int y);
        void DrawScaledRegion (const std::string& name, int x, int y, float scaleX, float scaleY);
        ALLEGRO_BITMAP* GetImage ();
        std::vector<AtlasRegion> GetRegions ();

    private:
        ALLEGRO_BITMAP* m_Image;
        std::string m_Path;
        std::map<std::string, AtlasRegion> m_Regions;
        std::vector<AtlasRegion> m_RegionsVector;
    };
}

#endif //   __ATLAS_H__
