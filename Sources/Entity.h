// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __ENTITY_H__
#define __ENTITY_H__

#include "Transformable.h"

namespace aga
{
    class Entity : public Transformable
    {
    public:
        int ID = 0;
        std::string Name;
        int ZOrder = 0;
        int RenderID = 0;

        virtual std::string GetTypeName () = 0;

        static int GetNextID () { return ++GlobalID; }

        static bool CompareByZOrder (const Entity* a, const Entity* b) { return a->ZOrder < b->ZOrder; }

        static int GlobalID;
    };
}

#endif //   __ENTITY_H__
