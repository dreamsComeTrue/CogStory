// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __QUAD_TREE_H__
#define __QUAD_TREE_H__

#include "Common.h"

namespace aga
{
    class Entity;

    // The objects that we want stored in the quadtree
    struct QuadTreeData
    {
        Point Pos;
        Entity* EntityData;

        QuadTreeData (Point pos, Entity* entityData)
          : Pos (pos)
          , EntityData (entityData)
        {
        }
    };

    // The main quadtree class
    class QuadTreeNode
    {
    public:
        QuadTreeNode (Rect size);

        void Insert (QuadTreeData*);
        QuadTreeData* Search (Point);
        bool InBoundary (Point);

        Rect GetBounds ();

    private:
        // Hold details of the boundary of this node
        Rect m_Bounds;

        // Contains details of node
        std::vector<QuadTreeData*> m_Data;

        // Children of this tree
        QuadTreeNode* m_TopLeftTree;
        QuadTreeNode* m_TopRightTree;
        QuadTreeNode* m_BottomLeftTree;
        QuadTreeNode* m_BottomRightTree;
    };
}

#endif //   __QUAD_TREE_H__
