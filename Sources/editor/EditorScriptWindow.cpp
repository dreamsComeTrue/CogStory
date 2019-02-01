// Copyright 2017-2019 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "EditorScriptWindow.h"
#include "Editor.h"
#include "MainLoop.h"
#include "Screen.h"

#include "imgui.h"
#include "imguifilesystem.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    EditorScriptWindow::EditorScriptWindow (Editor* editor)
        : m_Editor (editor)
        , m_IsVisible (false)
    {
    }

    //--------------------------------------------------------------------------------------------------

    EditorScriptWindow ::~EditorScriptWindow () {}

    //--------------------------------------------------------------------------------------------------

    void EditorScriptWindow ::Show (std::function<void(std::string, std::string)> OnAcceptFunc,
        std::function<void(std::string, std::string)> OnCancelFunc)
    {
        m_OnAcceptFunc = OnAcceptFunc;
        m_OnCancelFunc = OnCancelFunc;

        memset (m_Name, 0, ARRAY_SIZE (m_Name));
        memset (m_Path, 0, ARRAY_SIZE (m_Path));

        m_IsVisible = true;
        m_BrowseButtonPressed = false;

        ImGui::OpenPopup ("Script");
    }

    //--------------------------------------------------------------------------------------------------

    void EditorScriptWindow::OnAccept ()
    {
        if (m_OnAcceptFunc)
        {
            m_OnAcceptFunc (GetName (), GetPath ());
        }
    }

    //--------------------------------------------------------------------------------------------------

    void EditorScriptWindow::OnCancel ()
    {
        if (m_OnCancelFunc)
        {
            m_OnCancelFunc (GetName (), GetPath ());
        }
    }

    //--------------------------------------------------------------------------------------------------

    void EditorScriptWindow::OnBrowse ()
    {
        if (m_BrowseButtonPressed)
        {
            std::string path = GetDataPath () + "scripts/x/";
            static ImGuiFs::Dialog dlg;
            const char* chosenPath = dlg.chooseFileDialog (m_BrowseButtonPressed, path.c_str ());

            if (strlen (chosenPath) > 0)
            {
                std::string fileName = chosenPath;
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
                m_BrowseButtonPressed = false;
            }
            else if (dlg.hasUserJustCancelledDialog ())
            {
                m_BrowseButtonPressed = false;
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    void EditorScriptWindow::Render ()
    {
        ImGui::SetNextWindowSize (ImVec2 (390, 110));

        if (ImGui::BeginPopupModal ("Script", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            if (m_Name[0] == '\0')
            {
            }

            ImGui::InputText ("Name", m_Name, ARRAY_SIZE (m_Name));
            ImGui::SetItemDefaultFocus ();
            ImGui::InputText ("Path", m_Path, ARRAY_SIZE (m_Path));
            ImGui::SameLine ();

            if (ImGui::Button ("BROWSE", ImVec2 (80.f, 18)))
            {
                m_BrowseButtonPressed = true;
            }

            OnBrowse ();

            ImGui::Separator ();

            ImGui::BeginGroup ();

            if (ImGui::Button ("ACCEPT", ImVec2 (80.f, 18.f)))
            {
                ImGui::CloseCurrentPopup ();
                m_IsVisible = false;

                if (GetName () != "" && GetPath () != "")
                {
                    OnAccept ();
                }
            }

            ImGui::SameLine ();

            if (ImGui::Button ("CANCEL", ImVec2 (80.f, 18.f)) || m_Editor->IsCloseCurrentPopup ())
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

    std::string EditorScriptWindow::GetName () const { return m_Name; }

    //--------------------------------------------------------------------------------------------------

    std::string EditorScriptWindow::GetPath () const { return m_Path; }

    //--------------------------------------------------------------------------------------------------

    bool EditorScriptWindow::IsVisible () { return m_IsVisible; }

    //--------------------------------------------------------------------------------------------------
}
