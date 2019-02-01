// Copyright 2017-2019 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "EditorSaveSceneWindow.h"
#include "Editor.h"
#include "MainLoop.h"
#include "Screen.h"

#include "imgui.h"
#include "imguifilesystem.h"

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
        m_BrowseButtonPressed = false;
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
        ImGui::SetNextWindowSize (ImVec2 (440, 85));

        if (ImGui::BeginPopupModal ("Save Scene", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::PushItemWidth (330);
            ImGui::InputText ("", m_SceneName, ARRAY_SIZE (m_SceneName));
            ImGui::PopItemWidth ();
            ImGui::SetItemDefaultFocus ();
            ImGui::SameLine ();

            if (ImGui::Button ("BROWSE", ImVec2 (80.f, 18.f)))
            {
                m_BrowseButtonPressed = true;
            }

            if (m_BrowseButtonPressed)
            {
                std::string path = GetDataPath () + "scenes/x/";
                static ImGuiFs::Dialog dlg;
                const char* chosenPath = dlg.saveFileDialog (m_BrowseButtonPressed, path.c_str ());

                if (strlen (chosenPath) > 0)
                {
                    std::string fileName = chosenPath;
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
                    m_BrowseButtonPressed = false;
                }
                else if (dlg.hasUserJustCancelledDialog ())
                {
                    m_BrowseButtonPressed = false;
                }
            }

            ImGui::Separator ();
            ImGui::BeginGroup ();

            if (ImGui::Button ("SAVE", ImVec2 (80.f, 18.f)))
            {
                ImGui::CloseCurrentPopup ();
                m_IsVisible = false;

                m_Editor->SaveScene (m_SceneName);
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

    std::string EditorSaveSceneWindow::GetSceneName () const { return m_SceneName; }

    //--------------------------------------------------------------------------------------------------

    bool EditorSaveSceneWindow::IsVisible () { return m_IsVisible; }

    //--------------------------------------------------------------------------------------------------
}
