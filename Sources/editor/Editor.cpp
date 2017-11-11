// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "Editor.h"
#include "AtlasManager.h"
#include "MainLoop.h"
#include "Screen.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_a5.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    bool askNewScene = false;
    char menuFileName[512] = "0_home/0_0_home.scn";

    //--------------------------------------------------------------------------------------------------
    //--------------------------------------------------------------------------------------------------

    Editor::Editor (MainLoop* mainLoop)
        : m_EditorTileMode (this)
        , m_EditorPhysMode (this)
        , m_EditorFlagPointMode (this)
        , m_EditorTriggerAreaMode (this)
        , m_MainLoop (mainLoop)
        , m_IsSnapToGrid (true)
        , m_IsMousePan (false)
        , m_IsMouseWheel (false)
        , m_BaseGridSize (16)
        , m_GridSize (16)
        , m_CursorMode (CursorMode::TileSelectMode)
    {
    }

    //--------------------------------------------------------------------------------------------------

    Editor::~Editor ()
    {
        if (!IsDestroyed ())
        {
            Destroy ();
        }
    }

    //--------------------------------------------------------------------------------------------------

    bool Editor::Initialize ()
    {
        Lifecycle::Initialize ();

        m_EditorTileMode.InitializeUI ();

        ImGui_ImplA5_Init (m_MainLoop->GetScreen ()->GetDisplay ());
        ImGui::GetIO ().IniFilename = nullptr;

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    bool Editor::Destroy ()
    {
        ImGui_ImplA5_Shutdown ();

        return Lifecycle::Destroy ();
    }

    //--------------------------------------------------------------------------------------------------

    bool Editor::Update (float deltaTime)
    {
        ALLEGRO_KEYBOARD_STATE state;
        al_get_keyboard_state (&state);

        int delta = 5;

        if (al_key_down (&state, ALLEGRO_KEY_LSHIFT))
        {
            delta *= 3;
        }

        if (al_key_down (&state, ALLEGRO_KEY_DOWN))
        {
            m_MainLoop->GetSceneManager ().GetCamera ().Move (0, -delta);
        }

        if (al_key_down (&state, ALLEGRO_KEY_UP))
        {
            m_MainLoop->GetSceneManager ().GetCamera ().Move (0, delta);
        }

        if (al_key_down (&state, ALLEGRO_KEY_RIGHT))
        {
            m_MainLoop->GetSceneManager ().GetCamera ().Move (-delta, 0);
        }

        if (al_key_down (&state, ALLEGRO_KEY_LEFT))
        {
            m_MainLoop->GetSceneManager ().GetCamera ().Move (delta, 0);
        }

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    bool openTest = false;
    bool saveRequested = false;

    void Editor::ProcessEvent (ALLEGRO_EVENT* event, float deltaTime)
    {
        ImGui_ImplA5_ProcessEvent (event);

        if (ImGui::GetIO ().WantCaptureMouse)
        {
            return;
        }

        bool tileSelected = false;
        if (event->type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN)
        {
            if (event->mouse.button == 1 && m_EditorTileMode.m_IsDrawTiles)
            {
                tileSelected = m_EditorTileMode.ChooseTile (event->mouse.x, event->mouse.y);
            }
        }

        if (event->type == ALLEGRO_EVENT_KEY_CHAR)
        {
            switch (event->keyboard.keycode)
            {
            case ALLEGRO_KEY_R:
            {
                m_EditorTileMode.ChangeRotation (event->keyboard.modifiers == ALLEGRO_KEYMOD_SHIFT);
                break;
            }

            case ALLEGRO_KEY_G:
            {
                ChangeGridSize (event->keyboard.modifiers == ALLEGRO_KEYMOD_SHIFT);
                break;
            }

            case ALLEGRO_KEY_Z:
            {
                m_EditorTileMode.ChangeZOrder (event->keyboard.modifiers == ALLEGRO_KEYMOD_SHIFT);
                break;
            }

            case ALLEGRO_KEY_X:
            {
                m_EditorTileMode.RemoveSelectedTile ();
                break;
            }

            case ALLEGRO_KEY_C:
            {
                m_EditorTileMode.CopySelectedTile ();
                break;
            }

            case ALLEGRO_KEY_S:
            {
                if (event->keyboard.modifiers == ALLEGRO_KEYMOD_CTRL)
                {
                    saveRequested = true;
                }
                else
                {
                    m_IsSnapToGrid = !m_IsSnapToGrid;
                }
                break;
            }

            case ALLEGRO_KEY_P:
            {
                m_MainLoop->GetSceneManager ().GetActiveScene ()->SetDrawPhysData (
                    !m_MainLoop->GetSceneManager ().GetActiveScene ()->IsDrawPhysData ());
                break;
            }
            }
        }

        if (event->type == ALLEGRO_EVENT_KEY_UP)
        {
            switch (event->keyboard.keycode)
            {
            case ALLEGRO_KEY_F1:
            {
                MenuItemPlay ();
                break;
            }

            case ALLEGRO_KEY_F5:
            {
                m_EditorTileMode.m_IsDrawTiles = !m_EditorTileMode.m_IsDrawTiles;
                break;
            }

            case ALLEGRO_KEY_SPACE:
            {
                break;
            }

            case ALLEGRO_KEY_TAB:
            {
                if (m_EditorTileMode.m_SelectedTile)
                {
                    if (m_CursorMode != CursorMode::EditPhysBodyMode)
                    {
                        m_CursorMode = CursorMode::EditPhysBodyMode;
                    }
                    else
                    {
                        m_CursorMode = CursorMode::TileSelectMode;
                    }
                }
                break;
            }
            }
        }

        if (event->type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN)
        {
            m_IsMousePan = event->mouse.button == 3;

            if (event->mouse.button == 1)
            {
                if (m_CursorMode == CursorMode::TileSelectMode && m_EditorFlagPointMode.m_FlagPoint == "")
                {
                    Rect r;
                    m_EditorTileMode.m_SelectedTile
                        = m_EditorTileMode.GetTileUnderCursor (event->mouse.x, event->mouse.y, std::move (r));

                    if (m_EditorTileMode.m_SelectedTile)
                    {
                        m_CursorMode = CursorMode::TileEditMode;
                        m_EditorTileMode.m_Rotation = m_EditorTileMode.m_SelectedTile->Rotation;
                    }
                }
                else if (m_CursorMode == CursorMode::TileEditMode && !tileSelected)
                {
                    Rect r;
                    Tile* newSelectedTile
                        = m_EditorTileMode.GetTileUnderCursor (event->mouse.x, event->mouse.y, std::move (r));

                    if (newSelectedTile != m_EditorTileMode.m_SelectedTile || !newSelectedTile)
                    {
                        if (newSelectedTile)
                        {
                            m_EditorTileMode.m_SelectedTile = newSelectedTile;
                            m_CursorMode = CursorMode::TileEditMode;
                            m_EditorTileMode.m_Rotation = m_EditorTileMode.m_SelectedTile->Rotation;
                        }
                        else
                        {
                            m_CursorMode = CursorMode::TileSelectMode;
                            m_EditorTileMode.m_SelectedTile = nullptr;
                        }
                    }
                }
                else if (m_CursorMode == CursorMode::EditPhysBodyMode)
                {
                    m_EditorPhysMode.InsertPhysPointAtCursor (event->mouse.x, event->mouse.y);
                }
                else if (m_CursorMode == CursorMode::EditFlagPointsMode)
                {
                    m_EditorFlagPointMode.InsertFlagPointAtCursor (event->mouse.x, event->mouse.y);
                }
                else if (m_CursorMode == CursorMode::EditTriggerAreaMode)
                {
                    m_EditorTriggerAreaMode.InsertTriggerAreaAtCursor (event->mouse.x, event->mouse.y);
                }

                m_EditorFlagPointMode.m_FlagPoint
                    = m_EditorFlagPointMode.GetFlagPointUnderCursor (event->mouse.x, event->mouse.y);

                m_EditorTriggerAreaMode.m_TriggerPoint
                    = m_EditorTriggerAreaMode.GetTriggerPointUnderCursor (event->mouse.x, event->mouse.y);
                m_EditorTriggerAreaMode.m_TriggerArea
                    = m_EditorTriggerAreaMode.GetTriggerAreaUnderCursor (event->mouse.x, event->mouse.y);

                if (m_EditorTriggerAreaMode.m_TriggerPoint && m_EditorTriggerAreaMode.m_TriggerArea)
                {
                    m_CursorMode = CursorMode::EditTriggerAreaMode;
                }
            }

            if (event->mouse.button == 2)
            {
                bool flagPointRemoved
                    = m_EditorFlagPointMode.RemoveFlagPointUnderCursor (event->mouse.x, event->mouse.y);
                bool triggerPointRemoved
                    = m_EditorTriggerAreaMode.RemoveTriggerPointUnderCursor (event->mouse.x, event->mouse.y);
                bool physPointRemoved = false;

                if (m_CursorMode == CursorMode::EditPhysBodyMode)
                {
                    physPointRemoved = m_EditorPhysMode.RemovePhysPointUnderCursor (event->mouse.x, event->mouse.y);
                }

                if (!flagPointRemoved && !triggerPointRemoved && !physPointRemoved)
                {
                    m_CursorMode = CursorMode::TileSelectMode;
                }
            }
        }
        else if (event->type == ALLEGRO_EVENT_MOUSE_BUTTON_UP)
        {
            m_IsMousePan = false;

            if (m_EditorPhysMode.m_PhysPoint && m_EditorTileMode.m_SelectedTile && event->mouse.button == 1)
            {
                m_EditorTileMode.m_SelectedTile->UpdatePhysPolygon ();

                if (!m_EditorPhysMode.m_PhysPoint && m_EditorPhysMode.m_PhysPoly
                    && !(*m_EditorPhysMode.m_PhysPoly).empty ())
                {
                    m_EditorPhysMode.m_PhysPoint = &(*m_EditorPhysMode.m_PhysPoly)[0];
                }
            }

            if (m_EditorTriggerAreaMode.m_TriggerArea && event->mouse.button == 1)
            {
                m_EditorTriggerAreaMode.m_TriggerArea->UpdatePolygons (
                    &m_MainLoop->GetPhysicsManager ().GetTriangulator ());

                if (!m_EditorTriggerAreaMode.m_TriggerPoint && m_EditorTriggerAreaMode.m_TriggerArea
                    && !m_EditorTriggerAreaMode.m_TriggerArea->Points.empty ())
                {
                    m_EditorTriggerAreaMode.m_TriggerPoint = &m_EditorTriggerAreaMode.m_TriggerArea->Points[0];
                }
            }
        }
        else if (event->type == ALLEGRO_EVENT_MOUSE_AXES)
        {
            if (!m_EditorFlagPointMode.MoveSelectedFlagPoint () && !m_EditorTriggerAreaMode.MoveSelectedTriggerPoint ())
            {
                if (m_CursorMode == CursorMode::TileEditMode)
                {
                    m_EditorTileMode.MoveSelectedTile ();
                }
                else if (m_CursorMode == CursorMode::EditPhysBodyMode)
                {
                    m_EditorPhysMode.MoveSelectedPhysPoint ();
                }
            }

            HandleCameraMovement (event->mouse);
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::HandleCameraMovement (const ALLEGRO_MOUSE_EVENT& event)
    {
        if (event.dz < 0.0)
        {
            m_MainLoop->GetSceneManager ().GetCamera ().Scale (0.75f, 0.75f, event.x, event.y);
            m_GridSize = std::max (1.0f, m_BaseGridSize * m_MainLoop->GetSceneManager ().GetCamera ().GetScale ().X);
        }
        else if (event.dz > 0.0)
        {
            m_MainLoop->GetSceneManager ().GetCamera ().Scale (1.25f, 1.25f, event.x, event.y);
            m_GridSize = std::max (1.0f, m_BaseGridSize * m_MainLoop->GetSceneManager ().GetCamera ().GetScale ().X);
        }

        if (m_IsMousePan)
        {
            m_MainLoop->GetSceneManager ().GetCamera ().Move (event.dx, event.dy);
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::ChangeGridSize (bool clockwise)
    {
        m_BaseGridSize *= clockwise ? 0.5 : 2;
        m_BaseGridSize = std::max (1, std::min (m_BaseGridSize, 1024));
        m_GridSize = std::max (1.0f, m_BaseGridSize * m_MainLoop->GetSceneManager ().GetCamera ().GetScale ().X);
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::Render (float deltaTime)
    {
        if (m_IsSnapToGrid)
        {
            DrawGrid ();
        }

        m_MainLoop->GetSceneManager ().Render (deltaTime);

        ALLEGRO_MOUSE_STATE state;
        al_get_mouse_state (&state);

        m_MainLoop->GetSceneManager ().GetCamera ().UseIdentityTransform ();

        if (m_EditorTileMode.m_SelectedTile)
        {
            Rect b = m_EditorTileMode.GetRenderBounds (m_EditorTileMode.m_SelectedTile);
            al_draw_rectangle (
                b.Transform.Pos.X, b.Transform.Pos.Y, b.Transform.Size.Width, b.Transform.Size.Height, COLOR_RED, 2);
        }

        if (m_CursorMode == CursorMode::TileSelectMode)
        {
            Rect r;
            m_EditorTileMode.m_TileUnderCursor = m_EditorTileMode.GetTileUnderCursor (state.x, state.y, std::move (r));

            if (m_EditorTileMode.m_TileUnderCursor)
            {
                al_draw_rectangle (r.Transform.Pos.X, r.Transform.Pos.Y, r.Transform.Size.Width,
                    r.Transform.Size.Height, COLOR_YELLOW, 2);
            }
        }
        else if (m_CursorMode == CursorMode::EditPhysBodyMode)
        {
            m_EditorPhysMode.DrawPhysBody (state.x, state.y);
        }

        m_EditorFlagPointMode.DrawFlagPoints (state.x, state.y);
        m_EditorTriggerAreaMode.DrawTriggerAreas (state.x, state.y);

        if (m_EditorTileMode.m_IsDrawTiles)
        {
            m_EditorTileMode.DrawTiles ();
            RenderUI ();
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::DrawGrid ()
    {
        const ALLEGRO_COLOR LIGHT_GRAY{ 0.5f, 0.5f, 0.5f, 1.0f };

        Point t = m_MainLoop->GetSceneManager ().GetCamera ().GetTranslate ();
        Point scale = m_MainLoop->GetSceneManager ().GetCamera ().GetScale ();

        int horBeginX = t.X / 2 * (1 / scale.X);

        if (t.X > 0)
        {
            horBeginX *= -1;
        }

        int horBeginY = t.Y / 1 * (1 / scale.Y);

        if (t.Y > 0)
        {
            horBeginY *= -1;
        }

        const Point screenSize = m_MainLoop->GetScreen ()->GetWindowSize ();

        int horEndX = 300 + std::fabs (horBeginX);
        int horEndY = screenSize.Height + std::fabs (horBeginY);

        for (int i = horBeginX; i < horEndX; ++i)
        {
            float xOffset = i * m_GridSize - t.X;

            //  |
            al_draw_line (xOffset, horBeginY, xOffset, horEndY, LIGHT_GRAY, 1);
        }

        horEndX = screenSize.Width + std::fabs (horBeginX);
        horEndY = 300 + std::fabs (horBeginY);

        for (int i = horBeginY; i < horEndY; ++i)
        {
            float yOffset = i * m_GridSize - t.Y;

            //  --
            al_draw_line (horBeginX, yOffset, horEndX, yOffset, LIGHT_GRAY, 1);
        }
    }

    //--------------------------------------------------------------------------------------------------

    bool Editor::IsMouseWithinPointRect (int mouseX, int mouseY, Point point, int outsets)
    {
        Point translate = m_MainLoop->GetSceneManager ().GetCamera ().GetTranslate ();
        Point scale = m_MainLoop->GetSceneManager ().GetCamera ().GetScale ();

        Rect r = Rect{ { point.X - outsets, point.Y - outsets }, { point.X + outsets, point.Y + outsets } };

        if (InsideRect ((mouseX + translate.X) * 1 / scale.X, (mouseY + translate.Y) * 1 / scale.Y, r))
        {
            return true;
        }

        return false;
    }

    //--------------------------------------------------------------------------------------------------

    Point Editor::CalculateCursorPoint (int mouseX, int mouseY)
    {
        Point translate = m_MainLoop->GetSceneManager ().GetCamera ().GetTranslate ();
        int finalX = (mouseX + translate.X);
        int finalY = (mouseY + translate.Y);

        if (m_IsSnapToGrid)
        {
            finalX = std::floor ((finalX + m_GridSize * 0.5) / m_GridSize) * m_GridSize;
            finalY = std::floor ((finalY + m_GridSize * 0.5) / m_GridSize) * m_GridSize;
        }

        finalX = (finalX - translate.X);
        finalY = (finalY - translate.Y);

        return { finalX, finalY };
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::OnNewScene ()
    {
        m_MainLoop->GetSceneManager ().GetActiveScene ()->Reset ();

        ResetSettings ();
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::OnLoadScene (const std::string& openFileName)
    {
        std::string path = GetDataPath () + "scenes/" + openFileName;

        if (IsFileExists (path))
        {
            Scene* scene = Scene::LoadScene (&m_MainLoop->GetSceneManager (), path);

            if (scene)
            {
                m_MainLoop->GetSceneManager ().RemoveScene (m_MainLoop->GetSceneManager ().GetActiveScene ());
                m_MainLoop->GetSceneManager ().SetActiveScene (scene);

                std::vector<Tile*>& tiles = scene->GetTiles ();
                int maxTileID = 0;

                for (Tile* t : tiles)
                {
                    if (t->ID > maxTileID)
                    {
                        maxTileID = t->ID + 1;
                    }
                }

                m_EditorTileMode.SetCurrentID (maxTileID);
                ResetSettings ();
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::OnSaveScene (const std::string& saveFileName)
    {
        std::string path = GetDataPath () + "scenes/" + saveFileName;

        Scene::SaveScene (m_MainLoop->GetSceneManager ().GetActiveScene (), path);
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::ResetSettings ()
    {
        m_EditorTileMode.ResetSettings ();
        m_EditorPhysMode.ResetSettings ();

        OnResetScale ();
        OnResetTranslate ();
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::OnPlay () { MenuItemPlay (); }

    //--------------------------------------------------------------------------------------------------

    void Editor::MenuItemPlay () { m_MainLoop->GetStateManager ().SetActiveState ("GAMEPLAY_STATE"); }

    //--------------------------------------------------------------------------------------------------

    void Editor::OnExit () { m_MainLoop->Exit (); }

    //--------------------------------------------------------------------------------------------------

    void Editor::OnResetTranslate ()
    {
        const Point& windowSize = m_MainLoop->GetScreen ()->GetWindowSize ();
        m_MainLoop->GetSceneManager ().GetCamera ().SetTranslate (windowSize.Width * 0.5, windowSize.Height * 0.5);
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::OnResetScale ()
    {
        ALLEGRO_MOUSE_STATE state;
        al_get_mouse_state (&state);

        Point scale = m_MainLoop->GetSceneManager ().GetCamera ().GetScale ();

        m_MainLoop->GetSceneManager ().GetCamera ().Scale (1 / scale.X, 1 / scale.Y, state.x, state.y);
        m_GridSize = std::max (1.0f, m_BaseGridSize * m_MainLoop->GetSceneManager ().GetCamera ().GetScale ().X);
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::OnShowGrid () { m_IsSnapToGrid = !m_IsSnapToGrid; }

    //--------------------------------------------------------------------------------------------------

    void Editor::OnGridIncrease ()
    {
        m_BaseGridSize /= 2;

        m_BaseGridSize = std::max (1, std::min (m_BaseGridSize, 1024));
        m_GridSize = std::max (1.0f, m_BaseGridSize * m_MainLoop->GetSceneManager ().GetCamera ().GetScale ().X);
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::OnGridDecrease ()
    {
        m_BaseGridSize *= 2;

        m_BaseGridSize = std::max (1, std::min (m_BaseGridSize, 1024));
        m_GridSize = std::max (1.0f, m_BaseGridSize * m_MainLoop->GetSceneManager ().GetCamera ().GetScale ().X);
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::RenderUI ()
    {
        ImGui_ImplA5_NewFrame ();

        Point translate = m_MainLoop->GetSceneManager ().GetCamera ().GetTranslate ();
        Point scale = m_MainLoop->GetSceneManager ().GetCamera ().GetScale ();
        const Point windowSize = m_MainLoop->GetScreen ()->GetWindowSize ();
        bool open = true;

        int winSize = 140.0f;
        int xOffset = 5.0f;

        ImGui::SetNextWindowPos (ImVec2 (xOffset, xOffset), ImGuiCond_FirstUseEver);
        ImGui::Begin ("FileMenu", &open, ImVec2 (winSize, 100.f), 0.0f,
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
        ImVec2 buttonSize (100, 20);

        ImGui::TextColored (ImVec4 (0, 1, 0, 1),
            std::string ("SCENE: " + m_MainLoop->GetSceneManager ().GetActiveScene ()->GetName ()).c_str ());

        static bool newSceneDontAsk = false;
        if (ImGui::Button ("NEW SCENE", buttonSize))
        {
            if (newSceneDontAsk)
            {
                OnNewScene ();
            }
            else
            {
                ImGui::OpenPopup ("Erase?");
            }
        }

        if (ImGui::BeginPopupModal ("Erase?", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text ("All those beautiful files will be deleted.\nThis operation cannot be undone!\n\n");
            ImGui::Separator ();

            ImGui::PushStyleVar (ImGuiStyleVar_FramePadding, ImVec2 (0, 0));
            ImGui::Checkbox ("Don't ask me next time", &newSceneDontAsk);
            ImGui::PopStyleVar ();

            if (ImGui::Button ("OK", ImVec2 (120, 0)))
            {
                OnNewScene ();
                ImGui::CloseCurrentPopup ();
            }

            ImGui::SameLine ();

            if (ImGui::Button ("Cancel", ImVec2 (120, 0)))
            {
                ImGui::CloseCurrentPopup ();
            }

            ImGui::EndPopup ();
        }

        if (ImGui::Button ("OPEN SCENE", buttonSize))
        {
            ImGui::OpenPopup ("Open Scene");
        }

        if (ImGui::BeginPopupModal ("Open Scene", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text ("File path:");

            if (ImGui::IsRootWindowOrAnyChildFocused () && !ImGui::IsAnyItemActive () && !ImGui::IsMouseClicked (0))
            {
                ImGui::SetKeyboardFocusHere (0);
            }

            if (ImGui::InputText ("##edit", menuFileName, sizeof (menuFileName), ImGuiInputTextFlags_EnterReturnsTrue))
            {
                OnLoadScene (menuFileName);
                ImGui::CloseCurrentPopup ();
            }

            ImGui::Separator ();

            if (ImGui::Button ("OK", ImVec2 (120, 0)))
            {
                OnLoadScene (menuFileName);
                ImGui::CloseCurrentPopup ();
            }

            ImGui::SameLine ();

            if (ImGui::Button ("Cancel", ImVec2 (120, 0)))
            {
                ImGui::CloseCurrentPopup ();
            }

            ImGui::EndPopup ();
        }

        if (ImGui::Button ("SAVE SCENE", buttonSize) || saveRequested)
        {
            ImGui::OpenPopup ("Save Scene");
            saveRequested = false;
        }

        if (ImGui::BeginPopupModal ("Save Scene", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text ("File path:");

            if (ImGui::IsRootWindowOrAnyChildFocused () && !ImGui::IsAnyItemActive () && !ImGui::IsMouseClicked (0))
            {
                ImGui::SetKeyboardFocusHere (0);
            }

            if (ImGui::InputText ("##edit", menuFileName, sizeof (menuFileName), ImGuiInputTextFlags_EnterReturnsTrue))
            {
                OnSaveScene (menuFileName);
                ImGui::CloseCurrentPopup ();
            }
            ImGui::Separator ();

            if (ImGui::Button ("OK", ImVec2 (120, 0)))
            {
                OnSaveScene (menuFileName);
                ImGui::CloseCurrentPopup ();
            }

            ImGui::SameLine ();

            if (ImGui::Button ("Cancel", ImVec2 (120, 0)))
            {
                ImGui::CloseCurrentPopup ();
            }

            ImGui::EndPopup ();
        }

        ImGui::End ();

        ImGui::SetNextWindowPos (ImVec2 (xOffset, 120), ImGuiCond_FirstUseEver);
        ImGui::Begin ("ToolbarMenu", &open, ImVec2 (winSize, 120.f), 0.0f,
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
        if (ImGui::Button ("RESET MOVE", buttonSize))
        {
            OnResetTranslate ();
        }

        if (ImGui::Button ("RESET SCALE", buttonSize))
        {
            OnResetScale ();
        }

        if (ImGui::Button (m_IsSnapToGrid ? "HIDE GRID" : "SHOW GRID", buttonSize))
        {
            OnShowGrid ();
        }

        if (ImGui::Button ("+++", ImVec2 (buttonSize.x / 2 - 4, buttonSize.y)))
        {
            OnGridIncrease ();
        }

        ImGui::SameLine ();

        if (ImGui::Button ("---", ImVec2 (buttonSize.x / 2 - 4, buttonSize.y)))
        {
            OnGridDecrease ();
        }

        ImGui::End ();

        ImGui::SetNextWindowPos (ImVec2 (xOffset, 240), ImGuiCond_FirstUseEver);
        ImGui::Begin ("Points", &open, ImVec2 (winSize, 60.f), 0.0f,
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

        if (m_CursorMode != CursorMode::EditFlagPointsMode)
        {
            if (ImGui::Button ("FLAG POINT", buttonSize))
            {
                m_EditorFlagPointMode.m_AskFlagPoint = true;
            }
        }

        if (m_EditorFlagPointMode.m_AskFlagPoint)
        {
            ImGui::OpenPopup ("Flag Point");
            m_EditorFlagPointMode.m_AskFlagPoint = false;
        }

        if (ImGui::BeginPopupModal ("Flag Point", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text ("Flag Point name:");

            if (ImGui::IsRootWindowOrAnyChildFocused () && !ImGui::IsAnyItemActive () && !ImGui::IsMouseClicked (0))
            {
                ImGui::SetKeyboardFocusHere (0);
            }

            if (ImGui::InputText ("##flagPoint", m_EditorFlagPointMode.m_FlagPointName,
                    sizeof (m_EditorFlagPointMode.m_FlagPointName), ImGuiInputTextFlags_EnterReturnsTrue))
            {
                m_CursorMode = CursorMode::EditFlagPointsMode;
                ImGui::CloseCurrentPopup ();
            }
            ImGui::Separator ();

            if (ImGui::Button ("OK", ImVec2 (120, 0)))
            {
                m_CursorMode = CursorMode::EditFlagPointsMode;
                ImGui::CloseCurrentPopup ();
            }

            ImGui::SameLine ();

            if (ImGui::Button ("Cancel", ImVec2 (120, 0)))
            {
                strset (m_EditorFlagPointMode.m_FlagPointName, 0);
                m_CursorMode = CursorMode::TileSelectMode;
                ImGui::CloseCurrentPopup ();
            }

            ImGui::EndPopup ();
        }

        if (ImGui::Button ("TRIGGER AREA", buttonSize))
        {
            strset (m_EditorTriggerAreaMode.m_TriggerAreaName, 0);
            ImGui::OpenPopup ("Trigger Area");
        }

        if (ImGui::BeginPopupModal ("Trigger Area", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text ("Trigger Area name:");

            if (ImGui::IsRootWindowOrAnyChildFocused () && !ImGui::IsAnyItemActive () && !ImGui::IsMouseClicked (0))
            {
                ImGui::SetKeyboardFocusHere (0);
            }

            if (ImGui::InputText ("##triggerArea", m_EditorTriggerAreaMode.m_TriggerAreaName,
                    sizeof (m_EditorTriggerAreaMode.m_TriggerAreaName), ImGuiInputTextFlags_EnterReturnsTrue))
            {
                m_CursorMode = CursorMode::EditTriggerAreaMode;
                m_EditorTriggerAreaMode.NewTriggerArea ();
                ImGui::CloseCurrentPopup ();
            }
            ImGui::Separator ();

            if (ImGui::Button ("OK", ImVec2 (120, 0)))
            {
                m_CursorMode = CursorMode::EditTriggerAreaMode;
                m_EditorTriggerAreaMode.NewTriggerArea ();
                ImGui::CloseCurrentPopup ();
            }

            ImGui::SameLine ();

            if (ImGui::Button ("Cancel", ImVec2 (120, 0)))
            {
                strset (m_EditorTriggerAreaMode.m_TriggerAreaName, 0);
                m_CursorMode = CursorMode::TileSelectMode;
                ImGui::CloseCurrentPopup ();
            }

            ImGui::EndPopup ();
        }

        ImGui::End ();

        ImGui::SetNextWindowPos (ImVec2 (xOffset, 300), ImGuiCond_FirstUseEver);
        ImGui::Begin ("Physics", &open, ImVec2 (winSize, 60.f), 0.0f,
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

        if (m_EditorTileMode.m_SelectedTile)
        {
            if (ImGui::Button (m_CursorMode != CursorMode::EditPhysBodyMode ? "TILE MODE" : "PHYS MODE", buttonSize))
            {
                if (m_CursorMode != CursorMode::EditPhysBodyMode)
                {
                    m_CursorMode = CursorMode::EditPhysBodyMode;
                }
                else
                {
                    m_CursorMode = CursorMode::TileSelectMode;
                }
            }

            if (m_CursorMode == CursorMode::EditPhysBodyMode)
            {
                if (ImGui::Button ("NEW POLY", buttonSize))
                {
                    m_EditorPhysMode.m_PhysPoly = nullptr;
                    m_EditorPhysMode.m_PhysPoint = nullptr;
                    m_EditorTileMode.m_SelectedTile->PhysPoints.push_back ({});
                    m_EditorPhysMode.m_PhysPoly
                        = &m_EditorTileMode.m_SelectedTile
                               ->PhysPoints[m_EditorTileMode.m_SelectedTile->PhysPoints.size () - 1];
                }
            }
        }

        ImGui::End ();

        ImGui::SetNextWindowPos (ImVec2 (xOffset, 360), ImGuiCond_FirstUseEver);
        ImGui::Begin ("GameMenu", &open, ImVec2 (winSize, 40.f), 0.0f,
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
        if (ImGui::Button ("PLAY", buttonSize))
        {
            MenuItemPlay ();
        }

        ImGui::End ();

        winSize = 140.0f;
        xOffset = windowSize.Width - winSize - 5.0f;

        ImGui::SetNextWindowPos (ImVec2 (xOffset, 5.0f), ImGuiCond_Always);
        ImGui::Begin ("ToolBox", &open, ImVec2 (winSize, 220.f), 0.0f,
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
        ImGui::SetWindowFontScale (1.2);

        ImGui::Text (" AVG: %.2f ms", 1000.0f / ImGui::GetIO ().Framerate);
        ImGui::Text (" FPS: %.1f", ImGui::GetIO ().Framerate);

        ImGui::TextColored (
            ImVec4 (0, 1, 0, 1), std::string ("   X: " + ToString (translate.X * (1 / scale.X))).c_str ());
        ImGui::TextColored (
            ImVec4 (0, 1, 0, 1), std::string ("   Y: " + ToString (translate.Y * (1 / scale.Y))).c_str ());
        ImGui::TextColored (ImVec4 (0, 1, 0, 1),
            std::string ("   W: " + ToString (m_EditorTileMode.m_SelectedAtlasRegion.Bounds.Transform.Size.Width))
                .c_str ());
        ImGui::TextColored (ImVec4 (0, 1, 0, 1),
            std::string ("   H: " + ToString (m_EditorTileMode.m_SelectedAtlasRegion.Bounds.Transform.Size.Height))
                .c_str ());
        ImGui::TextColored (ImVec4 (0, 1, 0, 1),
            std::string ("   A: "
                + (m_EditorTileMode.m_SelectedTile ? ToString (m_EditorTileMode.m_SelectedTile->Rotation) : "-"))
                .c_str ());
        ImGui::TextColored (ImVec4 (0, 1, 0, 1),
            std::string (
                "ZORD: " + (m_EditorTileMode.m_SelectedTile ? ToString (m_EditorTileMode.m_SelectedTile->ZOrder) : "-"))
                .c_str ());
        ImGui::TextColored (ImVec4 (0, 1, 0, 1), std::string ("   S: " + ToString (scale.X)).c_str ());
        ImGui::TextColored (
            ImVec4 (0, 1, 0, 1), std::string ("SNAP: " + ToString (m_IsSnapToGrid ? "YES" : "NO")).c_str ());
        ImGui::TextColored (ImVec4 (0, 1, 0, 1), std::string ("GRID: " + ToString (m_BaseGridSize)).c_str ());

        ImGui::End ();

        winSize = 150.0f;
        xOffset = windowSize.Width - winSize - 2.0f;

        ImGui::SetNextWindowPos (ImVec2 (xOffset, 230.0f), ImGuiCond_Always);
        ImGui::Begin ("ScriptsBox", &open, ImVec2 (winSize, 220.f), 0.0f,
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
        ImGui::PushItemWidth (-1);

        static int scriptsSelectedIndex = 0;

        std::vector<ScriptMetaData>& scripts = m_MainLoop->GetSceneManager ().GetActiveScene ()->GetScripts ();
        std::vector<const char*> scriptsList;

        for (auto& sc : scripts)
        {
            scriptsList.push_back (sc.Name.c_str ());
        }

        ImGui::ListBox ("##scripts", &scriptsSelectedIndex, scriptsList.data (), scriptsList.size (), 4);

        if (ImGui::Button ("RELOAD", ImVec2 (winSize - 17, 20)))
        {
            m_MainLoop->GetSceneManager ().GetActiveScene ()->ReloadScript (scriptsList[scriptsSelectedIndex]);

            std::experimental::optional<ScriptMetaData> metaScript
                = m_MainLoop->GetSceneManager ().GetActiveScene ()->GetScriptByName (scriptsList[scriptsSelectedIndex]);

            if (metaScript)
            {
                metaScript.value ().ScriptObj->Run ("void AfterLeaveScene ()");
                metaScript.value ().ScriptObj->Run ("void Start ()");
                metaScript.value ().ScriptObj->Run ("void BeforeEnterScene ()");
            }
        }

        ImGui::PopItemWidth ();
        ImGui::End ();

        if (openTest)
        {
            static bool openTestWindow = false;
            ImGui::SetNextWindowPos (ImVec2 (5, 5), ImGuiCond_FirstUseEver);
            ImGui::ShowTestWindow (&openTestWindow);
        }

        ImGui::Render ();
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::SetDrawUITiles (bool draw) { m_EditorTileMode.m_IsDrawTiles = draw; }

    //--------------------------------------------------------------------------------------------------
}
