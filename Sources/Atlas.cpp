// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "Atlas.h"
#include "Screen.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    Atlas::Atlas (const std::string& path)
        : m_Image (nullptr)
    {
        LoadFromFile (path);
    }

    //--------------------------------------------------------------------------------------------------

    Atlas::~Atlas ()
    {
        if (m_Image)
        {
            al_destroy_bitmap (m_Image);
            m_Image = nullptr;
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Atlas::LoadFromFile (const std::string& path)
    {
        m_Path = path;

        std::ifstream packFile (m_Path);
        std::string line = "";

        getline (packFile, line); //  empty
        getline (packFile, line); //  image file

        m_Name = GetBaseName (path);

        std::string fileName = GetDirectory (path) + "/" + line;
        m_Image = al_load_bitmap (fileName.c_str ());

        getline (packFile, line); //  skip - size
        getline (packFile, line); //  skip - format
        getline (packFile, line); //  skip - filter
        getline (packFile, line); //  skip - repeat

        std::string name;
        std::string xy;
        std::string size;

        while (packFile)
        {
            getline (packFile, line);

            //  We can parse - we have a name
            if (!StartsWith (line, "  "))
            {
                name = line;

                std::streampos oldpos;

                do
                {
                    oldpos = packFile.tellg ();
                    getline (packFile, line);

                    if (StartsWith (line, "  xy:"))
                    {
                        xy = line;
                    }
                    else if (StartsWith (line, "  size:"))
                    {
                        size = line;
                    }
                } while (StartsWith (line, "  "));

                packFile.seekg (oldpos);

                std::vector<std::string> xyData = SplitString (xy.substr (xy.find (":") + 1), ',');
                int x = atoi (xyData[0].c_str ());
                int y = atoi (xyData[1].c_str ());

                std::vector<std::string> sizeData = SplitString (size.substr (size.find (":") + 1), ',');
                int width = atoi (sizeData[0].c_str ());
                int height = atoi (sizeData[1].c_str ());

                AtlasRegion region;
                region.Bounds = Rect{ { x, y }, { x + width, y + height } };
                region.Name = name;

                m_Regions.insert (std::make_pair (name, region));
                m_RegionsVector.push_back (region);
            }
        }

        packFile.close ();
    }

    //--------------------------------------------------------------------------------------------------

    void Atlas::DrawRegion (const std::string& name, float x, float y, float scaleX, float scaleY, float rotation,
                            bool offsetByCenter, ALLEGRO_COLOR color)
    {
        if (m_Regions.find (name) != m_Regions.end ())
        {
            Rect r = m_Regions[name].Bounds;

            if (offsetByCenter)
            {
                x += r.GetHalfSize ().Width;
                y += r.GetHalfSize ().Height;
            }

            al_draw_tinted_scaled_rotated_bitmap_region (m_Image, r.GetPos ().X, r.GetPos ().Y, r.GetSize ().Width,
                                                         r.GetSize ().Height, color, r.GetHalfSize ().Width,
                                                         r.GetHalfSize ().Height, x, y, scaleX, scaleY, rotation, 0);
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Atlas::DrawRegion (float sourceX, float sourceY, float sourceWidth, float sourceHeight, float x, float y,
                            float scaleX, float scaleY, float rotation, bool offsetByCenter, ALLEGRO_COLOR color)
    {
        if (offsetByCenter)
        {
            x += sourceWidth * 0.5f;
            y += sourceHeight * 0.5f;
        }

        al_draw_tinted_scaled_rotated_bitmap_region (m_Image, sourceX, sourceY, sourceWidth, sourceHeight, color,
                                                     sourceWidth * 0.5f, sourceHeight * 0.5f, x, y, scaleX, scaleY,
                                                     rotation, 0);
    }

    //--------------------------------------------------------------------------------------------------

    ALLEGRO_BITMAP* Atlas::GetImage () { return m_Image; }

    //--------------------------------------------------------------------------------------------------

    AtlasRegion& Atlas::GetRegion (const std::string& name) { return m_Regions[name]; }

    //--------------------------------------------------------------------------------------------------

    std::vector<AtlasRegion>& Atlas::GetRegions () { return m_RegionsVector; }

    //--------------------------------------------------------------------------------------------------

    std::string Atlas::GetName () { return m_Name; }

    //--------------------------------------------------------------------------------------------------

    std::string Atlas::GetPath () { return m_Path; }

    //--------------------------------------------------------------------------------------------------
}
