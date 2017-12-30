// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "EditorActorMode.h"
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

    bool EditorActorMode::AddOrUpdateActor (const std::string& oldName) { return false; }

    //--------------------------------------------------------------------------------------------------

    void EditorActorMode::RemoveActor (const std::string& name) {}

    //--------------------------------------------------------------------------------------------------

    void EditorActorMode::Clear () {}

    //--------------------------------------------------------------------------------------------------
}
