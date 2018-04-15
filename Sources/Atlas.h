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
    class Screen;

    class Atlas
    {
    public:
        Atlas (const std::string& path);
        ~Atlas ();

        void DrawRegion (const std::string& name, float x, float y, float scaleX, float scaleY, float rotation,
            bool offsetByCenter = true, ALLEGRO_COLOR color = COLOR_WHITE);
        void DrawRegion (float sourceX, float sourceY, float sourceWidth, float sourceHeight, float x, float y,
            float scaleX, float scaleY, float rotation, bool offsetByCenter = true, ALLEGRO_COLOR color = COLOR_WHITE);
        ALLEGRO_BITMAP* GetImage ();
        AtlasRegion& GetRegion (const std::string& name);
        std::vector<AtlasRegion>& GetRegions ();

        std::string GetName ();
        std::string GetPath ();

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
