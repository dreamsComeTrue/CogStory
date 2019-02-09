// Copyright 2017-2019 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "EditorSceneWindow.h"
#include "Editor.h"
#include "EditorScriptWindow.h"
#include "MainLoop.h"
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
                    ImGui::OpenPopup ("Scene Window Alert");
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
        ImGui::SetNextWindowSize (ImVec2 (750, 200));

        if (ImGui::BeginPopupModal ("Scene Settings", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            if (m_SceneName[0] == '\0')
            {
                strcpy (
                    m_SceneName, m_Editor->GetMainLoop ()->GetSceneManager ().GetActiveScene ()->GetName ().c_str ());
            }

            ImGui::InputText ("Scene Name", m_SceneName, ARRAY_SIZE (m_SceneName));
            ImGui::SetItemDefaultFocus ();

            static ImVec4 color = m_BackColor;

            ImGui::ColorEdit4 ("Scene color", (float*)&color, 0);

            if (!AreSame (color.x, m_BackColor.x) || !AreSame (color.y, m_BackColor.y)
                || !AreSame (color.z, m_BackColor.z) || !AreSame (color.w, m_BackColor.w))
            {
                m_BackColor = color;
                m_Editor->GetMainLoop ()->GetSceneManager ().GetActiveScene ()->SetBackgroundColor (
                    m_BackColor.x, m_BackColor.y, m_BackColor.z, m_BackColor.w);
            }

            ImGui::Separator ();

            if (ImGui::Button ("ADD SCRIPT", ImVec2 (730, 18)))
            {
                m_Editor->GetScriptWindow ()->Show (
                    [&](std::string name, std::string path) {
                        m_Editor->GetMainLoop ()->GetSceneManager ().GetActiveScene ()->AttachScript (name, path);
                    },
                    nullptr);
            }

            m_Editor->GetScriptWindow ()->Render ();

            ImGui::Columns (4, "mycolumns");
            ImGui::SetColumnWidth (-1, 230);
            ImGui::Separator ();
            ImGui::Text ("Name");
            ImGui::NextColumn ();
            ImGui::SetColumnWidth (-1, 340);
            ImGui::Text ("Path");
            ImGui::NextColumn ();
            ImGui::SetColumnWidth (-1, 90);
            ImGui::Text ("Reload");
            ImGui::NextColumn ();
            ImGui::SetColumnWidth (-1, 90);
            ImGui::Text ("Remove");
            ImGui::Separator ();

            std::vector<ScriptMetaData>& scripts
                = m_Editor->GetMainLoop ()->GetSceneManager ().GetActiveScene ()->GetScripts ();

            for (ScriptMetaData& script : scripts)
            {
                ImGui::NextColumn ();
                ImGui::SetColumnWidth (-1, 230);
                ImGui::Text (script.Name.c_str ());
                ImGui::NextColumn ();
                ImGui::SetColumnWidth (-1, 340);
                ImGui::Text (script.Path.c_str ());
                ImGui::NextColumn ();
                ImGui::SetColumnWidth (-1, 90);
                if (ImGui::Button ("RELOAD", ImVec2 (80.f, 18.f)))
                {
                    OnReloadScript (script.Name);
                }

                ImGui::NextColumn ();
                ImGui::SetColumnWidth (-1, 90);

                if (ImGui::Button ("REMOVE", ImVec2 (80.f, 18.f)))
                {
                    m_Editor->GetMainLoop ()->GetSceneManager ().GetActiveScene ()->RemoveScript (script.Name);
                }
            }

            ImGui::Columns (1);
            ImGui::Separator ();

            ImGui::BeginGroup ();

            if (ImGui::Button ("ACCEPT", ImVec2 (80.f, 18.f)))
            {
                ImGui::CloseCurrentPopup ();

                m_Editor->GetMainLoop ()->GetSceneManager ().GetActiveScene ()->SetName (m_SceneName);
                m_IsVisible = false;
            }

            ImGui::SameLine ();

            if (ImGui::Button ("CANCEL", ImVec2 (80.f, 18.f)) || m_Editor->IsCloseCurrentPopup ())
            {
                ImGui::CloseCurrentPopup ();
                m_IsVisible = false;

                m_Editor->SetCloseCurrentPopup (false);
            }
            ImGui::EndGroup ();

            //  Alert window
            bool open = true;
            if (ImGui::BeginPopupModal ("Scene Window Alert", &open))
            {
                ImGui::Text (g_ScriptErrorBuffer);
                if (ImGui::Button ("OK", ImVec2 (100, 18)))
                {
                    memset (g_ScriptErrorBuffer, 0, sizeof (g_ScriptErrorBuffer));
                    ImGui::CloseCurrentPopup ();
                }

                ImGui::EndPopup ();
            }
            //  Alert window

            ImGui::EndPopup ();
        }
    }

    //--------------------------------------------------------------------------------------------------

    bool EditorSceneWindow::IsVisible () const { return m_IsVisible; }

    //--------------------------------------------------------------------------------------------------
}
