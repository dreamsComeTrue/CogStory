// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "EditorActorMode.h"
#include "ActorFactory.h"
#include "Editor.h"
#include "MainLoop.h"
#include "SceneManager.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    EditorActorMode::EditorActorMode (Editor* editor)
      : m_Editor (editor)
    {
    }

    //--------------------------------------------------------------------------------------------------

    EditorActorMode::~EditorActorMode () {}

    //--------------------------------------------------------------------------------------------------

    bool EditorActorMode::AddOrUpdateActor (const std::string& oldName, const std::string& actorType)
    {
        Actor* actor = m_Editor->m_MainLoop->GetSceneManager ().GetActiveScene ()->GetActor (oldName);

        if (actor)
        {
            m_Editor->m_MainLoop->GetSceneManager ().GetActiveScene ()->RemoveActor (oldName);
        }

        Actor* newActor = ActorFactory::GetActor (&m_Editor->m_MainLoop->GetSceneManager (), actorType);
        newActor->Name = oldName;

        m_Editor->m_MainLoop->GetSceneManager ().GetActiveScene ()->AddActor (oldName, newActor);

        m_Editor->m_EditorActorMode.Clear ();

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    void EditorActorMode::RemoveActor (const std::string& name)
    {
        m_Editor->m_MainLoop->GetSceneManager ().GetActiveScene ()->RemoveActor (name);
    }

    //--------------------------------------------------------------------------------------------------

    void EditorActorMode::Clear () {}

    //--------------------------------------------------------------------------------------------------
}
