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

    bool EditorActorMode::AddOrUpdateSpeech (const std::string& oldName)
    {
        if (strlen (m_Speech.Name.c_str ()) > 0)
        {
            std::string nameToFind = oldName != "" ? oldName : m_Speech.Name;
            SpeechData* speech = m_Editor->m_MainLoop->GetSceneManager ().GetActiveScene ()->GetSpeech (nameToFind);

            if (speech)
            {
                m_Editor->m_MainLoop->GetSceneManager ().GetActiveScene ()->RemoveSpeech (nameToFind);
            }

            m_Editor->m_MainLoop->GetSceneManager ().GetActiveScene ()->AddSpeech (m_Speech.Name, m_Speech);

            m_Editor->m_EditorSpeechMode.Clear ();

            return true;
        }

        return false;
    }

    //--------------------------------------------------------------------------------------------------

    void EditorActorMode::RemoveSpeech (const std::string& name)
    {
        m_Editor->m_MainLoop->GetSceneManager ().GetActiveScene ()->RemoveSpeech (name);
    }

    //--------------------------------------------------------------------------------------------------

    void EditorActorMode::Clear ()
    {
        m_Speech.Name = "";
        m_Speech.Text.clear ();
        m_Speech.Outcomes.clear ();
    }

    //--------------------------------------------------------------------------------------------------
}
