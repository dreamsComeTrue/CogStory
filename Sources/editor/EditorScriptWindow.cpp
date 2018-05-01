// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "EditorScriptWindow.h"
#include "Editor.h"
#include "EditorWindows.h"
#include "MainLoop.h"
#include "Screen.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    EditorScriptWindow::EditorScriptWindow (Editor* editor, Gwk::Controls::Canvas* canvas)
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
        nameTextBox->SetText (m_Name);
        nameTextBox->SetTextColor (Gwk::Colors::White);
        nameTextBox->SetWidth (300);
        nameTextBox->SetPos (nameLabel->Right () + 10, nameLabel->Y ());
        nameTextBox->onTextChanged.Add (this, &EditorScriptWindow::OnNameEdit);

        Gwk::Controls::Label* pathLabel = new Gwk::Controls::Label (m_SceneWindow);
        pathLabel->SetPos (20, nameLabel->Bottom () + 10);
        pathLabel->SetText ("Path:");
        pathLabel->SizeToContents ();

        m_PathTextBox = new Gwk::Controls::TextBox (m_SceneWindow);
        m_PathTextBox->SetText (m_Path);
        m_PathTextBox->SetTextColor (Gwk::Colors::White);
        m_PathTextBox->SetWidth (300);
        m_PathTextBox->SetPos (pathLabel->Right () + 19, pathLabel->Y ());
        m_PathTextBox->onTextChanged.Add (this, &EditorScriptWindow::OnPathEdit);

        Gwk::Controls::Button* browseButton = new Gwk::Controls::Button (m_SceneWindow);
        browseButton->SetText ("BROWSE");
        browseButton->SetPos (m_PathTextBox->Right () + 5, pathLabel->Y ());
        browseButton->onPress.Add (this, &EditorScriptWindow::OnBrowse);

        Gwk::Controls::Button* yesButton = new Gwk::Controls::Button (m_SceneWindow);
        yesButton->SetText ("ACCEPT");
        yesButton->SetPos (m_SceneWindow->Width () / 2 - 2 * 50 - 5, m_SceneWindow->Height () - 60);
        yesButton->onPress.Add (this, &EditorScriptWindow::OnAccept);

        Gwk::Controls::Button* noButton = new Gwk::Controls::Button (m_SceneWindow);
        noButton->SetText ("CANCEL");
        noButton->SetPos (yesButton->Right () + 10, m_SceneWindow->Height () - 60);
        noButton->onPress.Add (this, &EditorScriptWindow::OnCancel);
    }

    //--------------------------------------------------------------------------------------------------

    EditorScriptWindow ::~EditorScriptWindow () { SAFE_DELETE (m_SceneWindow); }

    //--------------------------------------------------------------------------------------------------

    void EditorScriptWindow::Show (std::function<bool(void)> OnAcceptFunc, std::function<bool(void)> OnCancelFunc)
    {
        m_OnAcceptFunc = OnAcceptFunc;
        m_OnCancelFunc = OnCancelFunc;

        m_SceneWindow->SetPosition (Gwk::Position::Center);
        m_SceneWindow->SetHidden (false);
        m_SceneWindow->MakeModal (true);
    }

    //--------------------------------------------------------------------------------------------------

    void EditorScriptWindow::OnBrowse ()
    {
        std::string path = GetDataPath () + "scripts/";

        ALLEGRO_FILECHOOSER* fileOpenDialog
            = al_create_native_file_dialog (path.c_str (), "Open script file", "*.script", 0);

        if (al_show_native_file_dialog (m_Editor->GetMainLoop ()->GetScreen ()->GetDisplay (), fileOpenDialog))
        {
            m_FileName = al_get_native_file_dialog_path (fileOpenDialog, 0);
            std::replace (m_FileName.begin (), m_FileName.end (), '\\', '/');

            if (!EndsWith (m_FileName, ".script"))
            {
                m_FileName += ".script";
            }

            std::string dataPath = "Data/scripts/";
            size_t index = m_FileName.find (dataPath);

            if (index != std::string::npos)
            {
                m_FileName = m_FileName.substr (index + dataPath.length ());
            }

            m_PathTextBox->SetText (m_FileName);
        }

        al_destroy_native_file_dialog (fileOpenDialog);
    }

    //--------------------------------------------------------------------------------------------------

    void EditorScriptWindow::OnAccept ()
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

    void EditorScriptWindow::OnCancel ()
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

    void EditorScriptWindow::OnNameEdit (Gwk::Controls::Base* control)
    {
        Gwk::Controls::TextBox* nameTextBox = (Gwk::Controls::TextBox*)control;

        m_Name = nameTextBox->GetText ();
    }

    //--------------------------------------------------------------------------------------------------

    void EditorScriptWindow::OnPathEdit (Gwk::Controls::Base* control)
    {
        Gwk::Controls::TextBox* pathTextBox = (Gwk::Controls::TextBox*)control;

        m_Path = pathTextBox->GetText ();
    }

    //--------------------------------------------------------------------------------------------------

    bool EditorScriptWindow::GetResult () { return m_Result; }

    //--------------------------------------------------------------------------------------------------
}
