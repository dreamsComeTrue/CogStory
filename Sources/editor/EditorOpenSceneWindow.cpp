// Copyright 2017-2019 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "EditorOpenSceneWindow.h"
#include "Editor.h"
#include "MainLoop.h"
#include "Screen.h"

#include "addons/tiny-file-dialogs/tinyfiledialogs.h"
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
		, m_ItemCurrent (-1)
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
		m_BrowseButtonPressed = false;

		ComputeGroups ();
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
		RecentFilesGroup& choosenGroup = m_Groups[m_SelectedGroup];

		switch (event->keyboard.keycode)
		{
		case ALLEGRO_KEY_UP:
			--choosenGroup.CurrentItem;

			if (choosenGroup.CurrentItem < 0)
			{
				--m_SelectedGroup;

				if (m_SelectedGroup < 0)
				{
					m_SelectedGroup = m_Groups.size () - 1;
				}

				m_Groups[m_SelectedGroup].CurrentItem = m_Groups[m_SelectedGroup].Paths.size () - 1;
			}
			break;

		case ALLEGRO_KEY_DOWN:
			++choosenGroup.CurrentItem;

			if (choosenGroup.CurrentItem >= choosenGroup.Paths.size ())
			{
				++m_SelectedGroup;

				if (m_SelectedGroup >= m_Groups.size ())
				{
					m_SelectedGroup = 0;
				}

				m_Groups[m_SelectedGroup].CurrentItem = 0;
			}
			break;

		case ALLEGRO_KEY_ENTER:
		case ALLEGRO_KEY_SPACE:
			std::string selectedSceneName = choosenGroup.Name + "/" + choosenGroup.Paths[choosenGroup.CurrentItem];
			strcpy (m_SceneName, selectedSceneName.c_str ());
			m_Editor->LoadScene (m_SceneName);

			break;
		}
	}

	//--------------------------------------------------------------------------------------------------

	void EditorOpenSceneWindow::Render ()
	{
		ImGui::SetNextWindowSize (ImVec2 (530, 490));

		if (ImGui::BeginPopupModal ("Open Scene", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			static int hoveredItem = -1;

			ImGui::BeginChild (
				"Child1", ImVec2 (500, ImGui::GetWindowSize ().y - 90), false, ImGuiWindowFlags_HorizontalScrollbar);
			ImGui::BeginGroup ();
			{
				for (int currentGroup = 0; currentGroup < m_Groups.size (); ++currentGroup)
				{
					RecentFilesGroup& group = m_Groups[currentGroup];

					if (ImGui::TreeNodeEx (group.Name.c_str (), ImGuiTreeNodeFlags_DefaultOpen))
					{
						size_t pathsCount = group.Paths.size ();
						char* items[pathsCount];

						for (int i = 0; i < pathsCount; ++i)
						{
							items[i] = const_cast<char*> (group.Paths[i].c_str ());
						}

						ImGui::PushItemWidth (430);
						if (ImGui::HoverableListBox (std::string ("##" + group.Name).c_str (), &group.CurrentItem,
								items, pathsCount, pathsCount, &hoveredItem))
						{
							std::string selectedSceneName = group.Name + "/" + items[group.CurrentItem];
							strcpy (m_SceneName, selectedSceneName.c_str ());
							m_Editor->LoadScene (m_SceneName);

							m_SelectedGroup = currentGroup;
							m_ScheduleClosed = true;
						}
						ImGui::PopItemWidth ();

						if (ImGui::IsItemClicked (1))
						{
							std::string selectedSceneName = group.Name + "/" + items[hoveredItem];
							std::vector<std::string>::iterator it
								= std::find (m_RecentFileNames.begin (), m_RecentFileNames.end (), selectedSceneName);

							if (it != m_RecentFileNames.end ())
							{
								m_RecentFileNames.erase (it);
								ComputeGroups ();
							}
						}

						ImGui::TreePop ();
					}
				}
			}
			ImGui::EndGroup ();
			ImGui::EndChild ();

			ImGui::PushItemWidth (430);
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
				char const* filterPatterns[1] = {"*.scn"};
				std::string dataPath = GetDataPath ();
				dataPath += GetPathSeparator ();
				dataPath += "scenes";
				dataPath += GetPathSeparator ();
				dataPath += "special";

				const char* chosenPath
					= tinyfd_openFileDialog ("Open Scene", dataPath.c_str (), 1, filterPatterns, "scene files", 0);

				if (chosenPath != nullptr && strlen (chosenPath) > 0)
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
				else
				{
					m_BrowseButtonPressed = false;
				}
			}

			ImGui::Separator ();
			ImGui::BeginGroup ();

			if (ImGui::Button ("OPEN", ImVec2 (80.f, 18.f)))
			{
				ImGui::CloseCurrentPopup ();
				m_IsVisible = false;

				m_Editor->LoadScene (m_SceneName);
			}

			ImGui::SameLine ();

			if (ImGui::Button ("CANCEL", ImVec2 (80.f, 18.f)) || m_Editor->IsCloseCurrentPopup ())
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

	void EditorOpenSceneWindow::ComputeGroups ()
	{
		m_Groups.clear ();
		m_SelectedGroup = 0;

		for (const std::string& recentName : m_RecentFileNames)
		{
			int separatorIndex = recentName.find ("/");
			std::string groupName = recentName.substr (0, separatorIndex);
			bool found = false;

			for (RecentFilesGroup& group : m_Groups)
			{
				if (group.Name == groupName)
				{
					group.Paths.push_back (recentName.substr (separatorIndex + 1));
					group.CurrentItem = -1;
					found = true;
					break;
				}
			}

			if (!found)
			{
				RecentFilesGroup newGroup;
				newGroup.Name = groupName;
				newGroup.Paths.push_back (recentName.substr (separatorIndex + 1));
				newGroup.CurrentItem = -1;

				m_Groups.push_back (newGroup);
			}
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
