// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "EditorFlagPointMode.h"
#include "Editor.h"
#include "MainLoop.h"
#include "Screen.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    EditorFlagPointMode::EditorFlagPointMode (Editor* editor)
      : m_Editor (editor)
      , m_AskFlagPoint (false)
      , m_FlagPoint ("")
      , m_DrawConnection (true)
    {
    }

    //--------------------------------------------------------------------------------------------------

    EditorFlagPointMode::~EditorFlagPointMode () {}

    //--------------------------------------------------------------------------------------------------

    bool EditorFlagPointMode::MoveSelectedFlagPoint ()
    {
        ALLEGRO_MOUSE_STATE state;
        al_get_mouse_state (&state);

        if (state.buttons == 1)
        {
            if (m_FlagPoint != "")
            {
                std::map<std::string, FlagPoint>& flagPoints = m_Editor->m_MainLoop->GetSceneManager ().GetActiveScene ()->GetFlagPoints ();

                Point p = m_Editor->CalculateCursorPoint (state.x, state.y);
                Point translate = m_Editor->m_MainLoop->GetSceneManager ().GetCamera ().GetTranslate ();
                Point scale = m_Editor->m_MainLoop->GetSceneManager ().GetCamera ().GetScale ();

                flagPoints[m_FlagPoint].Pos.X = (translate.X + p.X) * 1 / scale.X;
                flagPoints[m_FlagPoint].Pos.Y = (translate.Y + p.Y) * 1 / scale.Y;

                return true;
            }
        }

        return false;
    }

    //--------------------------------------------------------------------------------------------------

    void EditorFlagPointMode::DrawFlagPoints (float mouseX, float mouseY)
    {
        std::map<std::string, FlagPoint>& flagPoints = m_Editor->m_MainLoop->GetSceneManager ().GetActiveScene ()->GetFlagPoints ();
        int outsets = 4;

        if (m_DrawConnection && m_FlagPoint != "")
        {
            Point translate = m_Editor->m_MainLoop->GetSceneManager ().GetCamera ().GetTranslate ();
            Point scale = m_Editor->m_MainLoop->GetSceneManager ().GetCamera ().GetScale ();

            float xPoint = flagPoints[m_FlagPoint].Pos.X * scale.X - translate.X;
            float yPoint = flagPoints[m_FlagPoint].Pos.Y * scale.Y - translate.Y;

            al_draw_line (xPoint, yPoint, mouseX, mouseY, COLOR_ORANGE, 2);
        }

        for (std::map<std::string, FlagPoint>::iterator it = flagPoints.begin (); it != flagPoints.end (); ++it)
        {
            Point translate = m_Editor->m_MainLoop->GetSceneManager ().GetCamera ().GetTranslate ();
            Point scale = m_Editor->m_MainLoop->GetSceneManager ().GetCamera ().GetScale ();
            FlagPoint& fp = it->second;

            float xPoint = fp.Pos.X * scale.X - translate.X;
            float yPoint = fp.Pos.Y * scale.Y - translate.Y;

            m_Editor->m_MainLoop->GetScreen ()->GetFont ().DrawText (
              FONT_NAME_MAIN_SMALL, al_map_rgb (0, 255, 0), xPoint, yPoint - 15, it->first, ALLEGRO_ALIGN_CENTER);

            Point p = { fp.Pos.X, fp.Pos.Y };

            for (std::vector<FlagPoint*>::iterator it2 = it->second.Connections.begin (); it2 != it->second.Connections.end (); ++it2)
            {
                float x2Point = (*it2)->Pos.X * scale.X - translate.X;
                float y2Point = (*it2)->Pos.Y * scale.Y - translate.Y;

                al_draw_line (xPoint, yPoint, x2Point, y2Point, COLOR_ORANGE, 2);
            }

            if (m_Editor->IsMouseWithinPointRect (mouseX, mouseY, p, outsets))
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

    //--------------------------------------------------------------------------------------------------

    void EditorFlagPointMode::InsertFlagPointAtCursor (int mouseX, int mouseY)
    {
        if (std::string (m_FlagPointName) != "")
        {
            if (GetFlagPointUnderCursor (mouseX, mouseY) == "")
            {
                Point p = m_Editor->CalculateCursorPoint (mouseX, mouseY);
                Point translate = m_Editor->m_MainLoop->GetSceneManager ().GetCamera ().GetTranslate ();
                Point scale = m_Editor->m_MainLoop->GetSceneManager ().GetCamera ().GetScale ();

                Point pointToInsert = { (translate.X + p.X) * 1 / scale.X, (translate.Y + p.Y) * 1 / scale.Y };

                m_Editor->m_MainLoop->GetSceneManager ().GetActiveScene ()->AddFlagPoint (m_FlagPointName, pointToInsert);
                m_FlagPoint = m_FlagPointName;
            }

            m_FlagPointName = "";
        }
        else
        {
            m_AskFlagPoint = true;
        }
    }

    //--------------------------------------------------------------------------------------------------

    bool EditorFlagPointMode::RemoveFlagPointUnderCursor (int mouseX, int mouseY)
    {
        std::string flagPoint = GetFlagPointUnderCursor (mouseX, mouseY);

        if (flagPoint != "")
        {
            m_Editor->m_MainLoop->GetSceneManager ().GetActiveScene ()->GetFlagPoints ().erase (flagPoint);
            return true;
        }

        return false;
    }

    //--------------------------------------------------------------------------------------------------

    std::string EditorFlagPointMode::GetFlagPointUnderCursor (int mouseX, int mouseY)
    {
        std::map<std::string, FlagPoint>& flagPoints = m_Editor->m_MainLoop->GetSceneManager ().GetActiveScene ()->GetFlagPoints ();
        int outsets = 4;

        for (std::map<std::string, FlagPoint>::iterator it = flagPoints.begin (); it != flagPoints.end (); ++it)
        {
            Point p = { it->second.Pos.X, it->second.Pos.Y };

            if (m_Editor->IsMouseWithinPointRect (mouseX, mouseY, p, outsets))
            {
                if (m_FlagPoint != it->first && m_FlagPoint != "")
                {
                    //  Add or remove connection depending if we already have one
                    bool found = false;
                    std::vector<FlagPoint*>& others = flagPoints[m_FlagPoint].Connections;

                    for (int i = 0; i < others.size (); ++i)
                    {
                        if (others[i]->Name == it->first)
                        {
                            others.erase (others.begin () + i);

                            std::vector<FlagPoint*>& mine = flagPoints[it->first].Connections;

                            for (int j = 0; j < mine.size (); ++j)
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

                return it->first;
            }
        }

        return "";
    }

    //--------------------------------------------------------------------------------------------------
}
