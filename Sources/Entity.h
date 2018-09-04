// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __ENTITY_H__
#define __ENTITY_H__

#include "Common.h"
#include "Transformable.h"

namespace aga
{
    class SceneManager;

    struct OverlapCallback
    {
        asIScriptFunction* BeginFunc = nullptr;
        asIScriptFunction* OverlappingFunc = nullptr;
        asIScriptFunction* EndFunc = nullptr;
    };

    class Entity : public Transformable
    {
    public:
        Entity (SceneManager* sceneManager);

        void SetCheckOverlap (bool check);
        bool IsCheckOverlap ();

        SceneManager* GetSceneManager () { return m_SceneManager; }
        bool IsOverlaping (Entity* entity);

        float GetAngleWith (Entity* ent);

        int ID = 0;
        std::string Name;
        int ZOrder = 0;
        int RenderID = 0;

        virtual std::string GetTypeName () = 0;

        void AddBeginOverlapCallback (asIScriptFunction* func);
        void AddOverlappingCallback (asIScriptFunction* func);
        void AddEndOverlapCallback (asIScriptFunction* func);
        void AddOverlapCallbacks (asIScriptFunction* begin, asIScriptFunction* update, asIScriptFunction* end);

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
        std::vector<OverlapCallback> m_OverlapCallbacks;
    };
}

#endif //   __ENTITY_H__
