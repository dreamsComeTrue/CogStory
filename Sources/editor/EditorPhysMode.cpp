// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "EditorPhysMode.h"
#include "Editor.h"
#include "MainLoop.h"
#include "Screen.h"
#include "actors/TileActor.h"

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
        Point origin;

        if (m_Editor->GetEditorActorMode ().GetSelectedActor ())
        {
            origin = m_Editor->GetEditorActorMode ().GetSelectedActor ()->Bounds.GetPos ();
        }

        ALLEGRO_MOUSE_STATE state;
        al_get_mouse_state (&state);

        if (m_PhysPoint && state.buttons == 1)
        {
            Point p = m_Editor->CalculateCursorPoint (state.x, state.y);
            Point translate = m_Editor->GetMainLoop ()->GetSceneManager ().GetCamera ().GetTranslate ();
            Point scale = m_Editor->GetMainLoop ()->GetSceneManager ().GetCamera ().GetScale ();

            m_PhysPoint->X = (translate.X + p.X) * 1 / scale.X - origin.X;
            m_PhysPoint->Y = (translate.Y + p.Y) * 1 / scale.Y - origin.Y;

            return true;
        }

        return false;
    }

    //--------------------------------------------------------------------------------------------------

    void EditorPhysMode::DrawPhysBody (float mouseX, float mouseY)
    {
        Point translate = m_Editor->GetMainLoop ()->GetSceneManager ().GetCamera ().GetTranslate ();
        Point scale = m_Editor->GetMainLoop ()->GetSceneManager ().GetCamera ().GetScale ();
        Point* selectedPoint = GetPhysPointUnderCursor (mouseX, mouseY);
        Point origin;
        std::vector<std::vector<Point>>* physPoints = nullptr;

        if (m_Editor->GetEditorActorMode ().GetSelectedActor ())
        {
            origin = m_Editor->GetEditorActorMode ().GetSelectedActor ()->Bounds.GetPos ();
            physPoints = &m_Editor->GetEditorActorMode ().GetSelectedActor ()->PhysPoints;
        }

        if (physPoints)
        {
            for (std::vector<Point>& points : *physPoints)
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

                        if (m_Editor->GetMainLoop ()->GetSceneManager ().IsDrawPhysData () && false)
                        {
                            m_Editor->GetMainLoop ()->GetScreen ()->GetFont ().DrawText (
                                FONT_NAME_SMALL, al_map_rgb (0, 255, 0), xPoint, yPoint, ToString (i),
                                ALLEGRO_ALIGN_CENTER);
                        }

                        ++i;

                        al_draw_filled_circle (xPoint, yPoint, 4, color);
                    }
                }
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    void EditorPhysMode::InsertPhysPointAtCursor (int mouseX, int mouseY)
    {
        Point p = m_Editor->CalculateCursorPoint (mouseX, mouseY);
        Point translate = m_Editor->GetMainLoop ()->GetSceneManager ().GetCamera ().GetTranslate ();
        Point scale = m_Editor->GetMainLoop ()->GetSceneManager ().GetCamera ().GetScale ();
        Point origin;
        std::vector<std::vector<Point>>* physPoints;

        if (m_Editor->GetEditorActorMode ().GetSelectedActor ())
        {
            origin = m_Editor->GetEditorActorMode ().GetSelectedActor ()->Bounds.GetPos ();
            physPoints = &m_Editor->GetEditorActorMode ().GetSelectedActor ()->PhysPoints;
        }

        if (physPoints)
        {
            Point pointToInsert
                = { (translate.X + p.X) * 1 / scale.X - origin.X, (translate.Y + p.Y) * 1 / scale.Y - origin.Y };

            bool inserted = false;

            //  After we select one of physics point, we can insert next one accordingly
            Point* againSelected = GetPhysPointUnderCursor (mouseX, mouseY);

            if (physPoints->empty ())
            {
                physPoints->push_back ({});
            }

            if (!m_PhysPoly)
            {
                m_PhysPoly = &(*physPoints)[0];
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
                if (m_Editor->GetEditorActorMode ().GetSelectedActor ())
                {
                    m_Editor->GetEditorActorMode ().GetSelectedActor ()->SetPhysOffset (origin);
                }
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    Point* EditorPhysMode::GetPhysPointUnderCursor (int mouseX, int mouseY)
    {
        Point origin;
        std::vector<std::vector<Point>>* physPoints;

        if (m_Editor->GetEditorActorMode ().GetSelectedActor ())
        {
            origin = m_Editor->GetEditorActorMode ().GetSelectedActor ()->Bounds.GetPos ();
            physPoints = &m_Editor->GetEditorActorMode ().GetSelectedActor ()->PhysPoints;
        }

        if (physPoints && !physPoints->empty ())
        {
            int outsets = 4;
            for (std::vector<Point>& points : *physPoints)
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
        }

        return nullptr;
    }

    //--------------------------------------------------------------------------------------------------

    bool EditorPhysMode::RemovePhysPointUnderCursor (int mouseX, int mouseY)
    {
        Point* point = GetPhysPointUnderCursor (mouseX, mouseY);
        std::vector<std::vector<Point>>* physPoints;

        if (m_Editor->GetEditorActorMode ().GetSelectedActor ())
        {
            physPoints = &m_Editor->GetEditorActorMode ().GetSelectedActor ()->PhysPoints;
        }

        if (physPoints && point)
        {
            for (int j = 0; j < physPoints->size (); ++j)
            {
                std::vector<Point>& points = (*physPoints)[j];

                for (int i = 0; i < points.size (); ++i)
                {
                    if (points[i] == *point)
                    {
                        points.erase (points.begin () + i);

                        //  If no points left, remove polygon itself
                        if (points.empty ())
                        {
                            physPoints->erase (physPoints->begin () + j);
                            m_PhysPoly = nullptr;
                        }

                        if (m_Editor->GetEditorActorMode ().GetSelectedActor ())
                        {
                            m_Editor->GetEditorActorMode ().GetSelectedActor ()->UpdatePhysPolygon ();
                        }

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
