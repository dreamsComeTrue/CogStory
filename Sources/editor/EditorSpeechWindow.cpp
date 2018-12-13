// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "EditorSpeechWindow.h"
#include "Editor.h"
#include "EditorActorWindow.h"
#include "MainLoop.h"
#include "SpeechFrame.h"
#include "SpeechFrameManager.h"

#include "imgui.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    std::vector<std::string> g_SpeechFrames = {"player_head"};

    //--------------------------------------------------------------------------------------------------

    EditorSpeechWindow::EditorSpeechWindow (Editor* editor)
        : m_Editor (editor)
        , m_IsVisible (false)
        , m_LangIndex (0)
        , m_SelectedRegion (0)
        , m_MaxChars (0)
        , m_MaxLines (0)
        , m_AbsPosX (0)
        , m_AbsPosY (0)
        , m_RelPosition (0)
        , m_Action (0)
    {
        m_Languages.push_back ("EN");
        m_Languages.push_back ("PL");

        m_Regions.push_back ("");

        for (const std::string& s : g_SpeechFrames)
        {
            m_Regions.push_back (s);
        }

        for (const auto& s : g_ScreenRelativePosition)
        {
            m_Positions.push_back (s.second);
        }
    }

    //--------------------------------------------------------------------------------------------------

    void EditorSpeechWindow::Show ()
    {
        m_IsVisible = true;

        ClearControls ();
        UpdateOutcomes ();
    }

    //--------------------------------------------------------------------------------------------------

    void EditorSpeechWindow::OnSave ()
    {
        SpeechData& speechData = m_Editor->GetEditorSpeechMode ().GetSpeechData ();

        for (const auto& pos : g_ScreenRelativePosition)
        {
            if (pos.second == m_Positions[m_RelPosition])
            {
                speechData.RelativeFramePosition = pos.first;
                break;
            }
        }

        if (std::string (m_SpeechID) != "")
        {
            speechData.ID = atoi (std::string (m_SpeechID).c_str ());
        }
        else
        {
            int maxID = -1;

            std::map<long, SpeechData>& speeches
                = m_Editor->GetMainLoop ()->GetSceneManager ().GetActiveScene ()->GetSpeeches ();
            for (std::map<long, SpeechData>::iterator it = speeches.begin (); it != speeches.end (); ++it)
            {
                if (it->first > maxID)
                {
                    maxID = it->first;
                }
            }

            speechData.ID = maxID + 1;
        }

        speechData.ActorRegionName = m_Regions[m_SelectedRegion];
        speechData.AbsoluteFramePosition.X = m_AbsPosX;
        speechData.AbsoluteFramePosition.Y = m_AbsPosY;
        speechData.MaxCharsInLine = m_MaxChars;
        speechData.MaxLines = m_MaxLines;
        speechData.Action = m_Actions[m_Action];
        speechData.Group = m_SpeechGroup;

        if (std::string (m_SpeechName) != "")
        {
            if (m_Editor->GetEditorSpeechMode ().AddOrUpdateSpeech ())
            {
                ClearControls ();
                UpdateOutcomes ();

                SelectSpeech (speechData.ID);
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    void EditorSpeechWindow::OnRemove ()
    {
        if (std::string (m_SpeechName) != "")
        {
            m_Editor->GetEditorSpeechMode ().RemoveSpeech (atoi (std::string (m_SpeechID).c_str ()));

            ClearControls ();
            UpdateOutcomes ();
        }
    }

    //--------------------------------------------------------------------------------------------------

    void EditorSpeechWindow::OnOutcome ()
    {
        if (std::string (m_SpeechName) != "")
        {
            SpeechOutcome outcome;
            m_Editor->GetEditorSpeechMode ().GetSpeechData ().Outcomes[m_LangIndex].push_back (outcome);

            UpdateOutcomes ();
        }
    }

    //--------------------------------------------------------------------------------------------------

    void EditorSpeechWindow::OnPreview ()
    {
        SpeechFrameManager* frameManager = m_Editor->GetMainLoop ()->GetSceneManager ().GetSpeechFrameManager ();

        if (frameManager->GetSpeechFrames ().empty ())
        {
            frameManager->AddSpeechFrame (std::string (m_SpeechName));
        }
        else
        {
            frameManager->Clear ();
        }
    }

    //--------------------------------------------------------------------------------------------------

    void EditorSpeechWindow::OnWindoClosed ()
    {
        m_Editor->GetMainLoop ()->GetSceneManager ().GetSpeechFrameManager ()->Clear ();
    }

    //--------------------------------------------------------------------------------------------------

    void EditorSpeechWindow::OnPositionTypeChanged ()
    {
        for (const auto& pos : g_ScreenRelativePosition)
        {
            if (pos.second == m_Positions[m_RelPosition])
            {
                m_Editor->GetEditorSpeechMode ().GetSpeechData ().RelativeFramePosition = pos.first;
                break;
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    void EditorSpeechWindow::UpdateOutcomes ()
    {
        std::map<long, SpeechData>& speeches
            = m_Editor->GetMainLoop ()->GetSceneManager ().GetActiveScene ()->GetSpeeches ();

        std::map<std::string, asIScriptFunction*>& choiceFunctions
            = m_Editor->GetMainLoop ()->GetSceneManager ().GetActiveScene ()->GetChoiceFunctions ();

        m_Actions.clear ();
        m_Actions.push_back ("[CLOSE]");

        for (std::map<long, SpeechData>::iterator it = speeches.begin (); it != speeches.end (); ++it)
        {
            m_Actions.push_back ((*it).second.Name);
        }

        for (std::map<std::string, asIScriptFunction*>::iterator it = choiceFunctions.begin ();
             it != choiceFunctions.end (); ++it)
        {
            std::string name = REGISTERED_CHOICE_PREFIX + (*it).first;
            m_Actions.push_back (name);
        }

        m_SpeechOutcomes.clear ();

        std::vector<SpeechOutcome>& outcomes = m_Editor->GetEditorSpeechMode ().GetSpeechData ().Outcomes[m_LangIndex];

        for (SpeechOutcome& out : outcomes)
        {
            EditorSpeechOutcome edOut;

            strcpy (edOut.Name, out.Name.c_str ());
            strcpy (edOut.Text, out.Text.c_str ());
            strcpy (edOut.Action, out.Action.c_str ());

            for (size_t i = 0; i < m_Actions.size (); ++i)
            {
                if (out.Action == m_Actions[i])
                {
                    edOut.ActionIndex = i;
                    break;
                }
            }

            m_SpeechOutcomes.push_back (edOut);
        }
    }

    //--------------------------------------------------------------------------------------------------

    void EditorSpeechWindow::ClearControls ()
    {
        memset (m_SpeechID, 0, ARRAY_SIZE (m_SpeechID));
        memset (m_SpeechName, 0, ARRAY_SIZE (m_SpeechName));
        memset (m_SpeechGroup, 0, ARRAY_SIZE (m_SpeechGroup));

        m_LangIndex = 0;
        m_MaxChars = 10;
        m_MaxLines = 1;
        m_AbsPosX = 0;
        m_AbsPosY = 0;
        m_Action = 0;

        memset (m_Text, 0, ARRAY_SIZE (m_Text));
        m_RelPosition = BottomCenter;
        m_Editor->GetEditorSpeechMode ().Clear ();
        m_SpeechOutcomes.clear ();
    }

    //--------------------------------------------------------------------------------------------------

    void EditorSpeechWindow::SelectSpeech (long id)
    {
        std::map<long, SpeechData>& speeches
            = m_Editor->GetMainLoop ()->GetSceneManager ().GetActiveScene ()->GetSpeeches ();
        SpeechData& speech = speeches[id];
        m_Editor->GetEditorSpeechMode ().SetSpeechData (speech);

        strcpy (m_SpeechID, std::to_string (speech.ID).c_str ());
        strcpy (m_SpeechName, speech.Name.c_str ());
        strcpy (m_SpeechGroup, speech.Group.c_str ());

        for (size_t i = 0; i < m_Regions.size (); ++i)
        {
            if (m_Regions[i] == speech.ActorRegionName)
            {
                m_SelectedRegion = i;
                break;
            }
        }

        strcpy (m_Text, speech.Text[m_LangIndex].c_str ());
        m_MaxChars = speech.MaxCharsInLine;
        m_MaxLines = speech.MaxLines;
        m_RelPosition = speech.RelativeFramePosition;
        m_AbsPosX = static_cast<int> (speech.AbsoluteFramePosition.X);
        m_AbsPosY = static_cast<int> (speech.AbsoluteFramePosition.Y);

        for (size_t i = 0; i < m_Actions.size (); ++i)
        {
            if (m_Actions[i] == speech.Action)
            {
                m_Action = i;
                break;
            }
        }

        UpdateOutcomes ();
    }

    //--------------------------------------------------------------------------------------------------

    void EditorSpeechWindow::RenderUI ()
    {
        ImGui::SetNextWindowSize (ImVec2 (900, 550), ImGuiCond_Always);

        if (ImGui::BeginPopupModal ("Speech Editor", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::BeginChild (
                "Child1", ImVec2 (280, ImGui::GetWindowSize ().y - 50), false, ImGuiWindowFlags_HorizontalScrollbar);
            ImGui::BeginGroup ();
            {
                if (ImGui::TreeNodeEx ("Speeches", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    std::vector<std::string> groups;
                    groups.push_back ("Ungrouped");

                    std::map<long, SpeechData>& speeches
                        = m_Editor->GetMainLoop ()->GetSceneManager ().GetActiveScene ()->GetSpeeches ();

                    for (std::map<long, SpeechData>::iterator it = speeches.begin (); it != speeches.end (); ++it)
                    {
                        std::string groupName = (*it).second.Group;
                        bool foundGroup = false;

                        for (const std::string& gr : groups)
                        {
                            if (gr == groupName || groupName == "")
                            {
                                foundGroup = true;
                            }
                        }

                        if (!foundGroup)
                        {
                            groups.push_back (groupName);
                        }
                    }

                    for (const std::string& gr : groups)
                    {
                        if (ImGui::TreeNodeEx (gr.c_str (), ImGuiTreeNodeFlags_DefaultOpen))
                        {
                            for (std::map<long, SpeechData>::iterator it = speeches.begin (); it != speeches.end ();
                                 ++it)
                            {
                                std::string thisGroupName = (*it).second.Group;

                                if (thisGroupName == gr || (gr == "Ungrouped" && thisGroupName == ""))
                                {
                                    std::string name = (*it).second.Name;

                                    if (ImGui::Selectable (name.c_str (), false))
                                    {
                                        SelectSpeech ((*it).first);
                                    }
                                }
                            }

                            ImGui::TreePop ();
                        }
                    }

                    ImGui::TreePop ();
                }
            }
            ImGui::EndGroup ();
            ImGui::EndChild ();

            ImGui::SameLine ();

            ImGui::BeginChild (
                "Child2", ImVec2 (460, ImGui::GetWindowSize ().y - 50), false, ImGuiWindowFlags_HorizontalScrollbar);
            ImGui::BeginGroup ();
            {
                ImGui::LabelText ("ID", m_SpeechID);

                SpeechData& speechData = m_Editor->GetEditorSpeechMode ().GetSpeechData ();

                if (ImGui::InputText ("Name", m_SpeechName, ARRAY_SIZE (m_SpeechName)))
                {
                    speechData.Name = std::string (m_SpeechName);
                }

                if (ImGui::InputText ("Group", m_SpeechGroup, ARRAY_SIZE (m_SpeechGroup)))
                {
                    speechData.Group = std::string (m_SpeechGroup);
                }

                if (ImGui::Combo ("Language", &m_LangIndex, m_Languages))
                {
                    strcpy (m_Text, speechData.Text[m_LangIndex].c_str ());
                }

                if (ImGui::Combo ("Region", &m_SelectedRegion, m_Regions))
                {
                    speechData.ActorRegionName = m_Regions[m_SelectedRegion];
                }

                ImGui::InputInt ("Max chars/line", &m_MaxChars);
                ImGui::InputInt ("Max lines", &m_MaxLines);

                if (ImGui::Combo ("Position", &m_RelPosition, m_Positions))
                {
                    OnPositionTypeChanged ();
                }

                if (ImGui::InputInt ("X Pos", &m_AbsPosX))
                {
                    speechData.AbsoluteFramePosition.X = m_AbsPosX;
                }

                if (ImGui::InputInt ("Y Pos", &m_AbsPosY))
                {
                    speechData.AbsoluteFramePosition.Y = m_AbsPosY;
                }

                if (ImGui::Combo ("Actions", &m_Action, m_Actions))
                {
                    speechData.Action = m_Actions[m_Action];
                }

                ImGui::Dummy (ImVec2 (100, 10));

                if (ImGui::InputTextMultiline ("Text", m_Text, ARRAY_SIZE (m_Text), ImVec2 (305, 90)))
                {
                    speechData.Text[m_LangIndex] = std::string (m_Text);
                }

                ImGui::Dummy (ImVec2 (100, 10));

                ImGui::Separator ();
                ImGui::Text ("              --- OUTCOMES ---");

                std::vector<SpeechOutcome>& outcomes = speechData.Outcomes[m_LangIndex];
                std::vector<EditorSpeechOutcome>& edOutcomes = m_SpeechOutcomes;

                for (size_t i = 0; i < edOutcomes.size (); ++i)
                {
                    EditorSpeechOutcome& out = edOutcomes[i];

                    ImGui::PushItemWidth (40.f);
                    if (ImGui::InputText (
                            (std::string ("##Name") + std::to_string (i)).c_str (), out.Name, ARRAY_SIZE (out.Name)))
                    {
                        outcomes[i].Name = out.Name;
                    }
                    ImGui::PopItemWidth ();

                    ImGui::SameLine ();

                    ImGui::PushItemWidth (150.f);
                    if (ImGui::InputText (
                            (std::string ("##Text") + std::to_string (i)).c_str (), out.Text, ARRAY_SIZE (out.Text)))
                    {
                        outcomes[i].Text = out.Text;
                    }
                    ImGui::PopItemWidth ();

                    ImGui::SameLine ();

                    ImGui::PushItemWidth (85.f);
                    if (ImGui::Combo (
                            (std::string ("##Action") + std::to_string (i)).c_str (), &out.ActionIndex, m_Actions))
                    {
                        outcomes[i].Action = m_Actions[out.ActionIndex];
                    }
                    ImGui::PopItemWidth ();

                    ImGui::SameLine ();

                    if (ImGui::Button ((std::string ("^##Up") + std::to_string (i)).c_str ()))
                    {
                        if (i == 0)
                        {
                            std::iter_swap (outcomes.begin (), outcomes.end () - 1);
                        }
                        else
                        {
                            std::iter_swap (outcomes.begin () + i, outcomes.begin () + i - 1);
                        }

                        UpdateOutcomes ();
                    }

                    ImGui::SameLine ();

                    if (ImGui::Button ((std::string ("v##Down") + std::to_string (i)).c_str ()))
                    {
                        if (i == outcomes.size () - 1)
                        {
                            std::iter_swap (outcomes.begin (), outcomes.end () - 1);
                        }
                        else
                        {
                            std::iter_swap (outcomes.begin () + i, outcomes.begin () + i + 1);
                        }

                        UpdateOutcomes ();
                    }

                    ImGui::SameLine ();

                    if (ImGui::Button ((std::string ("X##Remove") + std::to_string (i)).c_str ()))
                    {
                        outcomes.erase (outcomes.begin () + i);
                        UpdateOutcomes ();
                    }
                }
            }
            ImGui::EndGroup ();
            ImGui::EndChild ();

            ImGui::SameLine ();

            ImGui::BeginGroup ();
            {
                ImVec2 buttonSize = ImVec2 (100.f, 18.f);

                if (ImGui::Button ("NEW", buttonSize))
                {
                    ClearControls ();
                }

                if (ImGui::Button ("SAVE", buttonSize))
                {
                    OnSave ();
                }

                if (ImGui::Button ("REMOVE", buttonSize))
                {
                    OnRemove ();
                }

                if (ImGui::Button ("OUTCOME", buttonSize))
                {
                    OnOutcome ();
                }

                ImGui::NewLine ();

                if (ImGui::Button ("PREVIEW", buttonSize))
                {
                    OnSave ();
                    OnPreview ();
                }

                ImGui::NewLine ();

                if (ImGui::Button ("ACCEPT", buttonSize))
                {
                    ImGui::CloseCurrentPopup ();
                    m_IsVisible = false;

                    if (m_Editor->GetActorWindow ()->IsVisible ())
                    {
                        m_Editor->GetActorWindow ()->UpdateSpeeches ();
                    }
                }

                if (ImGui::Button ("CANCEL", buttonSize) || m_Editor->IsCloseCurrentPopup ())
                {
                    ImGui::CloseCurrentPopup ();
                    m_IsVisible = false;

                    m_Editor->SetCloseCurrentPopup (false);

                    if (m_Editor->GetActorWindow ()->IsVisible ())
                    {
                        m_Editor->GetActorWindow ()->UpdateSpeeches ();
                    }
                }
            }
            ImGui::EndGroup ();

            ImGui::EndPopup ();
        }
    }

    //--------------------------------------------------------------------------------------------------

    bool EditorSpeechWindow::IsVisible () const { return m_IsVisible; }

    //--------------------------------------------------------------------------------------------------
}
