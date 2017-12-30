// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "QuadTree.h"
#include "Entity.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    QuadTreeNode::QuadTreeNode (Rect bounds, float quadSize, QuadTreeNode* parent)
      : m_Bounds (bounds)
      , m_QuadSize (quadSize)
      , m_Parent (parent)
      , m_TopLeftTree (nullptr)
      , m_TopRightTree (nullptr)
      , m_BottomLeftTree (nullptr)
      , m_BottomRightTree (nullptr)
      , m_IsDivided (false)
    {
    }

    //--------------------------------------------------------------------------------------------------

    QuadTreeNode::~QuadTreeNode ()
    {
        for (int i = 0; i < m_Data.size (); ++i)
        {
            SAFE_DELETE (m_Data[i]);
        }

        m_Data.clear ();

        if (m_TopLeftTree)
        {
            m_TopLeftTree->~QuadTreeNode ();
            // SAFE_DELETE (m_TopLeftTree);
        }

        if (m_TopRightTree)
        {
            m_TopRightTree->~QuadTreeNode ();
            //  SAFE_DELETE (m_TopRightTree);
        }

        if (m_BottomLeftTree)
        {
            m_BottomLeftTree->~QuadTreeNode ();
            //   SAFE_DELETE (m_BottomLeftTree);
        }

        if (m_BottomRightTree)
        {
            m_BottomRightTree->~QuadTreeNode ();
            // SAFE_DELETE (m_BottomRightTree);
        }
    }

    //--------------------------------------------------------------------------------------------------

    // Insert a node into the quadtree
    void QuadTreeNode::Insert (Entity* node)
    {
        if (!node)
        {
            return;
        }

        // Current quad cannot contain it
        if (!InBoundary (node->Bounds.GetPos ()))
        {
            return;
        }

        // We are at a quad of unit area
        // We cannot subdivide this quad further
        if (std::abs (m_Bounds.GetTopLeft ().X - m_Bounds.GetBottomRight ().X) <= m_QuadSize &&
            std::abs (m_Bounds.GetTopLeft ().Y - m_Bounds.GetBottomRight ().Y) <= m_QuadSize)
        {
            QuadTreeData* data = new QuadTreeData (node, this);
            m_Data.push_back (data);

            return;
        }

        if ((m_Bounds.GetTopLeft ().X + m_Bounds.GetBottomRight ().X) / 2 >= node->Bounds.GetPos ().X)
        {
            if ((m_Bounds.GetTopLeft ().Y + m_Bounds.GetBottomRight ().Y) / 2 >= node->Bounds.GetPos ().Y)
            {
                if (!m_TopLeftTree)
                {
                    m_TopLeftTree =
                      new QuadTreeNode (Rect ({ m_Bounds.GetTopLeft ().X, m_Bounds.GetTopLeft ().Y },
                                              { (m_Bounds.GetTopLeft ().X + m_Bounds.GetBottomRight ().X) / 2,
                                                (m_Bounds.GetTopLeft ().Y + m_Bounds.GetBottomRight ().Y) / 2 }),
                                        m_QuadSize,
                                        this);
                }

                m_TopLeftTree->Insert (node);
            }
            else
            {
                if (!m_BottomLeftTree)
                {
                    m_BottomLeftTree = new QuadTreeNode (
                      Rect ({ m_Bounds.GetTopLeft ().X, (m_Bounds.GetTopLeft ().Y + m_Bounds.GetBottomRight ().Y) / 2 },
                            { (m_Bounds.GetTopLeft ().X + m_Bounds.GetBottomRight ().X) / 2,
                              m_Bounds.GetBottomRight ().Y }),
                      m_QuadSize,
                      this);
                }

                m_BottomLeftTree->Insert (node);
            }
        }
        else
        {
            if ((m_Bounds.GetTopLeft ().Y + m_Bounds.GetBottomRight ().Y) / 2 >= node->Bounds.GetPos ().Y)
            {
                if (!m_TopRightTree)
                {
                    m_TopRightTree = new QuadTreeNode (
                      Rect ({ (m_Bounds.GetTopLeft ().X + m_Bounds.GetBottomRight ().X) / 2, m_Bounds.GetTopLeft ().Y },
                            { m_Bounds.GetBottomRight ().X,
                              (m_Bounds.GetTopLeft ().Y + m_Bounds.GetBottomRight ().Y) / 2 }),
                      m_QuadSize,
                      this);
                }

                m_TopRightTree->Insert (node);
            }
            else
            {
                if (!m_BottomRightTree)
                {
                    m_BottomRightTree =
                      new QuadTreeNode (Rect ({ (m_Bounds.GetTopLeft ().X + m_Bounds.GetBottomRight ().X) / 2,
                                                (m_Bounds.GetTopLeft ().Y + m_Bounds.GetBottomRight ().Y) / 2 },
                                              { m_Bounds.GetBottomRight ().X, m_Bounds.GetBottomRight ().Y }),
                                        m_QuadSize,
                                        this);
                }

                m_BottomRightTree->Insert (node);
            }
        }

        m_IsDivided = true;
    }

    //--------------------------------------------------------------------------------------------------

    void QuadTreeNode::Remove (Entity* node)
    {
        QuadTreeData* foundNode = Search (node);

        if (foundNode)
        {
            std::vector<QuadTreeData*>& data = foundNode->Container->m_Data;

            for (int i = 0; i < data.size (); ++i)
            {
                if (data[i]->EntityData->ID == node->ID)
                {
                    SAFE_DELETE (foundNode);
                    data.erase (data.begin () + i);

                    break;
                }
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

        if ((m_Bounds.GetTopLeft ().X + m_Bounds.GetBottomRight ().X) / 2 >= p.X)
        {
            if ((m_Bounds.GetTopLeft ().Y + m_Bounds.GetBottomRight ().Y) / 2 >= p.Y)
            {
                if (m_TopLeftTree)
                {
                    return m_TopLeftTree->Search (p);
                }
            }
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
            if ((m_Bounds.GetTopLeft ().Y + m_Bounds.GetBottomRight ().Y) / 2 >= p.Y)
            {
                if (m_TopRightTree)
                {
                    return m_TopRightTree->Search (p);
                }
            }
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

    QuadTreeData* QuadTreeNode::Search (Entity* entity)
    {
        for (QuadTreeData* data : m_Data)
        {
            if (data->EntityData->ID == entity->ID)
            {
                return data;
            }
        }

        QuadTreeData* result = nullptr;

        if (m_TopLeftTree)
        {
            result = m_TopLeftTree->Search (entity);
        }

        if (!result && m_TopRightTree)
        {
            result = m_TopRightTree->Search (entity);
        }

        if (!result && m_BottomLeftTree)
        {
            result = m_BottomLeftTree->Search (entity);
        }

        if (!result && m_BottomRightTree)
        {
            result = m_BottomRightTree->Search (entity);
        }

        return result;
    }

    //--------------------------------------------------------------------------------------------------

    Rect QuadTreeNode::GetBounds () { return m_Bounds; }

    //--------------------------------------------------------------------------------------------------

    std::vector<QuadTreeData*>& QuadTreeNode::GetData () { return m_Data; }

    //--------------------------------------------------------------------------------------------------

    std::vector<Entity*> QuadTreeNode::GetEntitiesWithinRect (Rect rect)
    {
        std::vector<Entity*> result;

        if (Intersect (m_Bounds, rect))
        {
            for (int i = 0; i < m_Data.size (); ++i)
            {
                Entity* ent = m_Data[i]->EntityData;

                if (Intersect (ent->Bounds, rect))
                {
                    result.push_back (ent);
                }
            }

            if (m_TopLeftTree)
            {
                std::vector<Entity*> childNodes = m_TopLeftTree->GetEntitiesWithinRect (rect);
                result.insert (result.end (), childNodes.begin (), childNodes.end ());
            }

            if (m_TopRightTree)
            {
                std::vector<Entity*> childNodes = m_TopRightTree->GetEntitiesWithinRect (rect);
                result.insert (result.end (), childNodes.begin (), childNodes.end ());
            }

            if (m_BottomLeftTree)
            {
                std::vector<Entity*> childNodes = m_BottomLeftTree->GetEntitiesWithinRect (rect);
                result.insert (result.end (), childNodes.begin (), childNodes.end ());
            }

            if (m_BottomRightTree)
            {
                std::vector<Entity*> childNodes = m_BottomRightTree->GetEntitiesWithinRect (rect);
                result.insert (result.end (), childNodes.begin (), childNodes.end ());
            }
        }

        return result;
    }

    //--------------------------------------------------------------------------------------------------

    void QuadTreeNode::UpdateStructures ()
    {
        if (m_IsDivided)
        {
            if (m_TopLeftTree)
            {
                m_TopLeftTree->UpdateStructures ();
            }

            if (m_TopRightTree)
            {
                m_TopRightTree->UpdateStructures ();
            }

            if (m_BottomLeftTree)
            {
                m_BottomLeftTree->UpdateStructures ();
            }

            if (m_BottomRightTree)
            {
                m_BottomRightTree->UpdateStructures ();
            }
        }
        else if (m_Data.empty () && m_Parent)
        {
            if (this == m_Parent->m_TopLeftTree)
            {
                SAFE_DELETE (m_Parent->m_TopLeftTree);
            }
            if (this == m_Parent->m_TopRightTree)
            {
                SAFE_DELETE (m_Parent->m_TopRightTree);
            }
            if (this == m_Parent->m_BottomLeftTree)
            {
                SAFE_DELETE (m_Parent->m_BottomLeftTree);
            }
            if (this == m_Parent->m_BottomRightTree)
            {
                SAFE_DELETE (m_Parent->m_BottomRightTree);
            }

            if (!m_Parent->m_TopLeftTree && !m_Parent->m_TopRightTree && !m_Parent->m_BottomLeftTree &&
                !m_Parent->m_BottomRightTree)
            {
                m_Parent->m_IsDivided = false;
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    QuadTreeNode* QuadTreeNode::GetTopLeftTree () { return m_TopLeftTree; }

    //--------------------------------------------------------------------------------------------------

    QuadTreeNode* QuadTreeNode::GetTopRightTree () { return m_TopRightTree; }

    //--------------------------------------------------------------------------------------------------

    QuadTreeNode* QuadTreeNode::GetBottomLeftTree () { return m_BottomLeftTree; }

    //--------------------------------------------------------------------------------------------------

    QuadTreeNode* QuadTreeNode::GetBottomRightTree () { return m_BottomRightTree; }

    //--------------------------------------------------------------------------------------------------

    // Check if current quadtree contains the point
    bool QuadTreeNode::InBoundary (Point p)
    {
        return (p.X >= m_Bounds.GetTopLeft ().X && p.X <= m_Bounds.GetBottomRight ().X &&
                p.Y >= m_Bounds.GetTopLeft ().Y && p.Y <= m_Bounds.GetBottomRight ().Y);
    }

    //--------------------------------------------------------------------------------------------------
}
