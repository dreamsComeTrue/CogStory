// Copyright 2017-2019 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "EditorScriptSelectWindow.h"
#include "Editor.h"
#include "MainLoop.h"
#include "Screen.h"

#include "addons/tiny-file-dialogs/tinyfiledialogs.h"
#include "imgui.h"

namespace aga
{
	//--------------------------------------------------------------------------------------------------

	EditorScriptSelectWindow::EditorScriptSelectWindow (Editor* editor)
		: m_Editor (editor)
		, m_IsVisible (false)
	{
	}

	//--------------------------------------------------------------------------------------------------

	EditorScriptSelectWindow ::~EditorScriptSelectWindow () {}

	//--------------------------------------------------------------------------------------------------

	void EditorScriptSelectWindow ::Show (std::function<void(std::string, std::string)> OnAcceptFunc,
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

	void EditorScriptSelectWindow::OnAccept ()
	{
		if (m_OnAcceptFunc)
		{
			m_OnAcceptFunc (GetName (), GetPath ());
		}
	}

	//--------------------------------------------------------------------------------------------------

	void EditorScriptSelectWindow::OnCancel ()
	{
		if (m_OnCancelFunc)
		{
			m_OnCancelFunc (GetName (), GetPath ());
		}
	}

	//--------------------------------------------------------------------------------------------------

	void EditorScriptSelectWindow::OnBrowse ()
	{
		if (m_BrowseButtonPressed)
		{
			char const* filterPatterns[1] = {"*.script"};

			std::string dataPath = GetDataPath ();
			dataPath += "/scripts/special";

			const char* chosenPath
				= tinyfd_openFileDialog ("Open Scene", dataPath.c_str (), 1, filterPatterns, "script files", 0);

			if (chosenPath != nullptr && strlen (chosenPath) > 0)
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
			else
			{
				m_BrowseButtonPressed = false;
			}
		}
	}

	//--------------------------------------------------------------------------------------------------

	void EditorScriptSelectWindow::Render ()
	{
		ImGui::SetNextWindowSize (ImVec2 (600, 110));

		if (ImGui::BeginPopupModal ("Script", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			if (m_Name[0] == '\0')
			{
			}

			ImGui::PushItemWidth (500);
			ImGui::InputText ("Name", m_Name, ARRAY_SIZE (m_Name));
			ImGui::SetItemDefaultFocus ();
			ImGui::PopItemWidth ();
			ImGui::PushItemWidth (450);
			ImGui::InputText ("Path", m_Path, ARRAY_SIZE (m_Path));
			ImGui::PopItemWidth ();
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

	std::string EditorScriptSelectWindow::GetName () const { return m_Name; }

	//--------------------------------------------------------------------------------------------------

	std::string EditorScriptSelectWindow::GetPath () const { return m_Path; }

	//--------------------------------------------------------------------------------------------------

	bool EditorScriptSelectWindow::IsVisible () { return m_IsVisible; }

	//--------------------------------------------------------------------------------------------------
}
