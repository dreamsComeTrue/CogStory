// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "EditorTriggerAreaMode.h"
#include "Editor.h"
#include "MainLoop.h"
#include "Screen.h"

#include "imgui.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    EditorTriggerAreaMode::EditorTriggerAreaMode (Editor* editor)
        : m_Editor (editor)
        , m_TriggerArea (nullptr)
        , m_TriggerPoint (nullptr)
        , m_TriggerAreaName ("")
        , m_Collidable (false)
        , m_Editing (false)
    {
        memset (m_TriggerAreaWindow, 0, ARRAY_SIZE (m_TriggerAreaWindow));
        memset (m_TriggerAreaData, 0, ARRAY_SIZE (m_TriggerAreaData));
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
            Point point = m_Editor->CalculateWorldPoint (state.x, state.y);
            m_TriggerPoint->X = point.X;
            m_TriggerPoint->Y = point.Y;

            return true;
        }

        return false;
    }

    //--------------------------------------------------------------------------------------------------

    void EditorTriggerAreaMode::MarkSelectedTriggerAreas ()
    {
        ALLEGRO_MOUSE_STATE state;
        al_get_mouse_state (&state);

        SceneManager& sceneManager = m_Editor->GetMainLoop ()->GetSceneManager ();
        Point translate = sceneManager.GetCamera ().GetTranslate ();
        Point scale = sceneManager.GetCamera ().GetScale ();
        Point* selectedPoint = GetTriggerPointUnderCursor (state.x, state.y);
        std::map<std::string, TriggerArea>& triggerAreas = sceneManager.GetActiveScene ()->GetTriggerAreas ();

        for (std::map<std::string, TriggerArea>::iterator it = triggerAreas.begin (); it != triggerAreas.end (); ++it)
        {
            if (!it->second.Points.empty ())
            {
                int i = 0;
                int selectedIndex = std::numeric_limits<int>::min ();

                for (size_t j = 0; j < it->second.Points.size (); ++j)
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

                for (const Point& p : it->second.Points)
                {
                    float xPoint = p.X * scale.X - translate.X;
                    float yPoint = p.Y * scale.Y - translate.Y;

                    if (i == selectedIndex)
                    {
                        //  Mark selected corner
                        al_draw_filled_circle (xPoint, yPoint, 4, COLOR_PINK);
                    }
                    else if ((i == 0 && selectedIndex == it->second.Points.size () - 1) || (i == selectedIndex + 1))
                    {
                        //  Mark also next corner
                        al_draw_filled_circle (xPoint, yPoint, 4, COLOR_PINK);
                    }

                    if (selectedPoint != nullptr && p == *selectedPoint)
                    {
                        al_draw_filled_circle (xPoint, yPoint, 4, COLOR_RED);
                    }

                    ++i;
                }
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    Point* EditorTriggerAreaMode::GetTriggerPointUnderCursor (int mouseX, int mouseY)
    {
        int outsets = 4;
        std::map<std::string, TriggerArea>& triggerAreas
            = m_Editor->GetMainLoop ()->GetSceneManager ().GetActiveScene ()->GetTriggerAreas ();

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
            = m_Editor->GetMainLoop ()->GetSceneManager ().GetActiveScene ()->GetTriggerAreas ();

        for (std::map<std::string, TriggerArea>::iterator it = triggerAreas.begin (); it != triggerAreas.end (); ++it)
        {
            for (Point& point : it->second.Points)
            {
                if (m_Editor->IsMouseWithinPointRect (mouseX, mouseY, point, outsets))
                {
                    m_TriggerAreaName = it->first;
                    strcpy (m_TriggerAreaWindow, m_TriggerAreaName.c_str ());
                    strcpy (m_TriggerAreaData, it->second.Data.c_str ());
                    m_Collidable = it->second.Collidable;
                    m_Editing = true;

                    return &it->second;
                }
            }
        }

        m_Editing = false;

        return nullptr;
    }

    //--------------------------------------------------------------------------------------------------

    void EditorTriggerAreaMode::InsertTriggerAreaAtCursor (int mouseX, int mouseY)
    {
        SceneManager& sceneManager = m_Editor->GetMainLoop ()->GetSceneManager ();

        if (sceneManager.GetActiveScene ()->GetTriggerAreas ().empty ())
        {
            NewTriggerArea ();
        }

        if (!m_TriggerArea)
        {
            m_TriggerArea
                = m_Editor->GetMainLoop ()->GetSceneManager ().GetActiveScene ()->GetTriggerArea (m_TriggerAreaName);
        }

        if (m_TriggerArea)
        {
            Point pointToInsert = m_Editor->CalculateWorldPoint (mouseX, mouseY);
            bool inserted = false;

            //  After we select one of trigger point, we can insert next one accordingly
            Point* againSelected = GetTriggerPointUnderCursor (mouseX, mouseY);

            if (m_TriggerPoint && !againSelected)
            {
                for (size_t i = 0; i < m_TriggerArea->Points.size (); ++i)
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
    }

    //--------------------------------------------------------------------------------------------------

    bool EditorTriggerAreaMode::RemoveTriggerPointUnderCursor (int mouseX, int mouseY)
    {
        TriggerArea* triggerArea = GetTriggerAreaUnderCursor (mouseX, mouseY);
        Point* triggerPoint = GetTriggerPointUnderCursor (mouseX, mouseY);

        if (triggerArea && triggerPoint)
        {
            for (size_t i = 0; i < triggerArea->Points.size (); ++i)
            {
                if (triggerArea->Points[i] == *triggerPoint)
                {
                    triggerArea->Points.erase (triggerArea->Points.begin () + i);

                    if (triggerArea->Points.empty ())
                    {
                        m_Editor->GetMainLoop ()->GetSceneManager ().GetActiveScene ()->RemoveTriggerArea (
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

                        triggerArea->UpdatePolygons (
                            &m_Editor->GetMainLoop ()->GetPhysicsManager ().GetTriangulator ());
                    }

                    ClearSelection ();

                    return true;
                }
            }
        }

        ClearSelection ();

        return false;
    }

    //--------------------------------------------------------------------------------------------------

    void EditorTriggerAreaMode::NewTriggerArea ()
    {
        Scene* scene = m_Editor->GetMainLoop ()->GetSceneManager ().GetActiveScene ();

        m_TriggerArea = scene->AddTriggerArea (m_TriggerAreaName, m_TriggerAreaData, {}, m_Collidable);
        m_TriggerPoint = nullptr;
    }

    //--------------------------------------------------------------------------------------------------

    void EditorTriggerAreaMode::Render ()
    {
        Point translate = m_Editor->GetMainLoop ()->GetSceneManager ().GetCamera ().GetTranslate ();
        Point scale = m_Editor->GetMainLoop ()->GetSceneManager ().GetCamera ().GetScale ();
        std::map<std::string, TriggerArea> triggerAreas
            = m_Editor->GetMainLoop ()->GetSceneManager ().GetActiveScene ()->GetTriggerAreas ();

        for (std::map<std::string, TriggerArea>::iterator it = triggerAreas.begin (); it != triggerAreas.end (); ++it)
        {
            if (!it->second.Points.empty ())
            {
                int i = 0;

                std::vector<float> out;
                for (const Point& p : it->second.Points)
                {
                    float xPoint = p.X * scale.X - translate.X;
                    float yPoint = p.Y * scale.Y - translate.Y;

                    out.push_back (xPoint);
                    out.push_back (yPoint);
                }

                al_draw_polygon (out.data (), static_cast<int> (it->second.Points.size ()), 0,
                    it->second.Collidable ? COLOR_GREEN : COLOR_LIGHTBLUE, 2, 0);

                Point min {std::numeric_limits<int>::max (), std::numeric_limits<int>::max ()};
                Point max {std::numeric_limits<int>::min (), std::numeric_limits<int>::min ()};

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

                    ++i;

                    al_draw_filled_circle (xPoint, yPoint, 4, color);
                }

                m_Editor->GetMainLoop ()->GetScreen ()->GetFont ().DrawText (FONT_NAME_SMALL, it->second.Name,
                    al_map_rgb (0, 255, 0), static_cast<float> (min.X + (max.X - min.X) * 0.5f),
                    static_cast<float> (min.Y + (max.Y - min.Y) * 0.5f), 1.0f, ALLEGRO_ALIGN_CENTER);
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    void EditorTriggerAreaMode::RenderUI ()
    {
        ImGui::SetNextWindowSize (ImVec2 (400, 130));

        if (ImGui::BeginPopupModal ("Trigger Area", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::PushItemWidth (350);
            ImGui::InputText ("Name", m_TriggerAreaWindow, ARRAY_SIZE (m_TriggerAreaWindow));
            ImGui::SetItemDefaultFocus ();
            ImGui::InputText ("Data", m_TriggerAreaData, ARRAY_SIZE (m_TriggerAreaData));
            ImGui::PopItemWidth ();

            ImGui::Checkbox ("Collidable?", &m_Collidable);

            ImGui::Separator ();
            ImGui::BeginGroup ();

            if (ImGui::Button ("ACCEPT", ImVec2 (50.f, 18.f)))
            {
                if (m_Editing)
                {
                    EditTriggerArea ();
                }
                else
                {
                    m_TriggerAreaName = m_TriggerAreaWindow;

                    NewTriggerArea ();
                    m_Editor->SetCursorMode (CursorMode::EditTriggerAreaMode);
                }

                ImGui::CloseCurrentPopup ();
            }

            ImGui::SameLine ();

            if (ImGui::Button ("CANCEL", ImVec2 (50.f, 18.f)) || m_Editor->IsCloseCurrentPopup ())
            {
                m_Editor->SetCursorMode (CursorMode::ActorSelectMode);
                m_TriggerAreaName = "";

                ImGui::CloseCurrentPopup ();
                m_Editor->SetCloseCurrentPopup (false);
            }
            ImGui::EndGroup ();

            ImGui::EndPopup ();
        }
    }

    void EditorTriggerAreaMode::EditTriggerArea ()
    {
        Scene* scene = m_Editor->GetMainLoop ()->GetSceneManager ().GetActiveScene ();
        TriggerArea* editedTriggerArea = scene->GetTriggerArea (m_TriggerAreaName);

        if (editedTriggerArea)
        {
            if (m_TriggerAreaName != std::string (m_TriggerAreaWindow))
            {
                TriggerArea* newTriggerArea = scene->AddTriggerArea (
                    m_TriggerAreaWindow, m_TriggerAreaData, editedTriggerArea->Points, m_Collidable);
                newTriggerArea->Polygons = editedTriggerArea->Polygons;
                newTriggerArea->OnEnterCallback = editedTriggerArea->OnEnterCallback;
                newTriggerArea->OnLeaveCallback = editedTriggerArea->OnLeaveCallback;
                newTriggerArea->ScriptOnEnterCallback = editedTriggerArea->ScriptOnEnterCallback;
                newTriggerArea->ScriptOnLeaveCallback = editedTriggerArea->ScriptOnLeaveCallback;

                //  Cleanup previous area
                scene->RemoveTriggerArea (m_TriggerAreaName);

                editedTriggerArea = newTriggerArea;
            }

            if (m_Collidable != editedTriggerArea->Collidable)
            {
                editedTriggerArea->Collidable = m_Collidable;
            }

            if (m_TriggerAreaData != editedTriggerArea->Data)
            {
                editedTriggerArea->Data = m_TriggerAreaData;
            }
        }

        m_Editing = false;
        m_Editor->SetCursorMode (CursorMode::ActorSelectMode);
    }

    //--------------------------------------------------------------------------------------------------

    Point* EditorTriggerAreaMode::GetTriggerPoint () { return m_TriggerPoint; }

    //--------------------------------------------------------------------------------------------------

    void EditorTriggerAreaMode::SetTriggerPoint (Point* point) { m_TriggerPoint = point; }

    //--------------------------------------------------------------------------------------------------

    TriggerArea* EditorTriggerAreaMode::GetTriggerArea () { return m_TriggerArea; }

    //--------------------------------------------------------------------------------------------------

    void EditorTriggerAreaMode::SetTriggerArea (TriggerArea* area) { m_TriggerArea = area; }

    //--------------------------------------------------------------------------------------------------

    std::string EditorTriggerAreaMode::GetTriggerAreaName () { return m_TriggerAreaName; }

    //--------------------------------------------------------------------------------------------------

    void EditorTriggerAreaMode::SetTriggerAreaName (const std::string& name) { m_TriggerAreaName = name; }

    //--------------------------------------------------------------------------------------------------

    void EditorTriggerAreaMode::SetTriggerAreaCollidable (bool collidable) { m_Collidable = collidable; }

    //--------------------------------------------------------------------------------------------------

    bool EditorTriggerAreaMode::IsTriggerAreaCollidable () const { return m_Collidable; }

    //--------------------------------------------------------------------------------------------------

    void EditorTriggerAreaMode::ClearSelection ()
    {
        m_TriggerAreaName = "";
        m_TriggerPoint = nullptr;
        m_TriggerArea = nullptr;

        m_Editor->SetCursorMode (CursorMode::ActorSelectMode);
    }

    //--------------------------------------------------------------------------------------------------
}
