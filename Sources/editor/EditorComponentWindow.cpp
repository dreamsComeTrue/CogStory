// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "EditorComponentWindow.h"
#include "Editor.h"
#include "EditorWindows.h"
#include "MainLoop.h"
#include "ActorFactory.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    EditorComponentWindow::EditorComponentWindow (Editor* editor, Gwk::Controls::Canvas* canvas)
    {
        m_Result = true;
        m_Editor = editor;

        m_SceneWindow = new Gwk::Controls::WindowControl (canvas);
        m_SceneWindow->SetTitle ("Component");
        m_SceneWindow->SetSize (490, 120);
        m_SceneWindow->CloseButtonPressed ();

        Gwk::Controls::Label* nameLabel = new Gwk::Controls::Label (m_SceneWindow);
        nameLabel->SetBounds (20, 10, m_SceneWindow->Width () - 40, m_SceneWindow->Height () - 40);
        nameLabel->SetText ("Name:");
        nameLabel->SizeToContents ();

        Gwk::Controls::TextBox* nameTextBox = new Gwk::Controls::TextBox (m_SceneWindow);
        nameTextBox->SetText (m_Name);
        nameTextBox->SetTextColor (Gwk::Colors::White);
        nameTextBox->SetWidth (300);
        nameTextBox->SetPos (nameLabel->Right () + 10, nameLabel->Y ());
        nameTextBox->onTextChanged.Add (this, &EditorComponentWindow::OnNameEdit);

        Gwk::Controls::Label* typeLabel = new Gwk::Controls::Label (m_SceneWindow);
        typeLabel->SetPos (20, nameLabel->Bottom () + 10);
        typeLabel->SetText ("Type:");
        typeLabel->SizeToContents ();

        Gwk::Controls::ComboBox* typeComboBox = new Gwk::Controls::ComboBox (m_SceneWindow);

        std::vector<std::string>& componentTypes = ActorFactory::GetActorComponents ();

        for (const std::string& type : componentTypes)
        {
            typeComboBox->AddItem (type, type);
        }

        typeComboBox->SetTextColor (Gwk::Colors::White);
        typeComboBox->SetWidth (300);
        typeComboBox->SetPos (typeLabel->Right () + 19, typeLabel->Y ());
        typeComboBox->onSelection.Add (this, &EditorComponentWindow::OnTypeEdit);

        Gwk::Controls::Button* yesButton = new Gwk::Controls::Button (m_SceneWindow);
        yesButton->SetText ("ACCEPT");
        yesButton->SetPos (m_SceneWindow->Width () / 2 - 2 * 50 - 5, m_SceneWindow->Height () - 60);
        yesButton->onPress.Add (this, &EditorComponentWindow::OnAccept);

        Gwk::Controls::Button* noButton = new Gwk::Controls::Button (m_SceneWindow);
        noButton->SetText ("CANCEL");
        noButton->SetPos (yesButton->Right () + 10, m_SceneWindow->Height () - 60);
        noButton->onPress.Add (this, &EditorComponentWindow::OnCancel);

        m_Type = typeComboBox->GetSelectedItem ()->GetText ();
    }

    //--------------------------------------------------------------------------------------------------

    EditorComponentWindow ::~EditorComponentWindow () { SAFE_DELETE (m_SceneWindow); }

    //--------------------------------------------------------------------------------------------------

    void EditorComponentWindow::Show (std::function<bool(void)> OnAcceptFunc, std::function<bool(void)> OnCancelFunc)
    {
        m_OnAcceptFunc = OnAcceptFunc;
        m_OnCancelFunc = OnCancelFunc;

        m_SceneWindow->SetPosition (Gwk::Position::Center);
        m_SceneWindow->SetHidden (false);
        m_SceneWindow->MakeModal (true);
    }

    //--------------------------------------------------------------------------------------------------

    void EditorComponentWindow::OnAccept ()
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

    void EditorComponentWindow::OnCancel ()
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

    void EditorComponentWindow::OnNameEdit (Gwk::Controls::Base* control)
    {
        Gwk::Controls::TextBox* nameTextBox = (Gwk::Controls::TextBox*)control;

        m_Name = nameTextBox->GetText ();
    }

    //--------------------------------------------------------------------------------------------------

    void EditorComponentWindow::OnTypeEdit (Gwk::Controls::Base* control)
    {
        Gwk::Controls::ComboBox* typeComboBox = (Gwk::Controls::ComboBox*)control;

        m_Type = typeComboBox->GetSelectedItem ()->GetText ();
    }

    //--------------------------------------------------------------------------------------------------

    bool EditorComponentWindow::GetResult () { return m_Result; }

    //--------------------------------------------------------------------------------------------------
}

