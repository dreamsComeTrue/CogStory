// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __RESOURCES_H__
#define __RESOURCES_H__

#include <string>

namespace aga
{
    enum ResourceID
    {
        GFX_MENU_COG,
        GFX_TEXT_FRAME,
        GFX_PLAYER,
    };

    struct Resource
    {
        int ID;
        std::string Dir;
        std::string Name;
    };

    const std::string GetResourcePath (ResourceID id);
}

#endif //   __RESOURCES_H__
