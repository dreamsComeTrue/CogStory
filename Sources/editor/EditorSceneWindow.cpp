// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "EditorSceneWindow.h"
#include "Editor.h"
#include "EditorScriptWindow.h"
#include "EditorWindows.h"
#include "MainLoop.h"
#include "Screen.h"
#include "Script.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    EditorSceneWindow::EditorSceneWindow (Editor* editor)
        : m_Editor (editor)
        , m_IsVisible (false)
    {
        memset (m_SceneName, 0, ARRAY_SIZE (m_SceneName));
    }

    //--------------------------------------------------------------------------------------------------

    EditorSceneWindow ::~EditorSceneWindow () {}

    //--------------------------------------------------------------------------------------------------

    void EditorSceneWindow::Show ()
    {
        strcpy (m_SceneName, m_Editor->GetMainLoop ()->GetSceneManager ().GetActiveScene ()->GetName ().c_str ());
        ALLEGRO_COLOR color = m_Editor->GetMainLoop ()->GetSceneManager ().GetActiveScene ()->GetBackgroundColor ();

        m_BackColor = ImVec4 (color.r, color.g, color.b, color.a);
        m_IsVisible = true;
    }

    //--------------------------------------------------------------------------------------------------

    void EditorSceneWindow::OnReloadScript (const std::string& scriptName)
    {
        Scene* scene = m_Editor->GetMainLoop ()->GetSceneManager ().GetActiveScene ();
        std::vector<ScriptMetaData>& scripts = scene->GetScripts ();

        for (ScriptMetaData& script : scripts)
        {
            if (script.Name == scriptName)
            {
                scene->ReloadScript (script.Name);

                if (strlen (g_ScriptErrorBuffer) != 0)
                {
                    m_Editor->GetEditorInfoWindow ()->Show (g_ScriptErrorBuffer);
                    memset (g_ScriptErrorBuffer, 0, sizeof (g_ScriptErrorBuffer));
                }
                else
                {
#ifdef _MSC_VER
                    std::optional<ScriptMetaData> metaScript = scene->GetScriptByName (script.Name);
#else
                    std::experimental::optional<ScriptMetaData> metaScript = scene->GetScriptByName (script.Name);
#endif

                    if (metaScript)
                    {
                        (*metaScript).ScriptObj->Run ("void Start ()");
                    }
                }
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    void EditorSceneWindow::RenderUI ()
    {
        ImGui::SetNextWindowSize (ImVec2 (500, 200));

        if (ImGui::BeginPopupModal ("Scene Settings", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            if (m_SceneName[0] == '\0')
            {
                strcpy (
                    m_SceneName, m_Editor->GetMainLoop ()->GetSceneManager ().GetActiveScene ()->GetName ().c_str ());
            }

            ImGui::InputText ("Scene Name", m_SceneName, IM_ARRAYSIZE (m_SceneName));
            ImGui::SetItemDefaultFocus ();

            static ImVec4 color = m_BackColor;

            ImGui::ColorEdit4 ("Scene color", (float*)&color, 0);

            if ((color.x != m_BackColor.x) || (color.y != m_BackColor.y) || (color.z != m_BackColor.z)
                || (color.w != m_BackColor.w))
            {
                m_BackColor = color;
                m_Editor->GetMainLoop ()->GetSceneManager ().GetActiveScene ()->SetBackgroundColor (
                    m_BackColor.x, m_BackColor.y, m_BackColor.z, m_BackColor.w);
            }

            ImGui::Separator ();

            if (ImGui::Button ("ADD SCRIPT", ImVec2 (480, 18)))
            {
                ImGui::OpenPopup ("Script");
            }

            m_Editor->GetScriptWindow ()->Render ();

            ImGui::Columns (4, "mycolumns");
            ImGui::Separator ();
            ImGui::Text ("Name");
            ImGui::NextColumn ();
            ImGui::SetColumnWidth (-1, 240);
            ImGui::Text ("Path");
            ImGui::NextColumn ();
            ImGui::SetColumnWidth (-1, 60);
            ImGui::Text ("Reload");
            ImGui::NextColumn ();
            ImGui::SetColumnWidth (-1, 60);
            ImGui::Text ("Remove");
            ImGui::Separator ();

            std::vector<ScriptMetaData>& scripts
                = m_Editor->GetMainLoop ()->GetSceneManager ().GetActiveScene ()->GetScripts ();

            for (ScriptMetaData& script : scripts)
            {
                ImGui::NextColumn ();
                ImGui::Text (script.Name.c_str ());
                ImGui::NextColumn ();
                ImGui::Text (script.Path.c_str ());
                ImGui::NextColumn ();
                ImGui::SetColumnWidth (-1, 60);
                if (ImGui::Button ("RELOAD"))
                {
                    OnReloadScript (script.Name);
                }

                ImGui::NextColumn ();
                ImGui::SetColumnWidth (-1, 60);

                if (ImGui::Button ("REMOVE"))
                {
                    m_Editor->GetMainLoop ()->GetSceneManager ().GetActiveScene ()->RemoveScript (script.Name);
                }
            }

            ImGui::Columns (1);
            ImGui::Separator ();

            ImGui::BeginGroup ();

            if (ImGui::Button ("ACCEPT", ImVec2 (50.f, 18.f)))
            {
                ImGui::CloseCurrentPopup ();

                m_Editor->GetMainLoop ()->GetSceneManager ().GetActiveScene ()->SetName (m_SceneName);
                m_IsVisible = false;
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
}
