// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "EditorSpeechWindow.h"
#include "MainLoop.h"
#include "Editor.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    SpeechWindow::SpeechWindow (Editor* editor, Gwk::Controls::Canvas* canvas) :
        m_Editor (editor)
    {
        m_LangIndex = 0;

        m_SceneWindow = new Gwk::Controls::WindowControl (canvas);
        m_SceneWindow->SetTitle ("Speech Editor");
        m_SceneWindow->SetSize (canvas->Width () - 150, canvas->Height () - 80);
        m_SceneWindow->CloseButtonPressed ();

        Gwk::Controls::DockBase* dock = new Gwk::Controls::DockBase (m_SceneWindow);
        dock->Dock (Gwk::Position::Fill);

        m_SpeechesTree = new Gwk::Controls::TreeControl (dock);
        m_SpeechesTree->SetBounds (10, 10, 300, dock->Height () - 55);
        m_SpeechesTree->ExpandAll ();
        m_SpeechesTree->Dock (Gwk::Position::Fill);
        m_SpeechesTree->onSelect.Add (this, &SpeechWindow::OnSpeechSelect);

        dock->GetLeft ()->GetTabControl ()->AddPage ("Speeches", m_SpeechesTree);
        m_SpeechesTree->SetMargin (Gwk::Margin ());
        dock->GetLeft ()->GetTabControl ()->GetTab (0)->Hide ();

        int xOffset = 10;

        Gwk::Controls::Base* center = new Gwk::Controls::Base (dock);
        center->Dock (Gwk::Position::Fill);

        Gwk::Controls::Label* nameLabel = new Gwk::Controls::Label (center);
        nameLabel->SetPos (xOffset, 10);
        nameLabel->SetText ("Name:");
        nameLabel->SizeToContents ();

        int controlWidth = 610;

        m_NameTextBox = new Gwk::Controls::TextBox (center);
        m_NameTextBox->SetTextColor (Gwk::Colors::White);
        m_NameTextBox->SetText ("");
        m_NameTextBox->SetWidth (controlWidth);
        m_NameTextBox->SetPos (xOffset, nameLabel->Bottom () + 5);
        m_NameTextBox->onTextChanged.Add (this, &SpeechWindow::OnNameEdit);

        Gwk::Controls::Label* langLabel = new Gwk::Controls::Label (center);
        langLabel->SetPos (xOffset, m_NameTextBox->Bottom () + 5);
        langLabel->SetText ("Lang:");
        langLabel->SizeToContents ();

        m_LanguageCombo = new Gwk::Controls::ComboBox (center);
        m_LanguageCombo->SetPos (xOffset, langLabel->Bottom () + 5);
        m_LanguageCombo->SetWidth (controlWidth);
        m_LanguageCombo->AddItem ("EN", "EN");
        m_LanguageCombo->AddItem ("PL", "PL");
        m_LanguageCombo->onSelection.Add (this, &SpeechWindow::OnLangSelected);

        Gwk::Controls::Label* textLabel = new Gwk::Controls::Label (center);
        textLabel->SetPos (xOffset, m_LanguageCombo->Bottom () + 5);
        textLabel->SetText ("Text:");
        textLabel->SizeToContents ();

        m_TextData = new Gwk::Controls::TextBoxMultiline (center);
        m_TextData->SetTextColor (Gwk::Colors::White);
        m_TextData->SetPos (xOffset, textLabel->Bottom () + 5);
        m_TextData->SetSize (controlWidth, 90);
        m_TextData->onTextChanged.Add (this, &SpeechWindow::OnTextChanged);

        Gwk::Controls::Button* addSpeechButton = new Gwk::Controls::Button (center);
        addSpeechButton->SetText ("SAVE");
        addSpeechButton->SetWidth (155);
        addSpeechButton->SetPos (xOffset, m_TextData->Bottom () + 10);
        addSpeechButton->onPress.Add (this, &SpeechWindow::OnSave);

        Gwk::Controls::Button* removeSpeechButton = new Gwk::Controls::Button (center);
        removeSpeechButton->SetText ("REMOVE");
        removeSpeechButton->SetWidth (155);
        removeSpeechButton->SetPos (addSpeechButton->Right () + 5, m_TextData->Bottom () + 10);
        removeSpeechButton->onPress.Add (this, &SpeechWindow::OnRemove);

        Gwk::Controls::Button* outcomeButton = new Gwk::Controls::Button (center);
        outcomeButton->SetText ("OUTCOME");
        outcomeButton->SetWidth (160);
        outcomeButton->SetPos (removeSpeechButton->Right () + 5, m_TextData->Bottom () + 10);
        outcomeButton->onPress.Add (this, &SpeechWindow::OnOutcome);

        m_OutcomesContainer = new Gwk::Controls::ScrollControl (center);
        m_OutcomesContainer->SetBounds (xOffset, outcomeButton->Bottom () + 10, 610, 190);

        Gwk::Controls::Button* okButton = new Gwk::Controls::Button (center);
        okButton->SetText ("ACCEPT");
        okButton->SetPos (m_SceneWindow->Width () - 330, m_SceneWindow->Height () - 65);
        okButton->onPress.Add (this, &SpeechWindow::OnAccept);
    }

    //--------------------------------------------------------------------------------------------------

    void SpeechWindow::Show ()
    {
        m_SceneWindow->SetPosition (Gwk::Position::Center);
        m_SceneWindow->SetHidden (false);
        m_SceneWindow->MakeModal (true);
    }

    //--------------------------------------------------------------------------------------------------

    void SpeechWindow::OnSave ()
    {
        if (m_NameTextBox->GetText () != "")
        {
            Gwk::Controls::Base::List& childNodes = m_SpeechesTree->GetChildNodes ();
            std::string oldName = "";

            for (Gwk::Controls::Base* control : childNodes)
            {
                Gwk::Controls::TreeNode* node = (Gwk::Controls::TreeNode*)control;

                if (node->IsSelected ())
                {
                    oldName = node->GetText ();
                    break;
                }
            }

            bool ret = m_Editor->m_EditorSpeechMode.AddOrUpdateSpeech (oldName);

            if (ret)
            {
                m_NameTextBox->SetText ("");
                m_TextData->SetText ("");
                m_LanguageCombo->SelectItemByName ("EN");
                m_Editor->m_EditorSpeechMode.Clear ();

                UpdateSpeechesTree ();
                UpdateOutcomes ();
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    void SpeechWindow::OnRemove ()
    {
        if (m_NameTextBox->GetText () != "")
        {
            m_Editor->m_EditorSpeechMode.RemoveSpeech (m_NameTextBox->GetText ());
            UpdateSpeechesTree ();

            m_NameTextBox->SetText ("");
            m_TextData->SetText ("");
            m_LanguageCombo->SelectItemByName ("EN");
            m_Editor->m_EditorSpeechMode.Clear ();

            UpdateOutcomes ();
        }
    }

    //--------------------------------------------------------------------------------------------------

    void SpeechWindow::OnOutcome ()
    {
        SpeechOutcome outcome;
        m_Editor->m_EditorSpeechMode.m_Speech.Outcomes[m_LangIndex].push_back (outcome);

        UpdateOutcomes ();
    }

    //--------------------------------------------------------------------------------------------------

    void SpeechWindow::OnAccept ()
    {
        m_SceneWindow->CloseButtonPressed (); 
    }

    //--------------------------------------------------------------------------------------------------

    void SpeechWindow::OnCancel ()
    {
        m_SceneWindow->CloseButtonPressed (); 
    }

    //--------------------------------------------------------------------------------------------------

    void SpeechWindow::OnSpeechSelect (Gwk::Controls::Base* control)
    {
        Gwk::Controls::TreeNode* node = (Gwk::Controls::TreeNode*)control;

        if (node != nullptr && node->IsSelected ())
        {
            std::map<std::string, SpeechData>& speeches = m_Editor->m_MainLoop->GetSceneManager ().GetActiveScene ()->GetSpeeches ();
            m_Editor->m_EditorSpeechMode.m_Speech = speeches[node->GetText ()];

            m_NameTextBox->SetText (m_Editor->m_EditorSpeechMode.m_Speech.Name);
            m_TextData->SetText (m_Editor->m_EditorSpeechMode.m_Speech.Text[m_LangIndex]);

            UpdateOutcomes ();
        }
    }

    //--------------------------------------------------------------------------------------------------

    void SpeechWindow::OnNameEdit ()
    {
        m_Editor->m_EditorSpeechMode.m_Speech.Name = m_NameTextBox->GetText ();
    }

    //--------------------------------------------------------------------------------------------------

    void SpeechWindow::OnLangSelected ()
    {
        Gwk::Controls::Label* selItem = m_LanguageCombo->GetSelectedItem ();

        if (selItem->GetText () == "EN")
        {
            m_LangIndex = 0;
        }
        else if (selItem->GetText () == "PL")
        {
            m_LangIndex = 1;
        }

        m_TextData->SetText (m_Editor->m_EditorSpeechMode.m_Speech.Text[m_LangIndex]);
    }

    //--------------------------------------------------------------------------------------------------

    void SpeechWindow::OnTextChanged ()
    {
        m_Editor->m_EditorSpeechMode.m_Speech.Text[m_LangIndex] = m_TextData->GetText ();
    }

    //--------------------------------------------------------------------------------------------------

    void SpeechWindow::UpdateOutcomes ()
    {
        m_OutcomesContainer->RemoveAllChildren ();

        std::vector<SpeechOutcome>& outcomes = m_Editor->m_EditorSpeechMode.m_Speech.Outcomes[m_LangIndex];

        std::map<std::string, SpeechData>& speeches = m_Editor->m_MainLoop->GetSceneManager ().GetActiveScene ()->GetSpeeches ();

        int currentY = 0;
        for (int i = 0; i < outcomes.size (); ++i)
        {
            Gwk::Controls::TextBox* idTextBox = new Gwk::Controls::TextBox (m_OutcomesContainer);
            idTextBox->SetTextColor (Gwk::Colors::White);
            idTextBox->SetText (outcomes[i].Name);
            idTextBox->SetWidth (65);
            idTextBox->SetPos (0, currentY);
            idTextBox->onTextChanged.Add (this, &SpeechWindow::OnOutcomeIDTextChanged);

            Gwk::Controls::TextBox* dataTextBox = new Gwk::Controls::TextBox (m_OutcomesContainer);
            dataTextBox->SetTextColor (Gwk::Colors::White);
            dataTextBox->SetText (outcomes[i].Text);
            dataTextBox->SetWidth (255);
            dataTextBox->SetPos (idTextBox->Right () + 5, currentY);
            dataTextBox->onTextChanged.Add (this, &SpeechWindow::OnOutcomeDataTextChanged);

            Gwk::Controls::ComboBox* actionCombo = new Gwk::Controls::ComboBox (m_OutcomesContainer);
            actionCombo->SetPos (dataTextBox->Right () + 5, currentY);
            actionCombo->SetWidth (150);
            actionCombo->AddItem ("[CLOSE]", "[CLOSE]");

            for (std::map<std::string, SpeechData>::iterator it = speeches.begin (); it != speeches.end (); ++it)
            {
                actionCombo->AddItem ((*it).first, (*it).first);
            }

            actionCombo->SelectItemByName (outcomes[i].Action, false);
            actionCombo->onSelection.Add (this, &SpeechWindow::OnOutcomeActionChanged);

            Gwk::Controls::Button* upButton = new Gwk::Controls::Button (m_OutcomesContainer);
            upButton->SetText (" ^");
            upButton->SetWidth (20);
            upButton->SetName (ToString (i));
            upButton->SetPos (actionCombo->Right () + 5, currentY);
            upButton->onPress.Add (this, &SpeechWindow::OnUpOutcome);

            Gwk::Controls::Button* downButton = new Gwk::Controls::Button (m_OutcomesContainer);
            downButton->SetText (" v");
            downButton->SetWidth (20);
            downButton->SetName (ToString (i));
            downButton->SetPos (upButton->Right () + 5, currentY);
            downButton->onPress.Add (this, &SpeechWindow::OnDownOutcome);

            Gwk::Controls::Button* removeButton = new Gwk::Controls::Button (m_OutcomesContainer);
            removeButton->SetText ("REMOVE");
            removeButton->SetWidth (60);
            removeButton->SetName (ToString (i));
            removeButton->SetPos (downButton->Right () + 5, currentY);
            removeButton->onPress.Add (this, &SpeechWindow::OnRemoveOutcome);

            currentY += 22;
        }
    }

    //--------------------------------------------------------------------------------------------------

    void SpeechWindow::UpdateSpeechesTree ()
    {
        m_SpeechesTree->Clear ();

        std::map<std::string, SpeechData>& speeches = m_Editor->m_MainLoop->GetSceneManager ().GetActiveScene ()->GetSpeeches ();

        for (std::map<std::string, SpeechData>::iterator it = speeches.begin (); it != speeches.end (); ++it)
        {
            Gwk::Controls::TreeNode* node = m_SpeechesTree->AddNode ((*it).first);
            node->onSelect.Add (this, &SpeechWindow::OnSpeechSelect);
        }

        m_SpeechesTree->ExpandAll ();
    }

    //--------------------------------------------------------------------------------------------------

    void SpeechWindow::OnOutcomeIDTextChanged (Gwk::Controls::Base* control)
    {
        std::vector<SpeechOutcome>& outcomes = m_Editor->m_EditorSpeechMode.m_Speech.Outcomes[m_LangIndex];

        int childIndex = 0;
        for (int i = 0; i < outcomes.size (); ++i)
        {
            Gwk::Controls::Base* child = m_OutcomesContainer->GetChild (childIndex);

            if (control == child)
            {
                outcomes[i].Name = ((Gwk::Controls::TextBox*)control)->GetText ();
                break;
            }

            childIndex += 6;
        }
    }

    //--------------------------------------------------------------------------------------------------

    void SpeechWindow::OnOutcomeDataTextChanged (Gwk::Controls::Base* control)
    {
        std::vector<SpeechOutcome>& outcomes = m_Editor->m_EditorSpeechMode.m_Speech.Outcomes[m_LangIndex];

        int childIndex = 1;
        for (int i = 0; i < outcomes.size (); ++i)
        {
            Gwk::Controls::Base* child = m_OutcomesContainer->GetChild (childIndex);

            if (control == child)
            {
                outcomes[i].Text = ((Gwk::Controls::TextBox*)control)->GetText ();
                break;
            }

            childIndex += 6;
        }
    }

    //--------------------------------------------------------------------------------------------------

    void SpeechWindow::OnOutcomeActionChanged (Gwk::Controls::Base* control)
    {
        std::vector<SpeechOutcome>& outcomes = m_Editor->m_EditorSpeechMode.m_Speech.Outcomes[m_LangIndex];

        int childIndex = 2;
        for (int i = 0; i < outcomes.size (); ++i)
        {
            Gwk::Controls::Base* child = m_OutcomesContainer->GetChild (childIndex);

            if (control == child)
            {
                Gwk::Controls::ComboBox* actionCombo = (Gwk::Controls::ComboBox*)control;

                outcomes[i].Action = actionCombo->GetSelectedItem ()->GetText ();
                break;
            }

            childIndex += 6;
        }

        UpdateOutcomes ();
    }

    //--------------------------------------------------------------------------------------------------

    void SpeechWindow::OnUpOutcome (Gwk::Controls::Base* control)
    {
        std::vector<SpeechOutcome>& outcomes = m_Editor->m_EditorSpeechMode.m_Speech.Outcomes[m_LangIndex];

        int childIndex = 3;
        for (int i = 0; i < outcomes.size (); ++i)
        {
            Gwk::Controls::Base* child = m_OutcomesContainer->GetChild (childIndex);

            if (control == child)
            {
                if (i == 0)
                {
                    std::iter_swap (outcomes.begin (), outcomes.end () - 1);
                }
                else
                {
                    std::iter_swap (outcomes.begin () + i, outcomes.begin () + i - 1);
                }

                break;
            }

            childIndex += 6;
        }

        UpdateOutcomes ();
    }

    //--------------------------------------------------------------------------------------------------

    void SpeechWindow::OnDownOutcome (Gwk::Controls::Base* control)
    {
        std::vector<SpeechOutcome>& outcomes = m_Editor->m_EditorSpeechMode.m_Speech.Outcomes[m_LangIndex];

        int childIndex = 4;
        for (int i = 0; i < outcomes.size (); ++i)
        {
            Gwk::Controls::Base* child = m_OutcomesContainer->GetChild (childIndex);

            if (control == child)
            {
                if (i == outcomes.size () - 1)
                {
                    std::iter_swap (outcomes.begin (), outcomes.end () - 1);
                }
                else
                {
                    std::iter_swap (outcomes.begin () + i, outcomes.begin () + i + 1);
                }

                break;
            }

            childIndex += 6;
        }

        UpdateOutcomes ();
    }

    //--------------------------------------------------------------------------------------------------

    void SpeechWindow::OnRemoveOutcome (Gwk::Controls::Base* control)
    {
        std::vector<SpeechOutcome>& outcomes = m_Editor->m_EditorSpeechMode.m_Speech.Outcomes[m_LangIndex];

        int childIndex = 5;
        for (int i = 0; i < outcomes.size (); ++i)
        {
            Gwk::Controls::Base* child = m_OutcomesContainer->GetChild (childIndex);

            if (control == child)
            {
                outcomes.erase (outcomes.begin () + i);
                break;
            }

            childIndex += 6;
        }

        UpdateOutcomes ();
    }

    //--------------------------------------------------------------------------------------------------
}
