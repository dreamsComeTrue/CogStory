// Copyright 2017-2019 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "EditorScriptWindow.h"
#include "Editor.h"
#include "MainLoop.h"
#include "Screen.h"
#include "Script.h"

#include "addons/tiny-file-dialogs/tinyfiledialogs.h"
#include "imgui.h"

namespace aga
{
	//--------------------------------------------------------------------------------------------------

	EditorScriptWindow::EditorScriptWindow (Editor* editor)
		: m_Editor (editor)
		, m_IsVisible (false)
		, m_SelectedEntry (-1)
		, m_OpenButtonPressed (false)
	{
	}

	//--------------------------------------------------------------------------------------------------

	EditorScriptWindow ::~EditorScriptWindow () {}

	//--------------------------------------------------------------------------------------------------

	void EditorScriptWindow::Show () { m_IsVisible = true; }

	//--------------------------------------------------------------------------------------------------

	void EditorScriptWindow::ProcessEvent (ALLEGRO_EVENT* event)
	{
		switch (event->keyboard.keycode)
		{
		case ALLEGRO_KEY_S:
			if (event->keyboard.modifiers == ALLEGRO_KEYMOD_CTRL)
			{
				SaveCurrentFile ();
			}

			break;
		}
	}

	//--------------------------------------------------------------------------------------------------

	void EditorScriptWindow::RenderUI ()
	{
		Point winSize = m_Editor->GetMainLoop ()->GetScreen ()->GetRealWindowSize ();

		ImGui::SetNextWindowSize (ImVec2 (winSize.Width - 100, winSize.Height - 100));

		if (ImGui::BeginPopupModal ("Script", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::BeginChild (
				"Child1", ImVec2 (300, ImGui::GetWindowSize ().y - 40), false, ImGuiWindowFlags_HorizontalScrollbar);
			ImGui::BeginGroup ();
			{
				if (ImGui::Button ("OPEN", ImVec2 (300, 18.f)))
				{
					m_OpenButtonPressed = true;
				}

				if (m_OpenButtonPressed)
				{
					char const* filterPatterns[1] = {"*.script"};
					std::string dataPath = GetDataPath ();
					dataPath += GetPathSeparator ();
					dataPath += "scripts";
					dataPath += GetPathSeparator ();
					dataPath += "special";

					const char* chosenPath = tinyfd_openFileDialog (
						"Open Script", dataPath.c_str (), 1, filterPatterns, "script files", 0);

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

						AddEntry (fileName);

						m_OpenButtonPressed = false;
					}
					else
					{
						m_OpenButtonPressed = false;
					}
				}

				const char* items[m_Entries.size ()];

				for (int i = 0; i < m_Entries.size (); ++i)
				{
					m_Entries[i].Name = m_Entries[i].Path;

					if (m_Entries[i].Modified)
					{
						m_Entries[i].Name = "[*] " + m_Entries[i].Name;
					}

					items[i] = const_cast<char*> (m_Entries[i].Name.c_str ());
				}

				ImGui::PushItemWidth (300);

				static int hoveredScript = -1;

				if (ImGui::HoverableListBox ("", &m_SelectedEntry, items, IM_ARRAYSIZE (items),
						(ImGui::GetWindowSize ().y - 25) / 18, &hoveredScript))
				{
				}

				if (ImGui::IsItemClicked (1))
				{
					std::string selectedScript = items[hoveredScript];

					for (int pos = 0; pos < m_Entries.size (); ++pos)
					{
						if (m_Entries[pos].Path == selectedScript)
						{
							m_Entries.erase (m_Entries.begin () + pos);
							hoveredScript = -1;
							m_SelectedEntry = -1;
							break;
						}
					}
				}

				ImGui::PopItemWidth ();
			}
			ImGui::EndGroup ();
			ImGui::EndChild ();

			ImGui::SameLine ();

			ImGui::BeginChild ("Child2", ImVec2 (ImGui::GetWindowSize ().x - 430, ImGui::GetWindowSize ().y - 40),
				false, ImGuiWindowFlags_HorizontalScrollbar);

			if (m_SelectedEntry >= 0)
			{
				m_Entries[m_SelectedEntry].TextEditorControl.Render (
					"ScriptEditor", ImVec2 (ImGui::GetWindowSize ().x, ImGui::GetWindowSize ().y - 10), true);

				if (m_Entries[m_SelectedEntry].TextEditorControl.IsTextChanged ())
				{
					m_Entries[m_SelectedEntry].Text = m_Entries[m_SelectedEntry].TextEditorControl.GetText ();
					m_Entries[m_SelectedEntry].Modified = true;
				}
			}

			ImGui::EndChild ();

			ImGui::SameLine ();

			ImGui::BeginGroup ();
			{
				float buttonSize = 100.f;

				if (ImGui::Button ("SAVE", ImVec2 (buttonSize, 18.f)))
				{
					ImGui::CloseCurrentPopup ();

					m_IsVisible = false;
				}

				if (ImGui::Button ("CANCEL", ImVec2 (buttonSize, 18.f)) || m_Editor->IsCloseCurrentPopup ())
				{
					ImGui::CloseCurrentPopup ();
					m_IsVisible = false;

					m_Editor->SetCloseCurrentPopup (false);
				}
			}
			ImGui::EndGroup ();

			ImGui::EndPopup ();
		}
	}

	//--------------------------------------------------------------------------------------------------

	bool EditorScriptWindow::IsVisible () const { return m_IsVisible; }

	//--------------------------------------------------------------------------------------------------

	void EditorScriptWindow::AddEntry (const std::string& dataPath)
	{
		std::string realPath = GetDataPath () + "/scripts/" + dataPath;
		std::ifstream file (realPath.c_str ());
		std::stringstream strStream;

		strStream << file.rdbuf (); // read the file
		file.close ();

		FileEntry entry;
		entry.Path = dataPath;
		entry.Text = strStream.str ();
		entry.Modified = false;
		entry.TextEditorControl.SetText (entry.Text);
		entry.TextEditorControl.SetPalette (TextEditor::GetDarkPalette ());
		entry.TextEditorControl.SetLanguageDefinition (TextEditor::LanguageDefinition::AngelScript ());

		m_Entries.push_back (entry);
	}

	//--------------------------------------------------------------------------------------------------

	std::vector<FileEntry>& EditorScriptWindow::GetEntries () { return m_Entries; }

	//--------------------------------------------------------------------------------------------------

	void EditorScriptWindow::OpenSelectedFile ()
	{
		m_Entries[m_SelectedEntry].TextEditorControl.SetText (m_Entries[m_SelectedEntry].Text);
	}

	//--------------------------------------------------------------------------------------------------

	void EditorScriptWindow::SaveCurrentFile ()
	{
		std::string realPath = GetDataPath () + "/scripts/" + m_Entries[m_SelectedEntry].Path;

		std::ofstream outFile (realPath.c_str ());

		if (outFile.is_open ())
		{
			m_Entries[m_SelectedEntry].Text = m_Entries[m_SelectedEntry].TextEditorControl.GetText ();

			outFile << m_Entries[m_SelectedEntry].Text;
			outFile.close ();

			m_Entries[m_SelectedEntry].Modified = false;
		}
	}

	//--------------------------------------------------------------------------------------------------
}
