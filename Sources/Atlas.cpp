// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "Atlas.h"

#include <experimental/filesystem>

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    Atlas::Atlas () {}

    //--------------------------------------------------------------------------------------------------

    void Atlas::LoadFromFile (const std::string& path)
    {
        m_Path = path;

        std::ifstream packFile (m_Path);
        std::string line = "";
        int lineCounter = 0;

        while (getline (packFile, line))
        {
            ++lineCounter;

            if (line == "")
            {
                continue;
            }

            if (lineCounter == 2)
            {
                std::experimental::filesystem::path p{ path };
                m_Image = al_load_bitmap ((p.parent_path ().string () + "/" + line).c_str ());
            }
        }

        m_Regions.insert (std::make_pair ("MAIN", Rect{ { 275, 259 }, { 64, 64 } }));

        packFile.close ();
    }

    //--------------------------------------------------------------------------------------------------

    void Atlas::DrawRegion (const std::string& name, int x, int y)
    {
        if (m_Regions.find (name) != m_Regions.end ())
        {
            Rect r = m_Regions[name];
            al_draw_bitmap_region (m_Image, r.TopLeft.X, r.TopLeft.Y, r.BottomRight.Width, r.BottomRight.Height, x, y, 0);
        }
    }

    //--------------------------------------------------------------------------------------------------
}
