// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "EditorTriggerAreaWindow.h"
#include "Editor.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    EditorTriggerAreaWindow::EditorTriggerAreaWindow (Editor* editor, Gwk::Controls::Canvas* canvas)
        : m_Editor (editor)
    {
        m_SceneWindow = new Gwk::Controls::WindowControl (canvas);
        m_SceneWindow->SetTitle ("Trigger Area");
        m_SceneWindow->SetSize (450, 120);
        m_SceneWindow->CloseButtonPressed ();

        Gwk::Controls::Label* pathLabel = new Gwk::Controls::Label (m_SceneWindow);
        pathLabel->SetPos (20, 10);
        pathLabel->SetText ("Name:");
        pathLabel->SizeToContents ();

        Gwk::Controls::TextBox* pathTextBox = new Gwk::Controls::TextBox (m_SceneWindow);
        pathTextBox->SetText (m_Editor->GetEditorTriggerAreaMode ().GetTriggerAreaName ());
        pathTextBox->SetTextColor (Gwk::Colors::White);
        pathTextBox->SetWidth (350);
        pathTextBox->SetPos (20, 30);
        pathTextBox->onTextChanged.Add (this, &EditorTriggerAreaWindow::OnEdit);

        Gwk::Controls::Button* okButton = new Gwk::Controls::Button (m_SceneWindow);
        okButton->SetText ("ACCEPT");
        okButton->SetPos (120, 60);
        okButton->onPress.Add (this, &EditorTriggerAreaWindow::OnAccept);

        Gwk::Controls::Button* cancelButton = new Gwk::Controls::Button (m_SceneWindow);
        cancelButton->SetText ("CANCEL");
        cancelButton->SetPos (okButton->GetPos ().x + okButton->GetSize ().x + 10, 60);
        cancelButton->onPress.Add (this, &EditorTriggerAreaWindow::OnCancel);
    }

    //--------------------------------------------------------------------------------------------------

    void EditorTriggerAreaWindow::Show ()
    {
        m_Editor->GetEditorTriggerAreaMode ().SetTriggerAreaName ("");

        m_SceneWindow->SetPosition (Gwk::Position::Center);
        m_SceneWindow->SetHidden (false);
        m_SceneWindow->MakeModal (true);
    }

    //--------------------------------------------------------------------------------------------------

    void EditorTriggerAreaWindow::OnAccept (Gwk::Controls::Base*)
    {
        m_Editor->SetCursorMode (CursorMode::EditTriggerAreaMode);
        m_Editor->GetEditorTriggerAreaMode ().NewTriggerArea ();
        m_SceneWindow->CloseButtonPressed ();
    }

    //--------------------------------------------------------------------------------------------------

    void EditorTriggerAreaWindow::OnCancel ()
    {
        m_Editor->SetCursorMode (CursorMode::ActorSelectMode);
        m_Editor->GetEditorTriggerAreaMode ().SetTriggerAreaName ("");
        m_SceneWindow->CloseButtonPressed ();
    }

    //--------------------------------------------------------------------------------------------------

    void EditorTriggerAreaWindow::OnEdit (Gwk::Controls::Base* control)
    {
        Gwk::Controls::TextBox* textbox = (Gwk::Controls::TextBox*)(control);
        m_Editor->GetEditorTriggerAreaMode ().SetTriggerAreaName (textbox->GetText ());
    }

    //--------------------------------------------------------------------------------------------------
}
