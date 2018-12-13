// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "EditorSaveSceneWindow.h"
#include "Editor.h"
#include "MainLoop.h"
#include "Screen.h"

#include "imgui.h"

//--------------------------------------------------------------------------------------------------

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    EditorSaveSceneWindow::EditorSaveSceneWindow (Editor* editor)
        : m_Editor (editor)
        , m_IsVisible (false)
    {
    }

    //--------------------------------------------------------------------------------------------------

    EditorSaveSceneWindow ::~EditorSaveSceneWindow () {}

    //--------------------------------------------------------------------------------------------------

    void EditorSaveSceneWindow::Show (const std::string& filePath, std::function<void(std::string)> OnAcceptFunc,
        std::function<void(std::string)> OnCancelFunc)
    {
        m_OnAcceptFunc = OnAcceptFunc;
        m_OnCancelFunc = OnCancelFunc;

        memset (m_SceneName, 0, ARRAY_SIZE (m_SceneName));
        strcpy (m_SceneName, filePath.c_str ());

        m_IsVisible = true;
    }

    //--------------------------------------------------------------------------------------------------

    void EditorSaveSceneWindow::OnAccept ()
    {
        if (m_OnAcceptFunc)
        {
            m_OnAcceptFunc (GetSceneName ());
        }
    }

    //--------------------------------------------------------------------------------------------------

    void EditorSaveSceneWindow::OnCancel ()
    {
        if (m_OnCancelFunc)
        {
            m_OnCancelFunc (GetSceneName ());
        }
    }

    //--------------------------------------------------------------------------------------------------

    void EditorSaveSceneWindow::Render ()
    {
        ImGui::SetNextWindowSize (ImVec2 (400, 80));

        if (ImGui::BeginPopupModal ("Save Scene", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::PushItemWidth (330);
            ImGui::InputText ("", m_SceneName, ARRAY_SIZE (m_SceneName));
            ImGui::PopItemWidth ();
            ImGui::SetItemDefaultFocus ();
            ImGui::SameLine ();

            if (ImGui::Button ("BROWSE", ImVec2 (50.f, 18.f)))
            {
                std::string path = GetDataPath () + "scenes/x/";

                ALLEGRO_FILECHOOSER* fileSaveDialog = al_create_native_file_dialog (
                    path.c_str (), "Save scene file", "*.scn", ALLEGRO_FILECHOOSER_SAVE);

                if (al_show_native_file_dialog (m_Editor->GetMainLoop ()->GetScreen ()->GetDisplay (), fileSaveDialog)
                    && al_get_native_file_dialog_count (fileSaveDialog) > 0)
                {
                    std::string fileName = al_get_native_file_dialog_path (fileSaveDialog, 0);
                    std::replace (fileName.begin (), fileName.end (), '\\', '/');

                    if (!EndsWith (fileName, ".scn"))
                    {
                        fileName += ".scn";
                    }

                    std::string dataPath = "Data/scenes/";
                    size_t index = fileName.find (dataPath);

                    if (index != std::string::npos)
                    {
                        fileName = fileName.substr (index + dataPath.length ());
                    }

                    strcpy (m_SceneName, fileName.c_str ());
                }

                al_destroy_native_file_dialog (fileSaveDialog);
            }

            ImGui::Separator ();
            ImGui::BeginGroup ();

            if (ImGui::Button ("SAVE", ImVec2 (50.f, 18.f)))
            {
                ImGui::CloseCurrentPopup ();
                m_IsVisible = false;

                m_Editor->SaveScene (m_SceneName);
            }

            ImGui::SameLine ();

            if (ImGui::Button ("CANCEL", ImVec2 (50.f, 18.f)) || m_Editor->IsCloseCurrentPopup ())
            {
                ImGui::CloseCurrentPopup ();
                m_IsVisible = false;

                m_Editor->SetCloseCurrentPopup (false);
            }
            ImGui::EndGroup ();

            ImGui::EndPopup ();
        }
    }

    //--------------------------------------------------------------------------------------------------

    std::string EditorSaveSceneWindow::GetSceneName () const { return m_SceneName; }

    //--------------------------------------------------------------------------------------------------

    bool EditorSaveSceneWindow::IsVisible () { return m_IsVisible; }

    //--------------------------------------------------------------------------------------------------
}
