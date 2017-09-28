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
        friend class AtlasManager;

    public:
        Atlas (Screen* screen);
        ~Atlas ();

        void DrawRegion (const std::string& name, int x, int y);
        void DrawRegion (const std::string& name, int x, int y, float scaleX, float scaleY, float rotation);
        SDL_Texture* GetImage ();
        AtlasRegion& GetRegion (const std::string& name);
        std::vector<AtlasRegion>& GetRegions ();

        std::string GetName ();

    private:
        void LoadFromFile (const std::string& path);

    private:
        Screen* m_Screen;
        SDL_Texture* m_Image;
        std::string m_Path;
        std::string m_Name;
        std::map<std::string, AtlasRegion> m_Regions;
        std::vector<AtlasRegion> m_RegionsVector;
    };
}

#endif //   __ATLAS_H__
