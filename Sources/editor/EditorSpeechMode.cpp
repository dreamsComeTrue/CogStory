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

    bool EditorSpeechMode::AddOrUpdateSpeech (const std::string& oldName)
    {
        if (strlen (m_Speech.Name.c_str ()) > 0)
        {
            std::string nameToFind = oldName != "" ? oldName : m_Speech.Name;
            SpeechData* speech = m_Editor->m_MainLoop->GetSceneManager ().GetActiveScene ()->GetSpeech (nameToFind);

            if (speech)
            {
                m_Editor->m_MainLoop->GetSceneManager ().GetActiveScene ()->RemoveSpeech (nameToFind);
            }

            m_Editor->m_MainLoop->GetSceneManager ().GetActiveScene ()->AddSpeech (m_Speech);
            m_Editor->m_EditorSpeechMode.Clear ();

            return true;
        }

        return false;
    }

    //--------------------------------------------------------------------------------------------------

    void EditorSpeechMode::RemoveSpeech (const std::string& name)
    {
        m_Editor->m_MainLoop->GetSceneManager ().GetActiveScene ()->RemoveSpeech (name);
    }

    //--------------------------------------------------------------------------------------------------

    void EditorSpeechMode::Clear ()
    {
        m_Speech.Name = "";
        m_Speech.ActorRegionName = "";
        m_Speech.MaxCharsInLine = 0;
        m_Speech.MaxLines = 0;
        m_Speech.RelativeFramePosition = BottomCenter;
        m_Speech.AbsoluteFramePosition = { 0, 0 };
        m_Speech.Text.clear ();
        m_Speech.Outcomes.clear ();
    }

    //--------------------------------------------------------------------------------------------------
}
