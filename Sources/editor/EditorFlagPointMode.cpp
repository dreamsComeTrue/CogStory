// Copyright 2017-2019 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "EditorFlagPointMode.h"
#include "Editor.h"
#include "MainLoop.h"
#include "Screen.h"

#include "imgui.h"

namespace aga
{
	//--------------------------------------------------------------------------------------------------

	EditorFlagPointMode::EditorFlagPointMode (Editor* editor)
		: m_Editor (editor)
		, m_IsVisible (false)
		, m_AskFlagPoint (false)
		, m_FlagPoint ("")
		, m_Editing (false)
		, m_DrawConnection (true)
		, m_Dragging (false)
	{
		memset (m_FlagPointWindow, 0, ARRAY_SIZE (m_FlagPointWindow));
	}

	//--------------------------------------------------------------------------------------------------

	EditorFlagPointMode::~EditorFlagPointMode () {}

	//--------------------------------------------------------------------------------------------------

	void EditorFlagPointMode::ShowUI () { m_IsVisible = true; }

	//--------------------------------------------------------------------------------------------------

	bool EditorFlagPointMode::MoveSelectedFlagPoint ()
	{
		ALLEGRO_MOUSE_STATE state;
		al_get_mouse_state (&state);

		if (state.buttons == 1 && m_Dragging && m_FlagPoint != "")
		{
			std::map<std::string, FlagPoint>& flagPoints
				= m_Editor->GetMainLoop ()->GetSceneManager ().GetActiveScene ()->GetFlagPoints ();
			Point p = m_Editor->CalculateWorldPoint (state.x, state.y);

			flagPoints[m_FlagPoint].Pos.X = p.X;
			flagPoints[m_FlagPoint].Pos.Y = p.Y;

			return true;
		}

		return false;
	}

	//--------------------------------------------------------------------------------------------------

	void EditorFlagPointMode::Render ()
	{
		ALLEGRO_MOUSE_STATE state;
		al_get_mouse_state (&state);

		Scene* scene = m_Editor->GetMainLoop ()->GetSceneManager ().GetActiveScene ();

		if (scene)
		{
			std::map<std::string, FlagPoint>& flagPoints = scene->GetFlagPoints ();
			int outsets = 4;

			bool addedFound = false;
			for (auto& kv : flagPoints)
			{
				if (kv.first == m_FlagPoint)
				{
					addedFound = true;
				}
			}

			Camera& camera = m_Editor->GetMainLoop ()->GetSceneManager ().GetCamera ();

			if (m_DrawConnection && addedFound)
			{
				Point translate = camera.GetTranslate ();
				Point scale = camera.GetScale ();

				float xPoint = flagPoints[m_FlagPoint].Pos.X * scale.X - translate.X;
				float yPoint = flagPoints[m_FlagPoint].Pos.Y * scale.Y - translate.Y;

				al_draw_line (xPoint, yPoint, state.x, state.y, COLOR_ORANGE, 2);
			}

			for (std::map<std::string, FlagPoint>::iterator it = flagPoints.begin (); it != flagPoints.end (); ++it)
			{
				Point translate = camera.GetTranslate ();
				Point scale = camera.GetScale ();
				FlagPoint& fp = it->second;

				float xPoint = fp.Pos.X * scale.X - translate.X;
				float yPoint = fp.Pos.Y * scale.Y - translate.Y;

				m_Editor->GetMainLoop ()->GetScreen ()->GetFont ().DrawText (FONT_NAME_SMALL, it->first,
					al_map_rgb (0, 255, 0), xPoint, yPoint - 15, 1.0f, ALLEGRO_ALIGN_CENTER);

				Point p = {fp.Pos.X, fp.Pos.Y};

				for (std::vector<FlagPoint*>::iterator it2 = it->second.Connections.begin ();
					 it2 != it->second.Connections.end (); ++it2)
				{
					float x2Point = (*it2)->Pos.X * scale.X - translate.X;
					float y2Point = (*it2)->Pos.Y * scale.Y - translate.Y;

					al_draw_line (xPoint, yPoint, x2Point, y2Point, COLOR_ORANGE, 2);
				}

				if (m_Editor->IsMouseWithinPointRect (state.x, state.y, p, outsets))
				{
					al_draw_filled_circle (xPoint, yPoint, 4, COLOR_BLUE);
				}
				else
				{
					al_draw_filled_circle (xPoint, yPoint, 4, COLOR_GREEN);
					al_draw_filled_circle (xPoint, yPoint, 2, COLOR_RED);
				}
			}
		}
	}

	//--------------------------------------------------------------------------------------------------

	bool EditorFlagPointMode::InsertFlagPointAtCursor (int mouseX, int mouseY)
	{
		if (std::string (m_FlagPoint) != "")
		{
			if (GetFlagPointUnderCursor (mouseX, mouseY) == "")
			{
				Point p = m_Editor->CalculateWorldPoint (mouseX, mouseY);

				m_Editor->GetMainLoop ()->GetSceneManager ().GetActiveScene ()->AddFlagPoint (m_FlagPoint, p);
			}

			m_FlagPoint = "";

			return true;
		}
		else
		{
			m_AskFlagPoint = true;
		}

		return false;
	}

	//--------------------------------------------------------------------------------------------------

	bool EditorFlagPointMode::RemoveFlagPointUnderCursor (int mouseX, int mouseY)
	{
		std::string flagPoint = GetFlagPointUnderCursor (mouseX, mouseY);
		Scene* scene = m_Editor->GetMainLoop ()->GetSceneManager ().GetActiveScene ();

		if (flagPoint != "")
		{
			FlagPoint* flag = scene->GetFlagPoint (flagPoint);
			std::vector<FlagPoint*>& myConnections = flag->Connections;

			//	Search in own connections
			for (size_t i = 0; i < myConnections.size (); ++i)
			{
				std::vector<FlagPoint*>& otherConnections = myConnections[i]->Connections;

				for (size_t j = 0; j < otherConnections.size (); ++j)
				{
					if (otherConnections[j] == flag)
					{
						otherConnections.erase (otherConnections.begin () + j);
					}
				}
			}

			std::map<std::string, FlagPoint>& allFlagPoints = scene->GetFlagPoints ();

			//	Search in all connections
			for (std::map<std::string, FlagPoint>::iterator it = allFlagPoints.begin (); it != allFlagPoints.end ();
				 ++it)
			{
				std::vector<FlagPoint*>& otherConnections = (*it).second.Connections;

				for (size_t i = 0; i < otherConnections.size (); ++i)
				{
					if (otherConnections[i] == flag)
					{
						otherConnections.erase (otherConnections.begin () + i);
					}
				}
			}

			scene->GetFlagPoints ().erase (flagPoint);

			return true;
		}

		return false;
	}

	//--------------------------------------------------------------------------------------------------

	std::string EditorFlagPointMode::GetFlagPointUnderCursor (int mouseX, int mouseY)
	{
		std::map<std::string, FlagPoint>& flagPoints
			= m_Editor->GetMainLoop ()->GetSceneManager ().GetActiveScene ()->GetFlagPoints ();
		int outsets = 4;

		for (std::map<std::string, FlagPoint>::iterator it = flagPoints.begin (); it != flagPoints.end (); ++it)
		{
			Point p = {it->second.Pos.X, it->second.Pos.Y};

			if (m_Editor->IsMouseWithinPointRect (mouseX, mouseY, p, outsets))
			{
				if (m_FlagPoint != it->first && m_FlagPoint != "")
				{
					//  Add or remove connection depending if we already have one
					bool found = false;
					std::vector<FlagPoint*>& others = flagPoints[m_FlagPoint].Connections;

					for (size_t i = 0; i < others.size (); ++i)
					{
						if (others[i]->Name == it->first)
						{
							others.erase (others.begin () + i);

							std::vector<FlagPoint*>& mine = flagPoints[it->first].Connections;

							for (size_t j = 0; j < mine.size (); ++j)
							{
								if (mine[j]->Name == flagPoints[m_FlagPoint].Name)
								{
									mine.erase (mine.begin () + j);
									break;
								}
							}

							found = true;
							break;
						}
					}

					if (!found)
					{
						flagPoints[m_FlagPoint].Connections.push_back (&it->second);
						flagPoints[it->first].Connections.push_back (&flagPoints[m_FlagPoint]);
					}
				}

				m_FlagPoint = it->first;
				strcpy (m_FlagPointWindow, m_FlagPoint.c_str ());
				m_Editing = true;

				return m_FlagPoint;
			}
		}

		m_Editing = false;

		return "";
	}

	//--------------------------------------------------------------------------------------------------

	void EditorFlagPointMode::RenderUI ()
	{
		ImGui::SetNextWindowSize (ImVec2 (600, 85));

		if (ImGui::BeginPopupModal ("Flag Point", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			m_DrawConnection = false;

			ImGui::PushItemWidth (580);
			ImGui::InputText ("", m_FlagPointWindow, ARRAY_SIZE (m_FlagPointWindow));
			ImGui::PopItemWidth ();
			ImGui::SetItemDefaultFocus ();

			ImGui::Separator ();
			ImGui::BeginGroup ();

			m_AskFlagPoint = false;

			if (ImGui::Button ("ACCEPT", ImVec2 (80.f, 18.f)))
			{
				if (m_Editing)
				{
					Scene* scene = m_Editor->GetMainLoop ()->GetSceneManager ().GetActiveScene ();
					FlagPoint* oldFlagPoint = scene->GetFlagPoint (m_FlagPoint);

					if (oldFlagPoint)
					{
						std::string oldName = oldFlagPoint->Name;
						Point oldPos = oldFlagPoint->Pos;
						std::vector<FlagPoint*>& connections = oldFlagPoint->Connections;

						scene->RemoveFlagPoint (m_FlagPoint);

						FlagPoint* newFlagPoint = scene->AddFlagPoint (m_FlagPointWindow, oldPos);
						newFlagPoint->Connections = connections;

						std::map<std::string, FlagPoint>& allFlagPoints = scene->GetFlagPoints ();

						//	Search in all connections
						for (std::map<std::string, FlagPoint>::iterator it = allFlagPoints.begin ();
							 it != allFlagPoints.end (); ++it)
						{
							std::vector<FlagPoint*>& otherConnections = (*it).second.Connections;

							for (size_t i = 0; i < otherConnections.size (); ++i)
							{
								if (otherConnections[i]->Name == oldName)
								{
									otherConnections.erase (otherConnections.begin () + i);
									otherConnections.push_back (newFlagPoint);
								}
							}
						}
					}

					m_FlagPoint = "";
					m_Editing = false;
					m_Editor->SetCursorMode (CursorMode::ActorSelectMode);
				}
				else
				{
					m_FlagPoint = m_FlagPointWindow;
					m_Editor->SetCursorMode (CursorMode::EditFlagPointsMode);
				}

				m_IsVisible = false;
				m_DrawConnection = true;
				ImGui::CloseCurrentPopup ();
			}

			ImGui::SameLine ();

			if (ImGui::Button ("CANCEL", ImVec2 (80.f, 18.f)) || m_Editor->IsCloseCurrentPopup ())
			{
				m_FlagPoint = "";
				m_Editor->SetCursorMode (CursorMode::ActorSelectMode);

				m_IsVisible = false;
				m_DrawConnection = true;
				ImGui::CloseCurrentPopup ();
				m_Editor->SetCloseCurrentPopup (false);
			}
			ImGui::EndGroup ();

			ImGui::EndPopup ();
		}
	}

	//--------------------------------------------------------------------------------------------------

	std::string EditorFlagPointMode::GetFlagPoint () { return m_FlagPoint; }

	//--------------------------------------------------------------------------------------------------

	void EditorFlagPointMode::SetFlagPoint (const std::string& pointName) { m_FlagPoint = pointName; }

	//--------------------------------------------------------------------------------------------------

	void EditorFlagPointMode::SetAskFlagPoint (bool ask) { m_AskFlagPoint = ask; }

	//--------------------------------------------------------------------------------------------------

	bool EditorFlagPointMode::IsVisible () { return m_IsVisible; }

	//--------------------------------------------------------------------------------------------------

	void EditorFlagPointMode::SetDragging (bool drag) { m_Dragging = drag; }

	//--------------------------------------------------------------------------------------------------
}
