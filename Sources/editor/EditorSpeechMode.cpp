// Copyright 2017-2019 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "EditorSpeechMode.h"
#include "Editor.h"
#include "MainLoop.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    EditorSpeechMode::EditorSpeechMode (Editor* editor)
        : m_Editor (editor)
    {
        Clear ();
    }

    //--------------------------------------------------------------------------------------------------

    EditorSpeechMode::~EditorSpeechMode () {}

    //--------------------------------------------------------------------------------------------------

    bool EditorSpeechMode::AddOrUpdateSpeech ()
    {
        if (strlen (m_Speech.Name.c_str ()) > 0)
        {
            SceneManager& sceneManager = m_Editor->GetMainLoop ()->GetSceneManager ();
            SpeechData* speech = sceneManager.GetActiveScene ()->GetSpeech (m_Speech.ID);

            if (speech)
            {
                std::map<long, SpeechData>& speeches
                    = m_Editor->GetMainLoop ()->GetSceneManager ().GetActiveScene ()->GetSpeeches ();
                for (std::map<long, SpeechData>::iterator it = speeches.begin (); it != speeches.end (); ++it)
                {
                    std::map<int, std::vector<SpeechOutcome>>& outcomes = it->second.Outcomes;

                    for (std::map<int, std::vector<SpeechOutcome>>::iterator it2 = outcomes.begin ();
                         it2 != outcomes.end (); ++it2)
                    {
                        for (SpeechOutcome& out : it2->second)
                        {
                            if (out.Action == speech->Name)
                            {
                                out.Action = m_Speech.Name;
                            }
                        }
                    }
                }

                speech->AbsoluteFramePosition = m_Speech.AbsoluteFramePosition;
                speech->RelativeFramePosition = m_Speech.RelativeFramePosition;
                speech->ActorRegionName = m_Speech.ActorRegionName;
                speech->MaxCharsInLine = m_Speech.MaxCharsInLine;
                speech->MaxLines = m_Speech.MaxLines;
                speech->Name = m_Speech.Name;
                speech->Text = m_Speech.Text;
                speech->Outcomes = m_Speech.Outcomes;
                speech->Action = m_Speech.Action;
                speech->Group = m_Speech.Group;
            }
            else
            {
                sceneManager.GetActiveScene ()->AddSpeech (m_Speech);
            }

            m_Editor->GetEditorSpeechMode ().Clear ();

            return true;
        }

        return false;
    }

    //--------------------------------------------------------------------------------------------------

    void EditorSpeechMode::RemoveSpeech (int id)
    {
        m_Editor->GetMainLoop ()->GetSceneManager ().GetActiveScene ()->RemoveSpeech (id);
    }

    //--------------------------------------------------------------------------------------------------

    void EditorSpeechMode::Clear ()
    {
        m_Speech.Name = "";
        m_Speech.Group = "";
        m_Speech.ActorRegionName = "";
        m_Speech.MaxCharsInLine = 0;
        m_Speech.MaxLines = 0;
        m_Speech.RelativeFramePosition = BottomCenter;
        m_Speech.AbsoluteFramePosition = {0, 0};
        m_Speech.Text.clear ();
        m_Speech.Outcomes.clear ();
    }

    //--------------------------------------------------------------------------------------------------

    SpeechData& EditorSpeechMode::GetSpeechData () { return m_Speech; }

    //--------------------------------------------------------------------------------------------------

    void EditorSpeechMode::SetSpeechData (SpeechData& data) { m_Speech = data; }

    //--------------------------------------------------------------------------------------------------
}
