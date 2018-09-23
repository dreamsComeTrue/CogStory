// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "EditorScriptWindow.h"
#include "Editor.h"
#include "EditorWindows.h"
#include "MainLoop.h"
#include "Screen.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    EditorScriptWindow::EditorScriptWindow (Editor* editor)
        : m_Editor (editor)
    {
    }

    //--------------------------------------------------------------------------------------------------

    EditorScriptWindow ::~EditorScriptWindow () {}

    //--------------------------------------------------------------------------------------------------

    void EditorScriptWindow::OnBrowse ()
    {
        std::string path = GetDataPath () + "scripts/x/";

        ALLEGRO_FILECHOOSER* fileOpenDialog
            = al_create_native_file_dialog (path.c_str (), "Open script file", "*.script", 0);

        if (al_show_native_file_dialog (m_Editor->GetMainLoop ()->GetScreen ()->GetDisplay (), fileOpenDialog)
            && al_get_native_file_dialog_count (fileOpenDialog) > 0)
        {
            std::string fileName = al_get_native_file_dialog_path (fileOpenDialog, 0);
            std::replace (fileName.begin (), fileName.end (), '\\', '/');

            if (!EndsWith (fileName, ".script"))
            {
                fileName += ".script";
            }

            std::string dataPath = "Data/scripts/";
            size_t index = fileName.find (dataPath);

            if (index != std::string::npos)
            {
                fileName = fileName.substr (index + dataPath.length ());
            }

            strcpy (m_Path, fileName.c_str ());
        }

        al_destroy_native_file_dialog (fileOpenDialog);
    }

    //--------------------------------------------------------------------------------------------------

    void EditorScriptWindow::Render ()
    {
        if (ImGui::BeginPopupModal ("Script", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            if (m_Name[0] == '\0')
            {
            }

            ImGui::InputText ("Name", m_Name, IM_ARRAYSIZE (m_Name));
            ImGui::SetItemDefaultFocus ();
            ImGui::InputText ("Path", m_Path, IM_ARRAYSIZE (m_Path));
            ImGui::SameLine ();

            if (ImGui::Button ("BROWSE", ImVec2 (50.f, 18)))
            {
                OnBrowse ();
            }

            ImGui::Separator ();

            ImGui::BeginGroup ();

            if (ImGui::Button ("ACCEPT", ImVec2 (50.f, 18.f)))
            {
                ImGui::CloseCurrentPopup ();

                if (GetName () != "" && GetPath () != "")
                {
                    m_Editor->GetMainLoop ()->GetSceneManager ().GetActiveScene ()->AttachScript (
                        GetName (), GetPath ());
                }
            }

            ImGui::SameLine ();

            if (ImGui::Button ("CANCEL", ImVec2 (50.f, 18.f)) || m_Editor->IsCloseCurrentPopup ())
            {
                ImGui::CloseCurrentPopup ();

                m_Editor->SetCloseCurrentPopup (false);
            }
            ImGui::EndGroup ();

            ImGui::EndPopup ();
        }
    }

    //--------------------------------------------------------------------------------------------------
}
