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

        Rect myBounds = m_SceneManager->GetActiveScene ()->GetRenderBounds (this);
        std::vector<Entity*> visibleEntites = m_SceneManager->GetActiveScene ()->GetVisibleEntities ();

        //  Special-case entity :)
        visibleEntites.push_back (&m_SceneManager->GetPlayer ());

        for (Entity* ent : visibleEntites)
        {
            if (ent != this && ent->IsCheckOverlap ())
            {
                Rect otherBounds = m_SceneManager->GetActiveScene ()->GetRenderBounds (ent);

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
}
