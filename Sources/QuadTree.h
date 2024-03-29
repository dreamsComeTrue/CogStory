// Copyright 2017-2019 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __QUAD_TREE_H__
#define __QUAD_TREE_H__

#include "Common.h"

namespace aga
{
    class Entity;
    class QuadTreeNode;

    // The objects that we want stored in the quadtree
    struct QuadTreeData
    {
        QuadTreeNode* Container;
        Entity* EntityData;

        QuadTreeData (Entity* entityData, QuadTreeNode* container)
            : Container (container)
            , EntityData (entityData)
        {
        }
    };

    // The main quadtree class
    class QuadTreeNode
    {
    public:
        QuadTreeNode (Rect bounds, float quadSize = 128, QuadTreeNode* parent = nullptr);
        ~QuadTreeNode ();

        void Insert (Entity*);
        void Remove (Entity*);
        QuadTreeData* Search (Point);
        QuadTreeData* Search (Entity*);
        bool InBoundary (Point);

        QuadTreeNode* GetTopLeftTree ();
        QuadTreeNode* GetTopRightTree ();
        QuadTreeNode* GetBottomLeftTree ();
        QuadTreeNode* GetBottomRightTree ();

        Rect GetBounds ();
        std::vector<QuadTreeData>& GetData ();

        void UpdateStructures ();

        std::vector<Entity*> GetEntitiesWithinRect (Rect rect);

    private:
        Rect m_Bounds;
        bool m_IsDivided;
        std::vector<QuadTreeData> m_Data;
        float m_QuadSize;

        QuadTreeNode* m_Parent;

        QuadTreeNode* m_TopLeftTree;
        QuadTreeNode* m_TopRightTree;
        QuadTreeNode* m_BottomLeftTree;
        QuadTreeNode* m_BottomRightTree;
    };
}

#endif //   __QUAD_TREE_H__
