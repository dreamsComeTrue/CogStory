// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "EditorActorWindow.h"
#include "ActorFactory.h"
#include "Editor.h"
#include "MainLoop.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    EditorActorScriptWindow::EditorActorScriptWindow (Editor* editor, Gwk::Controls::Canvas* canvas)
    {
        m_Result = true;
        m_Editor = editor;

        m_SceneWindow = new Gwk::Controls::WindowControl (canvas);
        m_SceneWindow->SetTitle ("Script");
        m_SceneWindow->SetSize (490, 120);
        m_SceneWindow->CloseButtonPressed ();

        Gwk::Controls::Label* nameLabel = new Gwk::Controls::Label (m_SceneWindow);
        nameLabel->SetBounds (20, 10, m_SceneWindow->Width () - 40, m_SceneWindow->Height () - 40);
        nameLabel->SetText ("Name:");
        nameLabel->SizeToContents ();

        Gwk::Controls::TextBox* nameTextBox = new Gwk::Controls::TextBox (m_SceneWindow);
        nameTextBox->SetText (Name);
        nameTextBox->SetTextColor (Gwk::Colors::White);
        nameTextBox->SetWidth (300);
        nameTextBox->SetPos (nameLabel->Right () + 10, nameLabel->Y ());
        nameTextBox->onTextChanged.Add (this, &EditorActorScriptWindow::OnNameEdit);

        Gwk::Controls::Label* pathLabel = new Gwk::Controls::Label (m_SceneWindow);
        pathLabel->SetPos (20, nameLabel->Bottom () + 10);
        pathLabel->SetText ("Path:");
        pathLabel->SizeToContents ();

        Gwk::Controls::TextBox* pathTextBox = new Gwk::Controls::TextBox (m_SceneWindow);
        pathTextBox->SetText (Path);
        pathTextBox->SetTextColor (Gwk::Colors::White);
        pathTextBox->SetWidth (300);
        pathTextBox->SetPos (pathLabel->Right () + 19, pathLabel->Y ());
        pathTextBox->onTextChanged.Add (this, &EditorActorScriptWindow::OnPathEdit);

        Gwk::Controls::Button* browseButton = new Gwk::Controls::Button (m_SceneWindow);
        browseButton->SetText ("BROWSE");
        browseButton->SetPos (pathTextBox->Right () + 5, pathLabel->Y ());
        browseButton->onPress.Add (this, &EditorActorScriptWindow::OnAccept);

        Gwk::Controls::Button* yesButton = new Gwk::Controls::Button (m_SceneWindow);
        yesButton->SetText ("ACCEPT");
        yesButton->SetPos (m_SceneWindow->Width () / 2 - 2 * 50 - 5, m_SceneWindow->Height () - 60);
        yesButton->onPress.Add (this, &EditorActorScriptWindow::OnAccept);

        Gwk::Controls::Button* noButton = new Gwk::Controls::Button (m_SceneWindow);
        noButton->SetText ("CANCEL");
        noButton->SetPos (yesButton->Right () + 10, m_SceneWindow->Height () - 60);
        noButton->onPress.Add (this, &EditorActorScriptWindow::OnCancel);
    }

    //--------------------------------------------------------------------------------------------------

    void EditorActorScriptWindow::Show (std::function<bool(void)> OnAcceptFunc, std::function<bool(void)> OnCancelFunc)
    {
        m_OnAcceptFunc = OnAcceptFunc;
        m_OnCancelFunc = OnCancelFunc;

        m_SceneWindow->SetPosition (Gwk::Position::Center);
        m_SceneWindow->SetHidden (false);
        m_SceneWindow->MakeModal (true);
    }

    //--------------------------------------------------------------------------------------------------

    void EditorActorScriptWindow::OnAccept ()
    {
        m_Result = true;

        if (m_OnAcceptFunc)
        {
            if (!m_OnAcceptFunc ())
            {
                return;
            }
        }

        m_SceneWindow->CloseButtonPressed ();
    }

    //--------------------------------------------------------------------------------------------------

    void EditorActorScriptWindow::OnCancel ()
    {
        m_Result = false;

        if (m_OnCancelFunc)
        {
            if (!m_OnCancelFunc ())
            {
                return;
            }
        }

        m_SceneWindow->CloseButtonPressed ();
    }

    //--------------------------------------------------------------------------------------------------

    void EditorActorScriptWindow::OnNameEdit (Gwk::Controls::Base* control)
    {
        Gwk::Controls::TextBox* nameTextBox = (Gwk::Controls::TextBox*)control;

        Name = nameTextBox->GetText ();
    }

    //--------------------------------------------------------------------------------------------------

    void EditorActorScriptWindow::OnPathEdit (Gwk::Controls::Base* control)
    {
        Gwk::Controls::TextBox* pathTextBox = (Gwk::Controls::TextBox*)control;

        Path = pathTextBox->GetText ();
    }

    //--------------------------------------------------------------------------------------------------

    bool EditorActorScriptWindow::GetResult () { return m_Result; }

    //--------------------------------------------------------------------------------------------------
    //--------------------------------------------------------------------------------------------------

    EditorActorWindow::EditorActorWindow (Editor* editor, Gwk::Controls::Canvas* canvas)
      : m_Editor (editor)
    {
        m_SelectedType = 0;

        m_SceneWindow = new Gwk::Controls::WindowControl (canvas);
        m_SceneWindow->SetTitle ("Actor Editor");
        m_SceneWindow->SetSize (canvas->Width () - 150, canvas->Height () - 80);
        m_SceneWindow->CloseButtonPressed ();

        Gwk::Controls::DockBase* dock = new Gwk::Controls::DockBase (m_SceneWindow);
        dock->Dock (Gwk::Position::Fill);

        m_ActorsTree = new Gwk::Controls::TreeControl (dock);
        m_ActorsTree->SetBounds (10, 10, 300, dock->Height () - 55);
        m_ActorsTree->ExpandAll ();
        m_ActorsTree->Dock (Gwk::Position::Fill);

        dock->GetLeft ()->GetTabControl ()->AddPage ("Actors", m_ActorsTree);
        m_ActorsTree->SetMargin (Gwk::Margin ());
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

        Gwk::Controls::Label* typeLabel = new Gwk::Controls::Label (center);
        typeLabel->SetPos (xOffset, m_NameTextBox->Bottom () + 5);
        typeLabel->SetText ("Type:");
        typeLabel->SizeToContents ();

        m_ActorTypes = new Gwk::Controls::ListBox (center);
        m_ActorTypes->SetPos (xOffset, typeLabel->Bottom () + 5);
        m_ActorTypes->SetSize (150, 400);

        std::vector<std::string>& actorTypes = ActorFactory::GetActorTypes ();

        for (const std::string& type : actorTypes)
        {
            m_ActorTypes->AddItem (type, type);
        }

        m_ActorProperties = new Gwk::Controls::PropertyTree (center);
        m_ActorProperties->SetBounds (m_ActorTypes->Right () + 10, m_ActorTypes->Y (), 340, m_ActorTypes->Height ());
        {
            m_GeneralSection = m_ActorProperties->Add ("General");
            m_GeneralSection->Add ("Name");
            m_GeneralSection->Add ("Parent");

            m_ScriptSection = m_ActorProperties->Add ("Scripts");
        }

        m_ActorProperties->ExpandAll ();

        Gwk::Controls::Button* saveButton = new Gwk::Controls::Button (center);
        saveButton->SetText ("SAVE");
        saveButton->SetPos (m_SceneWindow->Width () - 330, typeLabel->Bottom () + 5);
        saveButton->onPress.Add (this, &EditorActorWindow::OnSave);

        Gwk::Controls::Button* removeButton = new Gwk::Controls::Button (center);
        removeButton->SetText ("REMOVE");
        removeButton->SetPos (m_SceneWindow->Width () - 330, saveButton->Bottom () + 10);
        removeButton->onPress.Add (this, &EditorActorWindow::OnRemove);

        Gwk::Controls::Button* addScriptButton = new Gwk::Controls::Button (center);
        addScriptButton->SetText ("ADD SCRIPT");
        addScriptButton->SetPos (m_SceneWindow->Width () - 330, removeButton->Bottom () + 50);
        addScriptButton->onPress.Add (this, &EditorActorWindow::OnAddScript);

        Gwk::Controls::Button* acceptButton = new Gwk::Controls::Button (center);
        acceptButton->SetText ("ACCEPT");
        acceptButton->SetPos (m_SceneWindow->Width () - 330, m_SceneWindow->Height () - 65);
        acceptButton->onPress.Add (this, &EditorActorWindow::OnAccept);

        m_ScriptWindow = new EditorActorScriptWindow (m_Editor, canvas);
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
            //            Gwk::Controls::Base::List& childNodes = m_SpeechesTree->GetChildNodes ();
            //            std::string oldName = "";

            //            for (Gwk::Controls::Base* control : childNodes)
            //            {
            //                Gwk::Controls::TreeNode* node = (Gwk::Controls::TreeNode*)control;

            //                if (node->IsSelected ())
            //                {
            //                    oldName = node->GetText ();
            //                    break;
            //                }
            //            }

            //            bool ret = m_Editor->m_EditorSpeechMode.AddOrUpdateSpeech (oldName);

            //            if (ret)
            //            {
            //                m_NameTextBox->SetText ("");
            //                m_TextData->SetText ("");
            //                m_LanguageCombo->SelectItemByName ("EN");
            //                m_Editor->m_EditorSpeechMode.Clear ();
            //            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    void EditorActorWindow::OnRemove ()
    {
        if (m_NameTextBox->GetText () != "")
        {
            m_NameTextBox->SetText ("");
        }
    }

    //--------------------------------------------------------------------------------------------------

    void EditorActorWindow::OnAddScript ()
    {
        std::function<bool(void)> AcceptFunc = [&] {
            if (m_ScriptWindow->Name != "" && m_ScriptWindow->Path != "")
            {
                Gwk::Controls::Property::LabelButton* node =
                  new Gwk::Controls::Property::LabelButton (m_ScriptSection, m_ScriptWindow->Path, "X");
                node->FuncButton->onPress.Add (this, &EditorActorWindow::OnRemoveScript);

                m_ScriptSection->Add (m_ScriptWindow->Name, node);

                return true;
            }

            return false;
        };

        m_ScriptWindow->Show (AcceptFunc, nullptr);
    }

    //--------------------------------------------------------------------------------------------------

    void EditorActorWindow::OnRemoveScript (Gwk::Controls::Base* control)
    {
        Gwk::Controls::Button* button = (Gwk::Controls::Button*)control;
        Gwk::Controls::Base::List& childNodes = m_ScriptSection->GetChildren ();

        for (Gwk::Controls::Base* control : childNodes)
        {
            Gwk::Controls::PropertyRow* node = (Gwk::Controls::PropertyRow*)control;
            Gwk::Controls::Property::LabelButton* property = (Gwk::Controls::Property::LabelButton*)node->GetProperty ();

            if (property->FuncButton == button)
            {
                m_ScriptSection->GetChildren ().remove (control);
                break;
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    void EditorActorWindow::OnAccept () { m_SceneWindow->CloseButtonPressed (); }

    //--------------------------------------------------------------------------------------------------

    void EditorActorWindow::OnCancel () { m_SceneWindow->CloseButtonPressed (); }

    //--------------------------------------------------------------------------------------------------
}