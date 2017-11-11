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
                std::map<std::string, Point>& flagPoints = m_Editor->m_MainLoop->GetSceneManager ().GetActiveScene ()->GetFlagPoints ();

                Point p = m_Editor->CalculateCursorPoint (state.x, state.y);
                Point translate = m_Editor->m_MainLoop->GetSceneManager ().GetCamera ().GetTranslate ();
                Point scale = m_Editor->m_MainLoop->GetSceneManager ().GetCamera ().GetScale ();

                flagPoints[m_FlagPoint].X = (translate.X + p.X) * 1 / scale.X;
                flagPoints[m_FlagPoint].Y = (translate.Y + p.Y) * 1 / scale.Y;

                return true;
            }
        }

        return false;
    }

    //--------------------------------------------------------------------------------------------------

    void EditorFlagPointMode::DrawFlagPoints (float mouseX, float mouseY)
    {
        std::map<std::string, Point>& flagPoints = m_Editor->m_MainLoop->GetSceneManager ().GetActiveScene ()->GetFlagPoints ();
        int outsets = 4;

        for (std::map<std::string, Point>::iterator it = flagPoints.begin (); it != flagPoints.end (); ++it)
        {
            Point translate = m_Editor->m_MainLoop->GetSceneManager ().GetCamera ().GetTranslate ();
            Point scale = m_Editor->m_MainLoop->GetSceneManager ().GetCamera ().GetScale ();

            float xPoint = it->second.X * scale.X - translate.X;
            float yPoint = it->second.Y * scale.Y - translate.Y;

            m_Editor->m_MainLoop->GetScreen ()->GetFont ().DrawText (
              FONT_NAME_MAIN_SMALL, al_map_rgb (0, 255, 0), xPoint, yPoint - 15, it->first, ALLEGRO_ALIGN_CENTER);

            Point p = { it->second.X, it->second.Y };

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

            memset (m_FlagPointName, 0, sizeof (m_FlagPointName));
        }
        else
        {
            m_AskFlagPoint = true;
        }
    }

    //--------------------------------------------------------------------------------------------------

    bool EditorFlagPointMode::RemoveFlagPointUnderCursor (int mouseX, int mouseY)
    {
        std::string flgPoint = GetFlagPointUnderCursor (mouseX, mouseY);

        if (flgPoint != "")
        {
            m_Editor->m_MainLoop->GetSceneManager ().GetActiveScene ()->GetFlagPoints ().erase (flgPoint);
            return true;
        }

        return false;
    }

    //--------------------------------------------------------------------------------------------------

    std::string EditorFlagPointMode::GetFlagPointUnderCursor (int mouseX, int mouseY)
    {
        std::map<std::string, Point>& flagPoints = m_Editor->m_MainLoop->GetSceneManager ().GetActiveScene ()->GetFlagPoints ();
        int outsets = 4;

        for (std::map<std::string, Point>::iterator it = flagPoints.begin (); it != flagPoints.end (); ++it)
        {
            Point p = { it->second.X, it->second.Y };

            if (m_Editor->IsMouseWithinPointRect (mouseX, mouseY, p, outsets))
            {
                return it->first;
            }
        }

        return "";
    }

    //--------------------------------------------------------------------------------------------------
}
