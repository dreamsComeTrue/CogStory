// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "EditorTriggerAreaMode.h"
#include "Editor.h"
#include "MainLoop.h"
#include "Screen.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    EditorTriggerAreaMode::EditorTriggerAreaMode (Editor* editor)
        : m_Editor (editor)
        , m_TriggerArea (nullptr)
        , m_TriggerPoint (nullptr)
        , m_TriggerAreaName ("")
    {
    }

    //--------------------------------------------------------------------------------------------------

    EditorTriggerAreaMode::~EditorTriggerAreaMode () {}

    //--------------------------------------------------------------------------------------------------

    bool EditorTriggerAreaMode::MoveSelectedTriggerPoint ()
    {
        ALLEGRO_MOUSE_STATE state;
        al_get_mouse_state (&state);

        if (m_TriggerPoint && state.buttons == 1)
        {
            Point p = m_Editor->CalculateCursorPoint (state.x, state.y);
            Point translate = m_Editor->m_MainLoop->GetSceneManager ().GetCamera ().GetTranslate ();
            Point scale = m_Editor->m_MainLoop->GetSceneManager ().GetCamera ().GetScale ();

            m_TriggerPoint->X = (translate.X + p.X) * 1 / scale.X;
            m_TriggerPoint->Y = (translate.Y + p.Y) * 1 / scale.Y;

            return true;
        }

        return false;
    }

    //--------------------------------------------------------------------------------------------------

    void EditorTriggerAreaMode::DrawTriggerAreas (float mouseX, float mouseY)
    {
        Point translate = m_Editor->m_MainLoop->GetSceneManager ().GetCamera ().GetTranslate ();
        Point scale = m_Editor->m_MainLoop->GetSceneManager ().GetCamera ().GetScale ();
        Point* selectedPoint = GetTriggerPointUnderCursor (mouseX, mouseY);
        std::map<std::string, TriggerArea>& triggerAreas
            = m_Editor->m_MainLoop->GetSceneManager ().GetActiveScene ()->GetTriggerAreas ();

        for (std::map<std::string, TriggerArea>::iterator it = triggerAreas.begin (); it != triggerAreas.end (); ++it)
        {
            if (!it->second.Points.empty ())
            {
                int i = 0;
                int selectedIndex = std::numeric_limits<int>::min ();

                for (int j = 0; j < it->second.Points.size (); ++j)
                {
                    if (m_TriggerPoint && it->second.Points[j] == *m_TriggerPoint)
                    {
                        selectedIndex = j;
                        break;
                    }
                }

                std::vector<float> out;
                for (const Point& p : it->second.Points)
                {
                    float xPoint = p.X * scale.X - translate.X;
                    float yPoint = p.Y * scale.Y - translate.Y;

                    out.push_back (xPoint);
                    out.push_back (yPoint);
                }

                al_draw_polygon (out.data (), it->second.Points.size (), 0, COLOR_DARKBLUE, 2, 0);

                Point min{ std::numeric_limits<int>::max (), std::numeric_limits<int>::max () };
                Point max{ std::numeric_limits<int>::min (), std::numeric_limits<int>::min () };

                for (const Point& p : it->second.Points)
                {
                    float xPoint = p.X * scale.X - translate.X;
                    float yPoint = p.Y * scale.Y - translate.Y;

                    if (xPoint < min.X)
                    {
                        min.X = xPoint;
                    }

                    if (yPoint < min.Y)
                    {
                        min.Y = yPoint;
                    }

                    if (xPoint > max.X)
                    {
                        max.X = xPoint;
                    }

                    if (yPoint > max.Y)
                    {
                        max.Y = yPoint;
                    }

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
                    else if ((i == 0 && selectedIndex == it->second.Points.size () - 1) || (i == selectedIndex + 1))
                    {
                        //  Mark also next corner
                        color = COLOR_LIGHTBLUE;
                    }

                    if (selectedPoint != nullptr && p == *selectedPoint)
                    {
                        color = COLOR_RED;
                    }

                    ++i;

                    al_draw_filled_circle (xPoint, yPoint, 4, color);
                }

                m_Editor->m_MainLoop->GetScreen ()->GetFont ().DrawText (FONT_NAME_SMALL, al_map_rgb (0, 255, 0),
                                                                         min.X + (max.X - min.X) * 0.5, min.Y + (max.Y - min.Y) * 0.5, ToString (it->second.Name),
                                                                         ALLEGRO_ALIGN_CENTER);
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    Point* EditorTriggerAreaMode::GetTriggerPointUnderCursor (int mouseX, int mouseY)
    {
        int outsets = 4;
        std::map<std::string, TriggerArea>& triggerAreas
            = m_Editor->m_MainLoop->GetSceneManager ().GetActiveScene ()->GetTriggerAreas ();

        for (std::map<std::string, TriggerArea>::iterator it = triggerAreas.begin (); it != triggerAreas.end (); ++it)
        {
            for (Point& point : it->second.Points)
            {
                if (m_Editor->IsMouseWithinPointRect (mouseX, mouseY, point, outsets))
                {
                    return &point;
                }
            }
        }

        return nullptr;
    }

    //--------------------------------------------------------------------------------------------------

    TriggerArea* EditorTriggerAreaMode::GetTriggerAreaUnderCursor (int mouseX, int mouseY)
    {
        int outsets = 4;

        std::map<std::string, TriggerArea>& triggerAreas
            = m_Editor->m_MainLoop->GetSceneManager ().GetActiveScene ()->GetTriggerAreas ();

        for (std::map<std::string, TriggerArea>::iterator it = triggerAreas.begin (); it != triggerAreas.end (); ++it)
        {
            for (Point& point : it->second.Points)
            {
                if (m_Editor->IsMouseWithinPointRect (mouseX, mouseY, point, outsets))
                {
                    return &it->second;
                }
            }
        }

        return nullptr;
    }

    //--------------------------------------------------------------------------------------------------

    void EditorTriggerAreaMode::InsertTriggerAreaAtCursor (int mouseX, int mouseY)
    {
        if (m_Editor->m_MainLoop->GetSceneManager ().GetActiveScene ()->GetTriggerAreas ().empty ())
        {
            NewTriggerArea ();
        }

        if (!m_TriggerArea)
        {
            m_TriggerArea
                = &m_Editor->m_MainLoop->GetSceneManager ().GetActiveScene ()->GetTriggerArea (m_TriggerAreaName);
        }

        Point p = m_Editor->CalculateCursorPoint (mouseX, mouseY);
        Point translate = m_Editor->m_MainLoop->GetSceneManager ().GetCamera ().GetTranslate ();
        Point scale = m_Editor->m_MainLoop->GetSceneManager ().GetCamera ().GetScale ();

        Point pointToInsert = { (translate.X + p.X) * 1 / scale.X, (translate.Y + p.Y) * 1 / scale.Y };

        bool inserted = false;

        //  After we select one of trigger point, we can insert next one accordingly
        Point* againSelected = GetTriggerPointUnderCursor (mouseX, mouseY);

        if (m_TriggerPoint && !againSelected)
        {
            for (int i = 0; i < m_TriggerArea->Points.size (); ++i)
            {
                if (m_TriggerPoint && (m_TriggerArea->Points)[i] == *m_TriggerPoint)
                {
                    m_TriggerArea->Points.insert (m_TriggerArea->Points.begin () + i + 1, pointToInsert);
                    m_TriggerPoint = nullptr;
                    inserted = true;
                    break;
                }
            }
        }

        m_TriggerPoint = GetTriggerPointUnderCursor (mouseX, mouseY);

        if (!inserted && !m_TriggerPoint)
        {
            m_TriggerArea->Points.push_back (pointToInsert);
            m_TriggerPoint = &(m_TriggerArea->Points)[m_TriggerArea->Points.size () - 1];
        }
    }

    //--------------------------------------------------------------------------------------------------

    bool EditorTriggerAreaMode::RemoveTriggerPointUnderCursor (int mouseX, int mouseY)
    {
        TriggerArea* triggerArea = GetTriggerAreaUnderCursor (mouseX, mouseY);
        Point* triggerPoint = GetTriggerPointUnderCursor (mouseX, mouseY);

        if (triggerArea && triggerPoint)
        {
            for (int i = 0; i < triggerArea->Points.size (); ++i)
            {
                if (triggerArea->Points[i] == *triggerPoint)
                {
                    triggerArea->Points.erase (triggerArea->Points.begin () + i);

                    if (triggerArea->Points.empty ())
                    {
                        m_Editor->m_MainLoop->GetSceneManager ().GetActiveScene ()->RemoveTriggerArea (
                            triggerArea->Name);
                    }
                    else
                    {
                        if (i < triggerArea->Points.size () - 1)
                        {
                            m_TriggerPoint = &triggerArea->Points[i + 1];
                        }
                        else
                        {
                            m_TriggerPoint = &triggerArea->Points[0];
                        }

                        triggerArea->UpdatePolygons (&m_Editor->m_MainLoop->GetPhysicsManager ().GetTriangulator ());
                    }

                    return true;
                }
            }
        }

        m_TriggerPoint = nullptr;
        m_TriggerArea = nullptr;

        return false;
    }

    //--------------------------------------------------------------------------------------------------

    void EditorTriggerAreaMode::NewTriggerArea ()
    {
        m_Editor->m_MainLoop->GetSceneManager ().GetActiveScene ()->AddTriggerArea (m_TriggerAreaName, {});
        m_TriggerArea = &m_Editor->m_MainLoop->GetSceneManager ().GetActiveScene ()->GetTriggerArea (m_TriggerAreaName);
        m_TriggerPoint = nullptr;
    }

    //--------------------------------------------------------------------------------------------------
}
