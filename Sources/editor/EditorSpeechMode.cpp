// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "EditorSpeechMode.h"
#include "Editor.h"
#include "MainLoop.h"
#include "SceneManager.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    EditorSpeechMode::EditorSpeechMode (Editor* editor)
      : m_Editor (editor)
    {
    }

    //--------------------------------------------------------------------------------------------------

    EditorSpeechMode::~EditorSpeechMode () {}

    //--------------------------------------------------------------------------------------------------

    void EditorSpeechMode::AddOrUpdateSpeech ()
    {
        if (strlen (m_Speech.Name.c_str ()) > 0)
        {
            if (m_Editor->m_MainLoop->GetSceneManager ().GetActiveScene ()->GetSpeech (m_Speech.Name))
            {
                m_Editor->m_MainLoop->GetSceneManager ().GetActiveScene ()->RemoveSpeech (m_Speech.Name);
            }

            m_Editor->m_MainLoop->GetSceneManager ().GetActiveScene ()->AddSpeech (m_Speech.Name, m_Speech);
        }
    }

    //--------------------------------------------------------------------------------------------------

    void EditorSpeechMode::RemoveSpeech (const std::string& name)
    {
        m_Editor->m_MainLoop->GetSceneManager ().GetActiveScene ()->RemoveSpeech (name);
    }

    //--------------------------------------------------------------------------------------------------

    void EditorSpeechMode::Clear ()
    {
        std::string shortS (50, 0);
        std::string longS (1000, 0);
        m_Speech.Name = shortS;
        m_Speech.Text.clear ();
        m_Speech.Outcomes.clear ();

        m_Speech.Text.insert (std::make_pair (LANG_EN, longS));
        m_Speech.Text.insert (std::make_pair (LANG_PL, longS));
    }

    //--------------------------------------------------------------------------------------------------
}
