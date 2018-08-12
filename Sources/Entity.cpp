// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "Entity.h"
#include "Scene.h"
#include "SceneManager.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    Entity::Entity (SceneManager* sceneManager)
        : m_SceneManager (sceneManager)
        , m_CheckOverlap (false)
    {
    }

    //--------------------------------------------------------------------------------------------------

    void Entity::SetCheckOverlap (bool check) { m_CheckOverlap = check; }

    //--------------------------------------------------------------------------------------------------

    bool Entity::IsCheckOverlap () { return m_CheckOverlap; }

    //--------------------------------------------------------------------------------------------------

    void Entity::CheckOverlap ()
    {
        if (!m_CheckOverlap)
        {
            return;
        }

        Scene* activeScene = m_SceneManager->GetActiveScene ();

        Rect myBounds = activeScene->GetRenderBounds (this);
        std::vector<Entity*> visibleEntites = activeScene->RecomputeVisibleEntities (true);

        //  Special-case entity :)
        visibleEntites.push_back (m_SceneManager->GetPlayer ());

        for (Entity* ent : visibleEntites)
        {
            if (ent != this && ent->IsCheckOverlap ())
            {
                Rect otherBounds = activeScene->GetRenderBounds (ent);

                if (Intersect (myBounds, otherBounds))
                {
                    bool found = false;

                    for (Entity* saved : m_OverlapedEntities)
                    {
                        if (saved == ent)
                        {
                            found = true;
                            break;
                        }
                    }

                    if (!found)
                    {
                        m_OverlapedEntities.push_back (ent);

                        BeginOverlap (ent);
                    }
                }
                else
                {
                    for (std::vector<Entity*>::iterator it = m_OverlapedEntities.begin ();
                         it != m_OverlapedEntities.end (); ++it)
                    {
                        if (*it == ent)
                        {
                            EndOverlap (ent);

                            m_OverlapedEntities.erase (it);
                            break;
                        }
                    }
                }
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    bool Entity::IsOverlaping (Entity* entity)
    {
        for (Entity* ent : m_OverlapedEntities)
        {
            if (ent == entity)
            {
                return true;
            }
        }

        return false;
    }

    //--------------------------------------------------------------------------------------------------

    float Entity::GetAngleWith (Entity* ent)
    {
        Point thisHalf = this->Bounds.GetHalfSize ();
        Point entHalf = ent->Bounds.GetHalfSize ();
        float dx = (this->Bounds.Pos.X + thisHalf.Width) - (ent->Bounds.Pos.X + entHalf.Width);
        float dy = (this->Bounds.Pos.Y + thisHalf.Height) - (ent->Bounds.Pos.Y + entHalf.Height);

        return ToPositiveAngle (RadiansToDegrees (std::atan2 (dy, dx)));
    }

    //--------------------------------------------------------------------------------------------------
}
