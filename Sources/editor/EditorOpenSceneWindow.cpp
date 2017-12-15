// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "EditorOpenSceneWindow.h"
#include "Editor.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    EditorOpenSceneWindow::EditorOpenSceneWindow (
        Editor* editor, Gwk::Controls::Canvas* canvas, const std::string& fileName)
        : m_Editor (editor)
        , m_FileName (fileName)
    {
        m_SceneWindow = new Gwk::Controls::WindowControl (canvas);
        m_SceneWindow->SetTitle ("Open Scene");
        m_SceneWindow->SetSize (450, 120);
        m_SceneWindow->CloseButtonPressed ();

        //   EditorOpenSceneWindow->SetDeleteOnClose (true);

        Gwk::Controls::Label* pathLabel = new Gwk::Controls::Label (m_SceneWindow);
        pathLabel->SetPos (20, 10);
        pathLabel->SetText ("File path:");
        pathLabel->SizeToContents ();

        Gwk::Controls::TextBox* pathTextBox = new Gwk::Controls::TextBox (m_SceneWindow);
        pathTextBox->SetText (m_FileName);
        pathTextBox->SetTextColor (Gwk::Colors::White);
        pathTextBox->SetWidth (300);
        pathTextBox->SetPos (20, 30);
        pathTextBox->onTextChanged.Add (this, &EditorOpenSceneWindow::OnEdit);

        Gwk::Controls::Button* browseButton = new Gwk::Controls::Button (m_SceneWindow);
        browseButton->SetText ("BROWSE");
        browseButton->SetPos (330, 30);
        browseButton->onPress.Add (this, &EditorOpenSceneWindow::OnOpen);

        Gwk::Controls::Button* okButton = new Gwk::Controls::Button (m_SceneWindow);
        okButton->SetText ("OPEN");
        okButton->SetPos (120, 60);
        okButton->onPress.Add (this, &EditorOpenSceneWindow::OnOpen);

        Gwk::Controls::Button* cancelButton = new Gwk::Controls::Button (m_SceneWindow);
        cancelButton->SetText ("CANCEL");
        cancelButton->SetPos (okButton->GetPos ().x + okButton->GetSize ().x + 10, 60);
        cancelButton->onPress.Add (this, &EditorOpenSceneWindow::OnCancel);
    }

    //--------------------------------------------------------------------------------------------------

    void EditorOpenSceneWindow::Show ()
    {
        m_SceneWindow->SetPosition (Gwk::Position::Center);
        m_SceneWindow->SetHidden (false);
        m_SceneWindow->MakeModal (true);
    }

    //--------------------------------------------------------------------------------------------------

    void EditorOpenSceneWindow::OnOpen (Gwk::Controls::Base*)
    {
        m_Editor->LoadScene (m_FileName);
        m_SceneWindow->CloseButtonPressed ();
    }

    //--------------------------------------------------------------------------------------------------

    void EditorOpenSceneWindow::OnCancel () { m_SceneWindow->CloseButtonPressed (); }

    //--------------------------------------------------------------------------------------------------

    void EditorOpenSceneWindow::OnEdit (Gwk::Controls::Base* control)
    {
        Gwk::Controls::TextBox* textbox = (Gwk::Controls::TextBox*)(control);
        m_FileName = textbox->GetText ();
    }

    //--------------------------------------------------------------------------------------------------
}
