// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "EditorPhysMode.h"
#include "Editor.h"
#include "MainLoop.h"
#include "Screen.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    EditorPhysMode::EditorPhysMode (Editor* editor)
        : m_Editor (editor)
        , m_PhysPoint (nullptr)
        , m_PhysPointIndex (-1)
        , m_PhysPoly (nullptr)
    {
    }

    //--------------------------------------------------------------------------------------------------

    EditorPhysMode::~EditorPhysMode () {}

    //--------------------------------------------------------------------------------------------------

    bool EditorPhysMode::MoveSelectedPhysPoint ()
    {
        ALLEGRO_MOUSE_STATE state;
        al_get_mouse_state (&state);

        if (m_PhysPoint && state.buttons == 1)
        {
            Point p = m_Editor->CalculateCursorPoint (state.x, state.y);
            Point translate = m_Editor->m_MainLoop->GetSceneManager ().GetCamera ().GetTranslate ();
            Point scale = m_Editor->m_MainLoop->GetSceneManager ().GetCamera ().GetScale ();
            Point origin = m_Editor->m_EditorTileMode.m_SelectedTile->Bounds.Transform.Pos;

            m_PhysPoint->X = (translate.X + p.X) * 1 / scale.X - origin.X;
            m_PhysPoint->Y = (translate.Y + p.Y) * 1 / scale.Y - origin.Y;

            return true;
        }

        return false;
    }

    //--------------------------------------------------------------------------------------------------

    void EditorPhysMode::DrawPhysBody (float mouseX, float mouseY)
    {
        if (!m_Editor->m_EditorTileMode.m_SelectedTile)
        {
            return;
        }

        Point translate = m_Editor->m_MainLoop->GetSceneManager ().GetCamera ().GetTranslate ();
        Point scale = m_Editor->m_MainLoop->GetSceneManager ().GetCamera ().GetScale ();
        Point origin = m_Editor->m_EditorTileMode.m_SelectedTile->Bounds.Transform.Pos;
        Point* selectedPoint = GetPhysPointUnderCursor (mouseX, mouseY);

        for (std::vector<Point>& points : m_Editor->m_EditorTileMode.m_SelectedTile->PhysPoints)
        {
            if (!points.empty ())
            {
                int i = 0;
                int selectedIndex = std::numeric_limits<int>::min ();

                for (int j = 0; j < points.size (); ++j)
                {
                    if (m_PhysPoint && points[j] == *m_PhysPoint)
                    {
                        selectedIndex = j;
                        break;
                    }
                }

                for (const Point& p : points)
                {
                    float xPoint = (origin.X + p.X) * scale.X - translate.X;
                    float yPoint = (origin.Y + p.Y) * scale.Y - translate.Y;

                    ALLEGRO_COLOR color;

                    if (i == 0)
                    {
                        color = COLOR_GREEN;
                    }
                    else
                    {
                        color = COLOR_YELLOW;
                    }

                    if (i == selectedIndex)
                    {
                        //  Mark selected corner
                        color = COLOR_BLUE;
                    }
                    else if ((i == 0 && selectedIndex == points.size () - 1) || (i == selectedIndex + 1))
                    {
                        //  Mark also next corner
                        color = COLOR_LIGHTBLUE;
                    }

                    if (selectedPoint != nullptr && p == *selectedPoint)
                    {
                        color = COLOR_RED;
                    }

                    if (m_Editor->m_MainLoop->GetSceneManager ().GetActiveScene ()->IsDrawPhysData () && false)
                    {
                        m_Editor->m_MainLoop->GetScreen ()->GetFont ().DrawText (FONT_NAME_MAIN_SMALL,
                            al_map_rgb (0, 255, 0), xPoint, yPoint, ToString (i), ALLEGRO_ALIGN_CENTER);
                    }

                    ++i;

                    al_draw_filled_circle (xPoint, yPoint, 4, color);
                }
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    void EditorPhysMode::InsertPhysPointAtCursor (int mouseX, int mouseY)
    {
        if (!m_Editor->m_EditorTileMode.m_SelectedTile)
        {
            return;
        }

        Point p = m_Editor->CalculateCursorPoint (mouseX, mouseY);
        Point translate = m_Editor->m_MainLoop->GetSceneManager ().GetCamera ().GetTranslate ();
        Point scale = m_Editor->m_MainLoop->GetSceneManager ().GetCamera ().GetScale ();
        Point origin = m_Editor->m_EditorTileMode.m_SelectedTile->Bounds.Transform.Pos;

        Point pointToInsert
            = { (translate.X + p.X) * 1 / scale.X - origin.X, (translate.Y + p.Y) * 1 / scale.Y - origin.Y };

        bool inserted = false;

        //  After we select one of physics point, we can insert next one accordingly
        Point* againSelected = GetPhysPointUnderCursor (mouseX, mouseY);

        if (m_Editor->m_EditorTileMode.m_SelectedTile->PhysPoints.empty ())
        {
            m_Editor->m_EditorTileMode.m_SelectedTile->PhysPoints.push_back ({});
        }

        if (!m_PhysPoly)
        {
            m_PhysPoly = &m_Editor->m_EditorTileMode.m_SelectedTile->PhysPoints[0];
        }

        if (m_PhysPoint && !againSelected)
        {
            for (int i = 0; i < m_PhysPoly->size (); ++i)
            {
                if (m_PhysPoint && (*m_PhysPoly)[i] == *m_PhysPoint)
                {
                    m_PhysPoly->insert (m_PhysPoly->begin () + i + 1, pointToInsert);
                    m_PhysPoint = nullptr;
                    inserted = true;
                    break;
                }
            }
        }

        m_PhysPointIndex = -1;
        m_PhysPoint = GetPhysPointUnderCursor (mouseX, mouseY);

        if (!inserted && !m_PhysPoint)
        {
            m_PhysPoly->push_back (pointToInsert);
            m_PhysPoint = &(*m_PhysPoly)[m_PhysPoly->size () - 1];
            inserted = true;
        }

        if (inserted)
        {
            m_Editor->m_EditorTileMode.m_SelectedTile->SetPhysOffset (origin);

            if (m_PhysPointIndex > -1)
            {
                //    m_PhysPoint = &(*m_PhysPoly)[m_PhysPointIndex];
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    Point* EditorPhysMode::GetPhysPointUnderCursor (int mouseX, int mouseY)
    {
        if (!m_Editor->m_EditorTileMode.m_SelectedTile)
        {
            return nullptr;
        }

        Point origin = m_Editor->m_EditorTileMode.m_SelectedTile->Bounds.Transform.Pos;

        int outsets = 4;
        for (std::vector<Point>& points : m_Editor->m_EditorTileMode.m_SelectedTile->PhysPoints)
        {
            int index = 0;
            for (Point& point : points)
            {
                Point p = { point.X + origin.X, point.Y + origin.Y };

                if (m_Editor->IsMouseWithinPointRect (mouseX, mouseY, p, outsets))
                {
                    m_PhysPoly = &points;
                    m_PhysPointIndex = index;
                    return &point;
                }

                index++;
            }
        }

        return nullptr;
    }

    //--------------------------------------------------------------------------------------------------

    bool EditorPhysMode::RemovePhysPointUnderCursor (int mouseX, int mouseY)
    {
        Point* point = GetPhysPointUnderCursor (mouseX, mouseY);

        if (m_Editor->m_EditorTileMode.m_SelectedTile && point)
        {
            for (int j = 0; j < m_Editor->m_EditorTileMode.m_SelectedTile->PhysPoints.size (); ++j)
            {
                std::vector<Point>& points = m_Editor->m_EditorTileMode.m_SelectedTile->PhysPoints[j];

                for (int i = 0; i < points.size (); ++i)
                {
                    if (points[i] == *point)
                    {
                        points.erase (points.begin () + i);

                        //  If no points left, remove polygon itself
                        if (points.empty ())
                        {
                            m_Editor->m_EditorTileMode.m_SelectedTile->PhysPoints.erase (
                                m_Editor->m_EditorTileMode.m_SelectedTile->PhysPoints.begin () + j);
                            m_PhysPoly = nullptr;
                        }

                        m_Editor->m_EditorTileMode.m_SelectedTile->UpdatePhysPolygon ();

                        if (m_PhysPointIndex == i)
                        {
                            m_PhysPointIndex = -1;
                        }

                        return true;
                    }
                }
            }
        }

        return false;
    }

    //--------------------------------------------------------------------------------------------------

    void EditorPhysMode::ResetSettings ()
    {
        m_PhysPoint = nullptr;
        m_PhysPointIndex = -1;
        m_PhysPoly = nullptr;
    }

    //--------------------------------------------------------------------------------------------------
}
