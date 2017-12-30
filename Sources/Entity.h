// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __ENTITY_H__
#define __ENTITY_H__

#include "Transformable.h"

namespace aga
{
    static int GlobalID = 0;

    class Entity : public Transformable
    {
    public:
        int ID = 0;
        std::string Name;
        int ZOrder = 0;
        int RenderID = 0;

        static int GetNextID () { return GlobalID++; }

        static bool CompareByZOrder (const Entity* a, const Entity* b) { return a->ZOrder < b->ZOrder; }
    };
}

#endif //   __ENTITY_H__
