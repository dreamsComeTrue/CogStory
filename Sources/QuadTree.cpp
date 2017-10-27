// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "QuadTree.h"
#include "Entity.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    QuadTreeNode::QuadTreeNode (Rect size)
      : m_Bounds (size)
      , m_TopLeftTree (nullptr)
      , m_TopRightTree (nullptr)
      , m_BottomLeftTree (nullptr)
      , m_BottomRightTree (nullptr)
    {
    }

    //--------------------------------------------------------------------------------------------------

    // Insert a node into the quadtree
    void QuadTreeNode::Insert (QuadTreeData* node)
    {
        if (!node)
        {
            return;
        }

        // Current quad cannot contain it
        if (!InBoundary (node->Pos))
        {
            return;
        }

        // We are at a quad of unit area
        // We cannot subdivide this quad further
        if (std::abs (m_Bounds.Dim.TopLeft.X - m_Bounds.Dim.BottomRight.X) <= 1 &&
            std::abs (m_Bounds.Dim.TopLeft.Y - m_Bounds.Dim.BottomRight.Y) <= 1)
        {
            m_Data.push_back (node);

            return;
        }

        if ((m_Bounds.Dim.TopLeft.X + m_Bounds.Dim.BottomRight.X) / 2 >= node->Pos.X)
        {
            // Indicates topLeftTree
            if ((m_Bounds.Dim.TopLeft.Y + m_Bounds.Dim.BottomRight.Y) / 2 >= node->Pos.Y)
            {
                if (m_TopLeftTree == nullptr)
                {
                    m_TopLeftTree = new QuadTreeNode (Rect ({ m_Bounds.Dim.TopLeft.X, m_Bounds.Dim.TopLeft.Y },
                                                            { (m_Bounds.Dim.TopLeft.X + m_Bounds.Dim.BottomRight.X) / 2,
                                                              (m_Bounds.Dim.TopLeft.Y + m_Bounds.Dim.BottomRight.Y) / 2 }));
                }

                m_TopLeftTree->Insert (node);
            }

            // Indicates botLeftTree
            else
            {
                if (m_BottomLeftTree == nullptr)
                {
                    m_BottomLeftTree =
                      new QuadTreeNode (Rect ({ m_Bounds.Dim.TopLeft.X, (m_Bounds.Dim.TopLeft.Y + m_Bounds.Dim.BottomRight.Y) / 2 },
                                              { (m_Bounds.Dim.TopLeft.X + m_Bounds.Dim.BottomRight.X) / 2, m_Bounds.Dim.BottomRight.Y }));
                }

                m_BottomLeftTree->Insert (node);
            }
        }
        else
        {
            // Indicates topRightTree
            if ((m_Bounds.Dim.TopLeft.Y + m_Bounds.Dim.BottomRight.Y) / 2 >= node->Pos.Y)
            {
                if (m_TopRightTree == nullptr)
                {
                    m_TopRightTree =
                      new QuadTreeNode (Rect ({ (m_Bounds.Dim.TopLeft.X + m_Bounds.Dim.BottomRight.X) / 2, m_Bounds.Dim.TopLeft.Y },
                                              { m_Bounds.Dim.BottomRight.X, (m_Bounds.Dim.TopLeft.Y + m_Bounds.Dim.BottomRight.Y) / 2 }));
                }

                m_TopRightTree->Insert (node);
            }

            // Indicates botRightTree
            else
            {
                if (m_BottomRightTree == nullptr)
                {
                    m_BottomRightTree = new QuadTreeNode (Rect ({ (m_Bounds.Dim.TopLeft.X + m_Bounds.Dim.BottomRight.X) / 2,
                                                                  (m_Bounds.Dim.TopLeft.Y + m_Bounds.Dim.BottomRight.Y) / 2 },
                                                                { m_Bounds.Dim.BottomRight.X, m_Bounds.Dim.BottomRight.Y }));
                }

                m_BottomRightTree->Insert (node);
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    // Find a node in a quadtree
    QuadTreeData* QuadTreeNode::Search (Point p)
    {
        // Current quad cannot contain it
        if (!InBoundary (p))
        {
            return nullptr;
        }

        if ((m_Bounds.Dim.TopLeft.X + m_Bounds.Dim.BottomRight.X) / 2 >= p.X)
        {
            // Indicates topLeftTree
            if ((m_Bounds.Dim.TopLeft.Y + m_Bounds.Dim.BottomRight.Y) / 2 >= p.Y)
            {
                if (m_TopLeftTree)
                {
                    return m_TopLeftTree->Search (p);
                }
            }

            // Indicates botLeftTree
            else
            {
                if (m_BottomLeftTree)
                {
                    return m_BottomLeftTree->Search (p);
                }
            }
        }
        else
        {
            // Indicates topRightTree
            if ((m_Bounds.Dim.TopLeft.Y + m_Bounds.Dim.BottomRight.Y) / 2 >= p.Y)
            {
                if (m_TopRightTree)
                {
                    return m_TopRightTree->Search (p);
                }
            }

            // Indicates botRightTree
            else
            {
                if (m_BottomRightTree)
                {
                    return m_BottomRightTree->Search (p);
                }
            }
        }

        for (QuadTreeData* data : m_Data)
        {
            if (InsideRect (p, data->EntityData->Bounds))
            {
                return data;
            }
        }

        return nullptr;
    }

    //--------------------------------------------------------------------------------------------------

    Rect QuadTreeNode::GetBounds () { return m_Bounds; }

    //--------------------------------------------------------------------------------------------------

    // Check if current quadtree contains the point
    bool QuadTreeNode::InBoundary (Point p)
    {
        return (p.X >= m_Bounds.Dim.TopLeft.X && p.X <= m_Bounds.Dim.BottomRight.X && p.Y >= m_Bounds.Dim.TopLeft.Y &&
                p.Y <= m_Bounds.Dim.BottomRight.Y);
    }

    //--------------------------------------------------------------------------------------------------
}
