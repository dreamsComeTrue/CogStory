// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "EditorOpenSceneWindow.h"
#include "Editor.h"
#include "MainLoop.h"
#include "Screen.h"

#include "imgui.h"
#include "imgui_internal.h"

//--------------------------------------------------------------------------------------------------

namespace ImGui
{
    static bool HoverableItems_ArrayGetter (void* data, int idx, const char** out_text)
    {
        const char* const* items = static_cast<const char* const*> (data);
        if (out_text)
            *out_text = items[idx];
        return true;
    }

    bool HoverableListBox (const char* label, int* current_item, const char* const items[], int items_count,
        int height_in_items, int* hovered_item)
    {
        if (!ListBoxHeader (label, items_count, height_in_items))
            return false;

        // Assume all items have even height (= 1 line of text). If you need items of different or variable sizes you
        // can create a custom version of ListBox() in your code without using the clipper.
        ImGuiContext& g = *GImGui;
        bool value_changed = false;
        ImGuiListClipper clipper (
            items_count, GetTextLineHeightWithSpacing ()); // We know exactly our line height here so we pass it as a
                                                           // minor optimization, but generally you don't need to.
        while (clipper.Step ())
            for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
            {
                const bool item_selected = (i == *current_item);
                const char* item_text;
                if (!HoverableItems_ArrayGetter ((void*)items, i, &item_text))
                    item_text = "*Unknown item*";

                PushID (i);
                if (Selectable (item_text, item_selected))
                {
                    *current_item = i;
                    value_changed = true;
                }

                if (ImGui::IsItemHovered ())
                {
                    *hovered_item = i;
                }

                if (item_selected)
                    SetItemDefaultFocus ();
                PopID ();
            }
        ListBoxFooter ();
        if (value_changed)
            MarkItemEdited (g.CurrentWindow->DC.LastItemId);

        return value_changed;
    }
}

//--------------------------------------------------------------------------------------------------

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    EditorOpenSceneWindow::EditorOpenSceneWindow (Editor* editor)
        : m_Editor (editor)
        , m_IsVisible (false)
        , m_ItemCurrent (0)
        , m_ScheduleClosed (false)
    {
    }

    //--------------------------------------------------------------------------------------------------

    EditorOpenSceneWindow ::~EditorOpenSceneWindow () {}

    //--------------------------------------------------------------------------------------------------

    void EditorOpenSceneWindow::Show (const std::string& filePath, std::function<void(std::string)> OnAcceptFunc,
        std::function<void(std::string)> OnCancelFunc)
    {
        m_OnAcceptFunc = OnAcceptFunc;
        m_OnCancelFunc = OnCancelFunc;

        memset (m_SceneName, 0, ARRAY_SIZE (m_SceneName));
        strcpy (m_SceneName, filePath.c_str ());

        m_IsVisible = true;
        m_ScheduleClosed = false;
    }

    //--------------------------------------------------------------------------------------------------

    void EditorOpenSceneWindow::OnAccept ()
    {
        if (m_OnAcceptFunc)
        {
            m_OnAcceptFunc (GetSceneName ());
        }
    }

    //--------------------------------------------------------------------------------------------------

    void EditorOpenSceneWindow::OnCancel ()
    {
        if (m_OnCancelFunc)
        {
            m_OnCancelFunc (GetSceneName ());
        }
    }

    //--------------------------------------------------------------------------------------------------

    void EditorOpenSceneWindow::ProcessEvent (ALLEGRO_EVENT* event)
    {
        switch (event->keyboard.keycode)
        {
        case ALLEGRO_KEY_UP:
            --m_ItemCurrent;

            if (m_ItemCurrent < 0)
            {
                m_ItemCurrent = m_RecentFileNames.size () - 1;
            }
            break;

        case ALLEGRO_KEY_DOWN:
            ++m_ItemCurrent;

            if (m_ItemCurrent >= m_RecentFileNames.size ())
            {
                m_ItemCurrent = 0;
            }
            break;

        case ALLEGRO_KEY_ENTER:
        case ALLEGRO_KEY_SPACE:
            strcpy (m_SceneName, m_RecentFileNames[m_ItemCurrent].c_str ());
            m_Editor->LoadScene (m_SceneName);
            m_ScheduleClosed = true;

            break;
        }
    }

    //--------------------------------------------------------------------------------------------------

    void EditorOpenSceneWindow::Render ()
    {
        ImGui::SetNextWindowSize (ImVec2 (400, 260));

        if (ImGui::BeginPopupModal ("Open Scene", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            char* items[m_RecentFileNames.size ()];

            for (size_t i = 0; i < m_RecentFileNames.size (); ++i)
            {
                char* fileName = const_cast<char*> (m_RecentFileNames[i].c_str ());
                items[i] = fileName;
            }

            static int hoveredItem = 0;
            ImGui::PushItemWidth (330);
            if (ImGui::HoverableListBox ("", &m_ItemCurrent, items, ARRAY_SIZE (items), 10, &hoveredItem))
            {
                strcpy (m_SceneName, items[m_ItemCurrent]);
                ImGui::CloseCurrentPopup ();

                m_Editor->LoadScene (m_SceneName);
            }

            if (ImGui::IsItemClicked (1))
            {
                m_RecentFileNames.erase (m_RecentFileNames.begin () + hoveredItem);
            }

            ImGui::PopItemWidth ();

            ImGui::PushItemWidth (330);
            ImGui::InputText ("", m_SceneName, ARRAY_SIZE (m_SceneName));
            ImGui::PopItemWidth ();
            ImGui::SetItemDefaultFocus ();
            ImGui::SameLine ();

            if (ImGui::Button ("BROWSE", ImVec2 (50.f, 18.f)))
            {
                std::string path = GetDataPath () + "scenes/x/";

                ALLEGRO_FILECHOOSER* fileOpenDialog
                    = al_create_native_file_dialog (path.c_str (), "Open scene file", "*.scn", 0);

                if (al_show_native_file_dialog (m_Editor->GetMainLoop ()->GetScreen ()->GetDisplay (), fileOpenDialog)
                    && al_get_native_file_dialog_count (fileOpenDialog) > 0)
                {
                    std::string fileName = al_get_native_file_dialog_path (fileOpenDialog, 0);
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

                al_destroy_native_file_dialog (fileOpenDialog);
            }

            ImGui::Separator ();
            ImGui::BeginGroup ();

            if (ImGui::Button ("OPEN", ImVec2 (50.f, 18.f)))
            {
                ImGui::CloseCurrentPopup ();
                m_IsVisible = false;

                m_Editor->LoadScene (m_SceneName);
            }

            ImGui::SameLine ();

            if (ImGui::Button ("CANCEL", ImVec2 (50.f, 18.f)) || m_Editor->IsCloseCurrentPopup ())
            {
                ImGui::CloseCurrentPopup ();
                m_IsVisible = false;

                m_Editor->SetCloseCurrentPopup (false);
            }

            if (m_ScheduleClosed)
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

    std::string EditorOpenSceneWindow::GetSceneName () const { return m_SceneName; }

    //--------------------------------------------------------------------------------------------------

    bool EditorOpenSceneWindow::IsVisible () { return m_IsVisible; }

    //--------------------------------------------------------------------------------------------------

    std::vector<std::string>& EditorOpenSceneWindow::GetRecentFileNames () { return m_RecentFileNames; }

    //--------------------------------------------------------------------------------------------------

    void EditorOpenSceneWindow::AddRecentFileName (const std::string& name) { m_RecentFileNames.push_back (name); }

    //--------------------------------------------------------------------------------------------------
}
