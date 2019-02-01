// Copyright 2017-2019 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "EditorComponentWindow.h"
#include "ActorFactory.h"
#include "Editor.h"

#include "imgui.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    EditorComponentWindow::EditorComponentWindow (Editor* editor)
        : m_Editor (editor)
        , m_IsVisible (false)
    {
    }

    //--------------------------------------------------------------------------------------------------

    EditorComponentWindow ::~EditorComponentWindow () {}

    //--------------------------------------------------------------------------------------------------

    void EditorComponentWindow::Show (std::function<void(std::string, std::string)> OnAcceptFunc,
        std::function<void(std::string, std::string)> OnCancelFunc)
    {
        m_OnAcceptFunc = OnAcceptFunc;
        m_OnCancelFunc = OnCancelFunc;

        memset (m_Name, 0, ARRAY_SIZE (m_Name));
        memset (m_Type, 0, ARRAY_SIZE (m_Type));

        m_IsVisible = true;

        m_SelectedType = 0;
        m_Types = ActorFactory::GetActorComponents ();

        ImGui::OpenPopup ("Component");
    }

    //--------------------------------------------------------------------------------------------------

    void EditorComponentWindow::OnAccept ()
    {
        if (m_OnAcceptFunc)
        {
            m_OnAcceptFunc (GetName (), GetTypeName ());
        }
    }

    //--------------------------------------------------------------------------------------------------

    void EditorComponentWindow::OnCancel ()
    {
        if (m_OnCancelFunc)
        {
            m_OnCancelFunc (GetName (), GetTypeName ());
        }
    }

    //--------------------------------------------------------------------------------------------------

    void EditorComponentWindow::Render ()
    {
        if (ImGui::BeginPopupModal ("Component", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::InputText ("Name", m_Name, ARRAY_SIZE (m_Name));
            ImGui::SetItemDefaultFocus ();
            ImGui::Combo ("Type", &m_SelectedType, m_Types);
            ImGui::SameLine ();

            ImGui::Separator ();

            ImGui::BeginGroup ();

            if (ImGui::Button ("ACCEPT", ImVec2 (50.f, 18.f)))
            {
                ImGui::CloseCurrentPopup ();
                m_IsVisible = false;

                if (GetName () != "" && GetTypeName () != "")
                {
                    OnAccept ();
                }
            }

            ImGui::SameLine ();

            if (ImGui::Button ("CANCEL", ImVec2 (50.f, 18.f)) || m_Editor->IsCloseCurrentPopup ())
            {
                OnCancel ();

                ImGui::CloseCurrentPopup ();
                m_IsVisible = false;

                m_Editor->SetCloseCurrentPopup (false);
            }
            ImGui::EndGroup ();

            ImGui::EndPopup ();
        }
    }

    //--------------------------------------------------------------------------------------------------

    std::string EditorComponentWindow::GetName () const { return m_Name; }

    //--------------------------------------------------------------------------------------------------

    std::string EditorComponentWindow::GetTypeName () const { return m_Types[m_SelectedType]; }

    //--------------------------------------------------------------------------------------------------

    bool EditorComponentWindow::IsVisible () { return m_IsVisible; }

    //--------------------------------------------------------------------------------------------------
}
