// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "EditorFlagPointWindow.h"
#include "Editor.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    FlagPointWindow::FlagPointWindow (Editor* editor, Gwk::Controls::Canvas* canvas) :
        m_Editor (editor)
    {
        m_SceneWindow = new Gwk::Controls::WindowControl (canvas);
        m_SceneWindow->SetTitle ("Flag Point");
        m_SceneWindow->SetSize (450, 120);
        m_SceneWindow->CloseButtonPressed ();

        Gwk::Controls::Label* pathLabel = new Gwk::Controls::Label (m_SceneWindow);
        pathLabel->SetPos (20, 10);
        pathLabel->SetText ("Name:");
        pathLabel->SizeToContents ();

        Gwk::Controls::TextBox* pathTextBox = new Gwk::Controls::TextBox (m_SceneWindow);
        pathTextBox->SetText (m_Editor->m_EditorFlagPointMode.m_FlagPointName);
        pathTextBox->SetTextColor (Gwk::Colors::White);
        pathTextBox->SetWidth (350);
        pathTextBox->SetPos (20, 30);
        pathTextBox->onTextChanged.Add (this, &FlagPointWindow::OnEdit);

        Gwk::Controls::Button* okButton = new Gwk::Controls::Button (m_SceneWindow);
        okButton->SetText ("ACCEPT");
        okButton->SetPos (120, 60);
        okButton->onPress.Add (this, &FlagPointWindow::OnAccept);

        Gwk::Controls::Button* cancelButton = new Gwk::Controls::Button (m_SceneWindow);
        cancelButton->SetText ("CANCEL");
        cancelButton->SetPos (okButton->GetPos ().x + okButton->GetSize ().x + 10, 60);
        cancelButton->onPress.Add (this, &FlagPointWindow::OnCancel);
    }

    //--------------------------------------------------------------------------------------------------

    void FlagPointWindow::Show ()
    {
        m_Editor->m_EditorFlagPointMode.m_AskFlagPoint = false;

        m_SceneWindow->SetPosition (Gwk::Position::Center);
        m_SceneWindow->SetHidden (false);
        m_SceneWindow->MakeModal (true);
    }

    //--------------------------------------------------------------------------------------------------

    void FlagPointWindow::OnAccept (Gwk::Controls::Base*)
    {
        m_Editor->m_CursorMode = CursorMode::EditFlagPointsMode;
        m_SceneWindow->CloseButtonPressed ();
    }

    //--------------------------------------------------------------------------------------------------

    void FlagPointWindow::OnCancel ()
    {
        m_Editor->m_CursorMode = CursorMode::TileSelectMode;
        m_Editor->m_EditorFlagPointMode.m_FlagPointName = "";
        m_SceneWindow->CloseButtonPressed ();
    }

    //--------------------------------------------------------------------------------------------------

    void FlagPointWindow::OnEdit (Gwk::Controls::Base* control)
    {
        Gwk::Controls::TextBox* textbox = (Gwk::Controls::TextBox*)(control);
        m_Editor->m_EditorFlagPointMode.m_FlagPointName = textbox->GetText ();
    }

    //--------------------------------------------------------------------------------------------------
}


