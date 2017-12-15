// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "EditorActorWindow.h"
#include "Editor.h"
#include "MainLoop.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    EditorActorWindow::EditorActorWindow (Editor* editor, Gwk::Controls::Canvas* canvas)
        : m_Editor (editor)
    {
        m_LangIndex = 0;

        m_SceneWindow = new Gwk::Controls::WindowControl (canvas);
        m_SceneWindow->SetTitle ("Actor Editor");
        m_SceneWindow->SetSize (canvas->Width () - 150, canvas->Height () - 80);
        m_SceneWindow->CloseButtonPressed ();

        Gwk::Controls::DockBase* dock = new Gwk::Controls::DockBase (m_SceneWindow);
        dock->Dock (Gwk::Position::Fill);

        m_SpeechesTree = new Gwk::Controls::TreeControl (dock);
        m_SpeechesTree->SetBounds (10, 10, 300, dock->Height () - 55);
        m_SpeechesTree->ExpandAll ();
        m_SpeechesTree->Dock (Gwk::Position::Fill);

        dock->GetLeft ()->GetTabControl ()->AddPage ("Actors", m_SpeechesTree);
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

        Gwk::Controls::Label* langLabel = new Gwk::Controls::Label (center);
        langLabel->SetPos (xOffset, m_NameTextBox->Bottom () + 5);
        langLabel->SetText ("Lang:");
        langLabel->SizeToContents ();

        m_LanguageCombo = new Gwk::Controls::ComboBox (center);
        m_LanguageCombo->SetPos (xOffset, langLabel->Bottom () + 5);
        m_LanguageCombo->SetWidth (controlWidth);
        m_LanguageCombo->AddItem ("EN", "EN");
        m_LanguageCombo->AddItem ("PL", "PL");

        Gwk::Controls::Label* textLabel = new Gwk::Controls::Label (center);
        textLabel->SetPos (xOffset, m_LanguageCombo->Bottom () + 5);
        textLabel->SetText ("Text:");
        textLabel->SizeToContents ();

        m_TextData = new Gwk::Controls::TextBoxMultiline (center);
        m_TextData->SetTextColor (Gwk::Colors::White);
        m_TextData->SetPos (xOffset, textLabel->Bottom () + 5);
        m_TextData->SetSize (controlWidth, 90);

        Gwk::Controls::Button* addSpeechButton = new Gwk::Controls::Button (center);
        addSpeechButton->SetText ("SAVE");
        addSpeechButton->SetWidth (155);
        addSpeechButton->SetPos (xOffset, m_TextData->Bottom () + 10);
        addSpeechButton->onPress.Add (this, &EditorActorWindow::OnSave);

        Gwk::Controls::Button* removeSpeechButton = new Gwk::Controls::Button (center);
        removeSpeechButton->SetText ("REMOVE");
        removeSpeechButton->SetWidth (155);
        removeSpeechButton->SetPos (addSpeechButton->Right () + 5, m_TextData->Bottom () + 10);
        removeSpeechButton->onPress.Add (this, &EditorActorWindow::OnRemove);

        Gwk::Controls::Button* outcomeButton = new Gwk::Controls::Button (center);
        outcomeButton->SetText ("OUTCOME");
        outcomeButton->SetWidth (160);
        outcomeButton->SetPos (removeSpeechButton->Right () + 5, m_TextData->Bottom () + 10);

        m_OutcomesContainer = new Gwk::Controls::ScrollControl (center);
        m_OutcomesContainer->SetBounds (xOffset, outcomeButton->Bottom () + 10, 610, 190);

        Gwk::Controls::Button* okButton = new Gwk::Controls::Button (center);
        okButton->SetText ("ACCEPT");
        okButton->SetPos (m_SceneWindow->Width () - 330, m_SceneWindow->Height () - 65);
        okButton->onPress.Add (this, &EditorActorWindow::OnAccept);
    }

    //--------------------------------------------------------------------------------------------------

    void EditorActorWindow::Show ()
    {
        m_SceneWindow->SetPosition (Gwk::Position::Center);
        m_SceneWindow->SetHidden (false);
        m_SceneWindow->MakeModal (true);
    }

    //--------------------------------------------------------------------------------------------------

    void EditorActorWindow::OnSave ()
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
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    void EditorActorWindow::OnRemove ()
    {
        if (m_NameTextBox->GetText () != "")
        {
            m_Editor->m_EditorSpeechMode.RemoveSpeech (m_NameTextBox->GetText ());
            m_NameTextBox->SetText ("");
            m_TextData->SetText ("");
            m_LanguageCombo->SelectItemByName ("EN");
            m_Editor->m_EditorSpeechMode.Clear ();
        }
    }

    //--------------------------------------------------------------------------------------------------

    void EditorActorWindow::OnAccept () { m_SceneWindow->CloseButtonPressed (); }

    //--------------------------------------------------------------------------------------------------

    void EditorActorWindow::OnCancel () { m_SceneWindow->CloseButtonPressed (); }

    //--------------------------------------------------------------------------------------------------
}
