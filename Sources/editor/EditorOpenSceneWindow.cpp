// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "EditorOpenSceneWindow.h"
#include "Editor.h"
#include "MainLoop.h"
#include "Screen.h"

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

        m_PathTextBox = new Gwk::Controls::TextBox (m_SceneWindow);
        m_PathTextBox->SetText (m_FileName);
        m_PathTextBox->SetTextColor (Gwk::Colors::White);
        m_PathTextBox->SetWidth (300);
        m_PathTextBox->SetPos (20, 30);
        m_PathTextBox->onTextChanged.Add (this, &EditorOpenSceneWindow::OnEdit);

        Gwk::Controls::Button* browseButton = new Gwk::Controls::Button (m_SceneWindow);
        browseButton->SetText ("BROWSE");
        browseButton->SetPos (330, 30);
        browseButton->onPress.Add (this, &EditorOpenSceneWindow::OnBrowse);

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

    void EditorOpenSceneWindow::Show (const std::string& path)
    {
        if (path != "")
        {
            m_PathTextBox->SetText (path);
        }
        else
        {
            m_PathTextBox->SetText (m_FileName);
        }

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

    void EditorOpenSceneWindow::OnBrowse ()
    {
        std::string path = GetDataPath () + "scenes/x/";

        ALLEGRO_FILECHOOSER* fileOpenDialog
            = al_create_native_file_dialog (path.c_str (), "Open scene file", "*.scn", 0);

        if (al_show_native_file_dialog (m_Editor->GetMainLoop ()->GetScreen ()->GetDisplay (), fileOpenDialog))
        {
            m_FileName = al_get_native_file_dialog_path (fileOpenDialog, 0);
            std::replace (m_FileName.begin (), m_FileName.end (), '\\', '/');

            if (!EndsWith (m_FileName, ".scn"))
            {
                m_FileName += ".scn";
            }

            std::string dataPath = "Data/scenes/";
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
}
