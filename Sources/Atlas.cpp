// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "Atlas.h"
#include "Screen.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    Atlas::Atlas (Screen* screen)
      : m_Screen (screen)
      , m_Image (nullptr)
    {
    }

    //--------------------------------------------------------------------------------------------------

    Atlas::~Atlas ()
    {
        if (m_Image)
        {
            SDL_DestroyTexture (m_Image);
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
        m_Image = m_Screen->LoadTexture (fileName);

        getline (packFile, line); //  skip
        getline (packFile, line); //  skip
        getline (packFile, line); //  skip

        std::string name;
        std::string xy;
        std::string size;

        while (packFile)
        {
            getline (packFile, name); //    name
            getline (packFile, line); //    rotate
            getline (packFile, xy);   //    xy
            getline (packFile, size); //    size
            getline (packFile, line); //    orig
            getline (packFile, line); //    offset
            getline (packFile, line); //    index

            std::vector<std::string> xyData = SplitString (xy.substr (xy.find (":") + 1), ',');
            int x = atoi (xyData[0].c_str ());
            int y = atoi (xyData[1].c_str ());

            std::vector<std::string> sizeData = SplitString (size.substr (size.find (":") + 1), ',');
            int width = atoi (sizeData[0].c_str ());
            int height = atoi (sizeData[1].c_str ());

            AtlasRegion region;
            region.Bounds = Rect{ { x, y }, { width, height } };
            region.Name = name;

            m_Regions.insert (std::make_pair (name, region));
            m_RegionsVector.push_back (region);
        }

        packFile.close ();
    }

    //--------------------------------------------------------------------------------------------------

    void Atlas::DrawRegion (const std::string& name, int x, int y)
    {
        if (m_Regions.find (name) != m_Regions.end ())
        {
            Rect r = m_Regions[name].Bounds;
            SDL_Rect srcRect = { r.TopLeft.X, r.TopLeft.Y, r.BottomRight.Width, r.BottomRight.Height };
            SDL_Rect dstRect = { x, y, r.BottomRight.Width, r.BottomRight.Height };

            SDL_RenderCopy (m_Screen->GetRenderer (), m_Image, &srcRect, &dstRect);
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Atlas::DrawRegion (const std::string& name, int x, int y, float scaleX, float scaleY, float rotation)
    {
        if (m_Regions.find (name) != m_Regions.end ())
        {
            Rect r = m_Regions[name].Bounds;
            SDL_Rect srcRect = { r.TopLeft.X, r.TopLeft.Y, r.BottomRight.Width, r.BottomRight.Height };
            SDL_Rect dstRect = { x, y, r.BottomRight.Width * scaleX, r.BottomRight.Height * scaleY };

            SDL_SetTextureColorMod (m_Image, 255, 255, 255);

            SDL_RenderCopyEx (m_Screen->GetRenderer (), m_Image, &srcRect, &dstRect, rotation, NULL, SDL_FLIP_NONE);
        }
    }

    //--------------------------------------------------------------------------------------------------

    SDL_Texture* Atlas::GetImage () { return m_Image; }

    //--------------------------------------------------------------------------------------------------

    AtlasRegion& Atlas::GetRegion (const std::string& name) { return m_Regions[name]; }

    //--------------------------------------------------------------------------------------------------

    std::vector<AtlasRegion>& Atlas::GetRegions () { return m_RegionsVector; }

    //--------------------------------------------------------------------------------------------------

    std::string Atlas::GetName () { return m_Name; }

    //--------------------------------------------------------------------------------------------------
}
