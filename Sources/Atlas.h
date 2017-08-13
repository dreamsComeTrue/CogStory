// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __ATLAS_H__
#define __ATLAS_H__

#include "Common.h"

namespace aga
{
    class Atlas
    {
    public:
        Atlas ();

        void LoadFromFile (const std::string& path);
        void DrawRegion (const std::string& name, int x, int y);

    private:
        ALLEGRO_BITMAP* m_Image;
        std::string m_Path;
        std::map<std::string, Rect> m_Regions;
    };
}

#endif //   __ATLAS_H__
