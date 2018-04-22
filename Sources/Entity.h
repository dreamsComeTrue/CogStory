// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __ENTITY_H__
#define __ENTITY_H__

#include "Transformable.h"

namespace aga
{
    class SceneManager;

    class Entity : public Transformable
    {
    public:
        Entity (SceneManager* sceneManager);

        void SetCheckOverlap (bool check);
        bool IsCheckOverlap ();

        int ID = 0;
        std::string Name;
        int ZOrder = 0;
        int RenderID = 0;

        virtual std::string GetTypeName () = 0;

        static int GetNextID () { return ++GlobalID; }

        static bool CompareByZOrder (const Entity* a, const Entity* b) { return a->ZOrder < b->ZOrder; }

        static int GlobalID;

    protected:
        void CheckOverlap ();
        virtual void BeginOverlap (Entity* entity) {}
        virtual void EndOverlap (Entity* entity) {}

    protected:
        SceneManager* m_SceneManager;
        bool m_CheckOverlap;
        std::vector<Entity*> m_OverlapedEntities;
    };
}

#endif //   __ENTITY_H__
