// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "Editor.h"
#include "AtlasManager.h"
#include "EditorActorWindow.h"
#include "EditorFlagPointWindow.h"
#include "EditorOpenSceneWindow.h"
#include "EditorSaveSceneWindow.h"
#include "EditorSpeechWindow.h"
#include "EditorTriggerAreaWindow.h"
#include "EditorWindows.h"
#include "MainLoop.h"
#include "SceneLoader.h"
#include "Screen.h"
#include "actors/TileActor.h"

using json = nlohmann::json;

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    std::string configFileName = "editor_config.json";
    bool askNewScene = false;

    const float DOUBLE_CLICK_SPEED = 300;

    //--------------------------------------------------------------------------------------------------
    //--------------------------------------------------------------------------------------------------

    Editor::Editor (MainLoop* mainLoop)
        : m_EditorPhysMode (this)
        , m_EditorFlagPointMode (this)
        , m_EditorTriggerAreaMode (this)
        , m_EditorSpeechMode (this)
        , m_EditorActorMode (this)
        , m_MainLoop (mainLoop)
        , m_IsSnapToGrid (true)
        , m_IsMousePan (false)
        , m_BaseGridSize (16.0f)
        , m_GridSize (16.0f)
        , m_CursorMode (CursorMode::TileSelectMode)
        , m_LastTimeClicked (0.0f)
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

    Gwk::Controls::Label* sceneNameLabel;
    Gwk::Controls::Button* newSceneButton;
    Gwk::Controls::Button* openSceneButton;
    Gwk::Controls::Button* saveSceneButton;

    Gwk::Controls::Button* resetMoveButton;
    Gwk::Controls::Button* resetScaleButton;
    Gwk::Controls::Button* showGridButton;
    Gwk::Controls::Button* increaseGridButton;
    Gwk::Controls::Button* decreaseGridButton;

    Gwk::Controls::Button* flagPointButton;
    Gwk::Controls::Button* triggerAreaButton;

    Gwk::Controls::Button* selectModeButton;
    Gwk::Controls::Button* newPolyButton;
    Gwk::Controls::Button* saveBodyButton;
    Gwk::Controls::Button* removeBodyButton;

    Gwk::Controls::Button* speechButton;
    Gwk::Controls::Button* actorButton;

    Gwk::Controls::Button* playButton;
    Gwk::Controls::ComboBox* tilesetCombo;
    Gwk::Controls::Button* leftPrevTileButton;
    Gwk::Controls::Button* leftNextTileButton;
    Gwk::Controls::Button* rightPrevTileButton;
    Gwk::Controls::Button* rightNextTileButton;

    Gwk::Controls::Label* avgFPSLabel;
    Gwk::Controls::Label* fpsLabel;
    Gwk::Controls::Label* idLabel;
    Gwk::Controls::Label* xPosLabel;
    Gwk::Controls::Label* yPosLabel;
    Gwk::Controls::Label* widthLabel;
    Gwk::Controls::Label* heightLabel;
    Gwk::Controls::Label* angleLabel;
    Gwk::Controls::Label* zOrderLabel;
    Gwk::Controls::Label* scaleLabel;
    Gwk::Controls::Label* snapLabel;
    Gwk::Controls::Label* gridLabel;

    Gwk::Controls::ListBox* scriptsBox;
    Gwk::Controls::Button* scriptReloadButton;

    bool Editor::Initialize ()
    {
        Lifecycle::Initialize ();

        al_init_native_dialog_addon ();

        Gwk::Platform::SetPlatformWindow (m_MainLoop->GetScreen ()->GetDisplay ());

        Gwk::Platform::RelativeToExecutablePaths paths ("../../Data/");
        m_ResourceLoader = new Gwk::Renderer::AllegroResourceLoader (paths);

        m_GUIRenderer = new Gwk::Renderer::Allegro (*m_ResourceLoader);

        m_GuiSkin = new Gwk::Skin::TexturedBase (m_GUIRenderer);
        m_GuiSkin->Init ("UISkin.png");

        // The fonts work differently in Allegro - it can't use
        // system fonts. So force the skin to use a local one.
        m_GuiSkin->SetDefaultFont ("fonts/OpenSans.ttf", 12);

        // Create a Canvas (it's root, on which all other Gwork panels are created)
        const Point screenSize = m_MainLoop->GetScreen ()->GetWindowSize ();

        m_MainCanvas = new Gwk::Controls::Canvas (m_GuiSkin);
        m_MainCanvas->SetSize (screenSize.Width, screenSize.Height);
        m_MainCanvas->SetDrawBackground (false);

        m_GUIInput.Initialize (m_MainCanvas);

        //  Add GUI Controls
        sceneNameLabel = new Gwk::Controls::Label (m_MainCanvas);
        sceneNameLabel->SetTextColor (Gwk::Color (0, 255, 0, 255));
        sceneNameLabel->SetPos (20, 10);
        sceneNameLabel->SetText ("SCENE: " + m_MainLoop->GetSceneManager ().GetActiveScene ()->GetName ());
        sceneNameLabel->SizeToContents ();

        newSceneButton = new Gwk::Controls::Button (m_MainCanvas);
        newSceneButton->SetText ("NEW SCENE");
        newSceneButton->SetPos (20, 30);
        newSceneButton->onPress.Add (this, &Editor::OnNewScene);

        openSceneButton = new Gwk::Controls::Button (m_MainCanvas);
        openSceneButton->SetText ("OPEN SCENE");
        openSceneButton->SetPos (20, newSceneButton->Bottom () + 5);
        openSceneButton->onPress.Add (this, &Editor::OnOpenScene);

        saveSceneButton = new Gwk::Controls::Button (m_MainCanvas);
        saveSceneButton->SetText ("SAVE SCENE");
        saveSceneButton->SetPos (20, openSceneButton->Bottom () + 5);
        saveSceneButton->onPress.Add (this, &Editor::OnSaveScene);

        //  Diaglos & windows
        {
            m_OpenSceneWindow = new EditorOpenSceneWindow (this, m_MainCanvas, "0_home/0_0_home.scn");
            m_SaveSceneWindow = new EditorSaveSceneWindow (this, m_MainCanvas, "0_home/0_0_home.scn");
            m_FlagPointWindow = new EditorFlagPointWindow (this, m_MainCanvas);
            m_TriggerAreaWindow = new EditorTriggerAreaWindow (this, m_MainCanvas);
            m_SpeechWindow = new EditorSpeechWindow (this, m_MainCanvas);
            m_ActorWindow = new EditorActorWindow (this, m_MainCanvas);
            m_InfoWindow = new EditorInfoWindow (this, m_MainCanvas);
            m_InputWindow = new EditorInputWindow (this, m_MainCanvas);
        }

        resetMoveButton = new Gwk::Controls::Button (m_MainCanvas);
        resetMoveButton->SetText ("RESET MOVE");
        resetMoveButton->SetPos (20, saveSceneButton->Bottom () + 40);
        resetMoveButton->onPress.Add (this, &Editor::OnResetTranslate);

        resetScaleButton = new Gwk::Controls::Button (m_MainCanvas);
        resetScaleButton->SetText ("RESET SCALE");
        resetScaleButton->SetPos (20, resetMoveButton->Bottom () + 5);
        resetScaleButton->onPress.Add (this, &Editor::OnResetScale);

        showGridButton = new Gwk::Controls::Button (m_MainCanvas);
        showGridButton->SetText ("SHOW GRID");
        showGridButton->SetPos (20, resetScaleButton->Bottom () + 5);
        showGridButton->onPress.Add (this, &Editor::OnShowGrid);

        increaseGridButton = new Gwk::Controls::Button (m_MainCanvas);
        increaseGridButton->SetText ("+++");
        increaseGridButton->SetWidth (45);
        increaseGridButton->SetPos (20, showGridButton->Bottom () + 5);
        increaseGridButton->onPress.Add (this, &Editor::OnGridIncrease);

        decreaseGridButton = new Gwk::Controls::Button (m_MainCanvas);
        decreaseGridButton->SetText ("---");
        decreaseGridButton->SetWidth (45);
        decreaseGridButton->SetPos (increaseGridButton->GetPos ().x + increaseGridButton->GetSize ().x + 10,
                                    showGridButton->Bottom () + 5);
        decreaseGridButton->onPress.Add (this, &Editor::OnGridDecrease);

        flagPointButton = new Gwk::Controls::Button (m_MainCanvas);
        flagPointButton->SetText ("FLAG POINT");
        flagPointButton->SetPos (20, decreaseGridButton->Bottom () + 40);
        flagPointButton->onPress.Add (this, &Editor::OnFlagPoint);

        triggerAreaButton = new Gwk::Controls::Button (m_MainCanvas);
        triggerAreaButton->SetText ("TRIGGER AREA");
        triggerAreaButton->SetPos (20, flagPointButton->Bottom () + 5);
        triggerAreaButton->onPress.Add (this, &Editor::OnTriggerArea);

        selectModeButton = new Gwk::Controls::Button (m_MainCanvas);
        selectModeButton->SetText (m_CursorMode != CursorMode::EditPhysBodyMode ? "PHYS MODE" : "EXIT PHYS");
        selectModeButton->SetPos (20, triggerAreaButton->Bottom () + 40);
        selectModeButton->onPress.Add (this, &Editor::SwitchCursorMode);
        selectModeButton->Hide ();

        saveBodyButton = new Gwk::Controls::Button (m_MainCanvas);
        saveBodyButton->SetText ("SAVE BODY");
        saveBodyButton->SetPos (20, selectModeButton->Bottom () + 5);
        saveBodyButton->onPress.Add (this, &Editor::SwitchCursorMode);
        saveBodyButton->Hide ();

        removeBodyButton = new Gwk::Controls::Button (m_MainCanvas);
        removeBodyButton->SetText ("REMOVE BODY");
        removeBodyButton->SetPos (20, saveBodyButton->Bottom () + 5);
        removeBodyButton->onPress.Add (this, &Editor::OnRemoveBody);
        removeBodyButton->Hide ();

        newPolyButton = new Gwk::Controls::Button (m_MainCanvas);
        newPolyButton->SetText ("NEW POLY");
        newPolyButton->SetPos (20, removeBodyButton->Bottom () + 5);
        newPolyButton->onPress.Add (this, &Editor::OnNewPoly);
        newPolyButton->Hide ();

        speechButton = new Gwk::Controls::Button (m_MainCanvas);
        speechButton->SetText ("SPEECH");
        speechButton->SetPos (20, selectModeButton->Bottom () + 40);
        speechButton->onPress.Add (this, &Editor::OnSpeech);

        actorButton = new Gwk::Controls::Button (m_MainCanvas);
        actorButton->SetText ("ACTOR");
        actorButton->SetPos (20, speechButton->Bottom () + 5);
        actorButton->onPress.Add (this, &Editor::OnActor);

        playButton = new Gwk::Controls::Button (m_MainCanvas);
        playButton->SetText ("PLAY");
        playButton->SetPos (20, actorButton->Bottom () + 20);
        playButton->onPress.Add (this, &Editor::OnPlay);

        tilesetCombo = new Gwk::Controls::ComboBox (m_MainCanvas);
        tilesetCombo->SetWidth (90);
        tilesetCombo->SetKeyboardInputEnabled (false);

        std::map<std::string, Atlas*>& atlases = m_MainLoop->GetSceneManager ().GetAtlasManager ()->GetAtlases ();

        for (const auto& atlas : atlases)
        {
            tilesetCombo->AddItem (atlas.first, atlas.second->GetPath ());
        }

        tilesetCombo->onSelection.Add (this, &Editor::OnTilesetSelected);

        leftPrevTileButton = new Gwk::Controls::Button (m_MainCanvas);
        leftPrevTileButton->SetWidth (30);
        leftPrevTileButton->SetText ("<<");
        leftPrevTileButton->onPress.Add (this, &Editor::OnPlay);

        leftNextTileButton = new Gwk::Controls::Button (m_MainCanvas);
        leftNextTileButton->SetWidth (30);
        leftNextTileButton->SetText (">>");
        leftNextTileButton->onPress.Add (this, &Editor::OnPlay);

        rightPrevTileButton = new Gwk::Controls::Button (m_MainCanvas);
        rightPrevTileButton->SetWidth (30);
        rightPrevTileButton->SetText ("<<");
        rightPrevTileButton->onPress.Add (this, &Editor::OnPlay);

        rightNextTileButton = new Gwk::Controls::Button (m_MainCanvas);
        rightNextTileButton->SetWidth (30);
        rightNextTileButton->SetText (">>");
        rightNextTileButton->onPress.Add (this, &Editor::OnPlay);

        playButton = new Gwk::Controls::Button (m_MainCanvas);
        playButton->SetText ("PLAY");
        playButton->SetPos (20, actorButton->Bottom () + 20);
        playButton->onPress.Add (this, &Editor::OnPlay);

        avgFPSLabel = new Gwk::Controls::Label (m_MainCanvas);
        avgFPSLabel->SetTextColor (Gwk::Color (0, 255, 0, 255));

        fpsLabel = new Gwk::Controls::Label (m_MainCanvas);
        fpsLabel->SetTextColor (Gwk::Color (0, 255, 0, 255));

        idLabel = new Gwk::Controls::Label (m_MainCanvas);
        idLabel->SetTextColor (Gwk::Color (0, 255, 0, 255));

        xPosLabel = new Gwk::Controls::Label (m_MainCanvas);
        xPosLabel->SetTextColor (Gwk::Color (0, 255, 0, 255));

        yPosLabel = new Gwk::Controls::Label (m_MainCanvas);
        yPosLabel->SetTextColor (Gwk::Color (0, 255, 0, 255));

        widthLabel = new Gwk::Controls::Label (m_MainCanvas);
        widthLabel->SetTextColor (Gwk::Color (0, 255, 0, 255));

        heightLabel = new Gwk::Controls::Label (m_MainCanvas);
        heightLabel->SetTextColor (Gwk::Color (0, 255, 0, 255));

        angleLabel = new Gwk::Controls::Label (m_MainCanvas);
        angleLabel->SetTextColor (Gwk::Color (0, 255, 0, 255));

        zOrderLabel = new Gwk::Controls::Label (m_MainCanvas);
        zOrderLabel->SetTextColor (Gwk::Color (0, 255, 0, 255));

        scaleLabel = new Gwk::Controls::Label (m_MainCanvas);
        scaleLabel->SetTextColor (Gwk::Color (0, 255, 0, 255));

        snapLabel = new Gwk::Controls::Label (m_MainCanvas);
        snapLabel->SetTextColor (Gwk::Color (0, 255, 0, 255));

        gridLabel = new Gwk::Controls::Label (m_MainCanvas);
        gridLabel->SetTextColor (Gwk::Color (0, 255, 0, 255));

        scriptsBox = new Gwk::Controls::ListBox (m_MainCanvas);
        scriptsBox->SetKeyboardInputEnabled (true);
        //        ctrl->onRowSelected.Add(this, &ThisClass::RowSelected);

        UpdateScriptsBox ();

        scriptReloadButton = new Gwk::Controls::Button (m_MainCanvas);
        scriptReloadButton->SetText ("RELOAD");
        scriptReloadButton->onPress.Add (this, &Editor::OnReloadScript);

        m_EditorActorMode.InitializeUI ();
        m_EditorSpeechMode.Clear ();

        LoadConfig ();
        ScreenResize ();

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    bool Editor::Destroy ()
    {
        SaveConfig ();

        SAFE_DELETE (m_OpenSceneWindow);
        SAFE_DELETE (m_SaveSceneWindow);
        SAFE_DELETE (m_FlagPointWindow);
        SAFE_DELETE (m_TriggerAreaWindow);
        SAFE_DELETE (m_SpeechWindow);
        SAFE_DELETE (m_ActorWindow);
        SAFE_DELETE (m_InfoWindow);
        SAFE_DELETE (m_InputWindow);

        SAFE_DELETE (m_ResourceLoader);
        SAFE_DELETE (m_MainCanvas);
        //     SAFE_DELETE (m_GuiSkin);
        SAFE_DELETE (m_GUIRenderer);

        return Lifecycle::Destroy ();
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::LoadConfig ()
    {
        try
        {
            std::ifstream file ((GetDataPath () + configFileName).c_str ());
            json j;
            file >> j;
            file.close ();

            m_IsSnapToGrid = j["show_grid"];
            m_MainLoop->GetSceneManager ().SetDrawPhysData (j["show_physics"]);
            m_MainLoop->GetSceneManager ().SetDrawBoundingBox (j["show_bounds"]);
            m_MainLoop->GetSceneManager ().SetDrawActorsNames (j["show_names"]);
        }
        catch (const std::exception&)
        {
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::SaveConfig ()
    {
        try
        {
            json j;

            j["show_grid"] = m_IsSnapToGrid;
            j["show_physics"] = m_MainLoop->GetSceneManager ().IsDrawPhysData ();
            j["show_bounds"] = m_MainLoop->GetSceneManager ().IsDrawBoundingBox ();
            j["show_names"] = m_MainLoop->GetSceneManager ().IsDrawActorsNames ();

            // write prettified JSON to another file
            std::ofstream out ((GetDataPath () + configFileName).c_str ());
            out << std::setw (4) << j.dump (4, ' ') << "\n";
        }
        catch (const std::exception&)
        {
        }
    }

    //--------------------------------------------------------------------------------------------------

    bool Editor::Update (float deltaTime)
    {
        if (IsEditorCanvasNotCovered ())
        {
            HandleCameraPan (deltaTime);
        }

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    bool Editor::IsEditorCanvasNotCovered ()
    {
        return ((m_CursorMode == CursorMode::TileSelectMode || m_CursorMode == CursorMode::TileEditMode)
                && !m_SpeechWindow->GetSceneWindow ()->Visible () && !m_TriggerAreaWindow->GetSceneWindow ()->Visible ()
                && !m_FlagPointWindow->GetSceneWindow ()->Visible () && !m_ActorWindow->GetSceneWindow ()->Visible ());
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::HandleCameraPan (float deltaTime)
    {
        ALLEGRO_KEYBOARD_STATE state;
        al_get_keyboard_state (&state);

        int delta = 300 * deltaTime;

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
    }

    //--------------------------------------------------------------------------------------------------

    bool openTest = false;
    bool saveRequested = false;
    bool g_IsToolBoxTileSelected = false;

    void Editor::ProcessEvent (ALLEGRO_EVENT* event, float)
    {
        if (m_GUIInput.ProcessMessage (*event))
        {
            return;
        }

        g_IsToolBoxTileSelected = false;
        if (event->type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN)
        {
            if (event->mouse.button == 1 && m_EditorActorMode.IsDrawTiles ())
            {
                g_IsToolBoxTileSelected = m_EditorActorMode.ChooseTile (event->mouse.x, event->mouse.y);
            }
        }

        if (event->type == ALLEGRO_EVENT_KEY_CHAR)
        {
            switch (event->keyboard.keycode)
            {
            case ALLEGRO_KEY_R:
            {
                if (m_EditorActorMode.GetSelectedActor ())
                {
                    m_EditorActorMode.ChangeRotation (event->keyboard.modifiers == ALLEGRO_KEYMOD_SHIFT);
                }
                break;
            }

            case ALLEGRO_KEY_G:
            {
                ChangeGridSize (event->keyboard.modifiers == ALLEGRO_KEYMOD_SHIFT);
                break;
            }

            case ALLEGRO_KEY_Z:
            {
                if (m_EditorActorMode.GetSelectedActor ())
                {
                    m_EditorActorMode.ChangeZOrder (event->keyboard.modifiers == ALLEGRO_KEYMOD_SHIFT);
                }

                break;
            }

            case ALLEGRO_KEY_X:
            {
                m_EditorActorMode.RemoveSelectedTile ();
                break;
            }

            case ALLEGRO_KEY_C:
            {
                m_EditorActorMode.CopySelectedTile ();
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
                m_MainLoop->GetSceneManager ().SetDrawPhysData (!m_MainLoop->GetSceneManager ().IsDrawPhysData ());
                break;
            }

            case ALLEGRO_KEY_N:
            {
                m_MainLoop->GetSceneManager ().SetDrawActorsNames (
                    !m_MainLoop->GetSceneManager ().IsDrawActorsNames ());
                break;
            }

            case ALLEGRO_KEY_B:
            {
                m_MainLoop->GetSceneManager ().SetDrawBoundingBox (
                    !m_MainLoop->GetSceneManager ().IsDrawBoundingBox ());
                break;
            }

            case ALLEGRO_KEY_SLASH:
            {
                MarkPlayerPosition ();

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
            case ALLEGRO_KEY_SPACE:
            {
                m_EditorActorMode.SetDrawTiles (!m_EditorActorMode.IsDrawTiles ());
                break;
            }

            case ALLEGRO_KEY_TAB:
            {
                if (m_EditorActorMode.GetSelectedActor ())
                {
                    SwitchCursorMode ();
                }
                break;
            }
            }
        }

        if (event->type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN)
        {
            ProcessMouseButtonDown (event->mouse);
        }
        else if (event->type == ALLEGRO_EVENT_MOUSE_BUTTON_UP)
        {
            long currentTime = GetCurrentTime ();

            if (currentTime - m_LastTimeClicked <= DOUBLE_CLICK_SPEED)
            {
                ProcessMouseButtonDoubleClick (event->mouse);
            }
            else
            {
                ProcessMouseButtonUp (event->mouse);
            }

            m_LastTimeClicked = currentTime;
        }
        else if (event->type == ALLEGRO_EVENT_MOUSE_AXES)
        {
            ProcessMouseAxes (event->mouse);
        }
        else if (event->type == ALLEGRO_EVENT_DISPLAY_RESIZE)
        {
            ScreenResize ();
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
        m_BaseGridSize *= clockwise ? 0.5f : 2.0f;
        m_BaseGridSize = std::max (1.0f, std::min (m_BaseGridSize, 1024.0f));
        m_GridSize = std::max (1.0f, m_BaseGridSize * m_MainLoop->GetSceneManager ().GetCamera ().GetScale ().X);
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::Render (float deltaTime)
    {
        if (m_IsSnapToGrid)
        {
            DrawGrid ();
        }

        if (m_CursorMode == CursorMode::EditPhysBodyMode)
        {
            RenderPhysBodyMode (deltaTime);
        }
        else
        {
            m_MainLoop->GetSceneManager ().Render (deltaTime);
            m_MainLoop->GetSceneManager ().GetCamera ().UseIdentityTransform ();

            if (IsEditorCanvasNotCovered ())
            {
                if (m_EditorActorMode.GetSelectedActor ())
                {
                    Rect r = m_MainLoop->GetSceneManager ().GetActiveScene ()->GetRenderBounds (
                        m_EditorActorMode.GetSelectedActor (), true);

                    al_draw_rectangle (r.GetTopLeft ().X, r.GetTopLeft ().Y, r.GetBottomRight ().X,
                                       r.GetBottomRight ().Y, COLOR_RED, 2);
                }

                if (m_CursorMode == CursorMode::TileSelectMode)
                {
                    ALLEGRO_MOUSE_STATE state;
                    al_get_mouse_state (&state);

                    Rect r;
                    m_EditorActorMode.SetSelectedActor (
                        m_EditorActorMode.GetActorUnderCursor (state.x, state.y, std::move (r)));

                    if (m_EditorActorMode.GetActorUnderCursor ())
                    {
                        al_draw_rectangle (r.GetTopLeft ().X, r.GetTopLeft ().Y, r.GetBottomRight ().X,
                                           r.GetBottomRight ().Y, COLOR_YELLOW, 2);
                    }
                }
            }

            m_EditorFlagPointMode.DrawFlagPoints ();
            m_EditorTriggerAreaMode.DrawTriggerAreas ();

            if (m_EditorActorMode.IsDrawTiles ())
            {
                m_EditorActorMode.DrawTiles ();
                RenderUI ();
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::RenderPhysBodyMode (float deltaTime)
    {
        Camera& camera = m_MainLoop->GetSceneManager ().GetCamera ();
        camera.Update (deltaTime);

        if (m_EditorActorMode.GetSelectedActor ())
        {
            m_EditorActorMode.GetSelectedActor ()->Render (deltaTime);
            m_EditorActorMode.GetSelectedActor ()->DrawBounds ();
            m_EditorActorMode.GetSelectedActor ()->DrawPhysBody ();
        }

        ALLEGRO_MOUSE_STATE state;
        al_get_mouse_state (&state);

        camera.UseIdentityTransform ();
        m_EditorPhysMode.DrawPhysBody (state.x, state.y);
        RenderUI ();
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::DrawGrid ()
    {
        const ALLEGRO_COLOR LIGHT_GRAY{ 0.4f, 0.4f, 0.4f, 1.0f };

        const Point screenSize = m_MainLoop->GetScreen ()->GetWindowSize ();
        Point t = m_MainLoop->GetSceneManager ().GetCamera ().GetTranslate ();
        Point cameraCenter = m_MainLoop->GetSceneManager ().GetCamera ().GetCenter ();
        cameraCenter.X = cameraCenter.X / m_GridSize;
        cameraCenter.Y = cameraCenter.Y / m_GridSize;

        int halfSegmentsX = screenSize.Width * 0.5f / m_GridSize;
        int halfSegmentsY = screenSize.Height * 0.5f / m_GridSize;

        float horBeginX = cameraCenter.X - halfSegmentsX;
        float horEndX = cameraCenter.X + halfSegmentsX;

        float horBeginY = cameraCenter.Y - halfSegmentsY;
        float horEndY = cameraCenter.Y + halfSegmentsY;

        for (int i = horBeginX; i < horEndX; ++i)
        {
            float xOffset = i * m_GridSize - t.X;

            //  |
            al_draw_line (xOffset, 0, xOffset, screenSize.Height, LIGHT_GRAY, 1);
        }

        for (int i = horBeginY; i < horEndY; ++i)
        {
            float yOffset = i * m_GridSize - t.Y;

            //  --
            al_draw_line (0, yOffset, screenSize.Width, yOffset, LIGHT_GRAY, 1);
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
        Point scale = m_MainLoop->GetSceneManager ().GetCamera ().GetScale ();
        Point translate = m_MainLoop->GetSceneManager ().GetCamera ().GetTranslate ();

        float finalX = (mouseX + translate.X);
        float finalY = (mouseY + translate.Y);

        if (m_IsSnapToGrid)
        {
            float gridSizeScale = m_GridSize; // * 1 / scale.X;

            if (scale.X < 1.0f)
            {
                // gridSizeScale *= 1.0f - scale.X;
            }

            finalX = std::floor ((finalX) / gridSizeScale) * gridSizeScale;
            finalY = std::floor ((finalY) / gridSizeScale) * gridSizeScale;

            // std::string txt = "[" + ToString (finalX) + ", " + ToString (finalY) + "]";

            // m_MainLoop->GetScreen ()->GetFont ().DrawText (FONT_NAME_MAIN_SMALL, al_map_rgb (0, 255, 0), 120, 20,
            // txt, ALLEGRO_ALIGN_LEFT);
        }

        return { finalX / scale.X, finalY / scale.Y };
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::OnNewScene (Gwk::Controls::Base*)
    {
        std::function<void(void)> YesFunc = [&] {
            m_MainLoop->GetSceneManager ().GetActiveScene ()->Reset ();
            m_MainLoop->GetSceneManager ().GetActiveScene ()->SetName (m_InputWindow->GetText ());

            ResetSettings ();
            UpdateScriptsBox ();

            sceneNameLabel->SetText (std::string ("SCENE: ")
                                     + m_MainLoop->GetSceneManager ().GetActiveScene ()->GetName ());
            sceneNameLabel->SizeToContents ();
        };

        m_InputWindow->Show ("Are you sure clearing current scene?", "", YesFunc, nullptr);
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::OnOpenScene (Gwk::Controls::Base* control) { m_OpenSceneWindow->Show (); }

    //--------------------------------------------------------------------------------------------------

    void Editor::OnSaveScene (Gwk::Controls::Base* control) { m_SaveSceneWindow->Show (); }

    //--------------------------------------------------------------------------------------------------

    void Editor::LoadScene (const std::string& openFileName)
    {
        std::string path = GetDataPath () + "scenes/" + openFileName;

        if (IsFileExists (path))
        {
            Scene* scene = SceneLoader::LoadScene (&m_MainLoop->GetSceneManager (), path);

            if (scene)
            {
                m_MainLoop->GetSceneManager ().RemoveScene (m_MainLoop->GetSceneManager ().GetActiveScene ());
                m_MainLoop->GetSceneManager ().SetActiveScene (scene);

                ResetSettings ();

                sceneNameLabel->SetText (
                    std::string ("SCENE: " + m_MainLoop->GetSceneManager ().GetActiveScene ()->GetName ()));
                sceneNameLabel->SizeToContents ();

                UpdateScriptsBox ();
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::SaveScene (const std::string& filePath)
    {
        std::string path = GetDataPath () + "scenes/" + filePath;
        SceneLoader::SaveScene (m_MainLoop->GetSceneManager ().GetActiveScene (), path);
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::ResetSettings ()
    {
        m_EditorActorMode.ResetSettings ();
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

        m_MainLoop->GetSceneManager ().GetCamera ().Scale (1.0f / scale.X, 1.0f / scale.Y, state.x, state.y);
        m_GridSize = std::max (1.0f, m_BaseGridSize * m_MainLoop->GetSceneManager ().GetCamera ().GetScale ().X);
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::OnShowGrid ()
    {
        m_IsSnapToGrid = !m_IsSnapToGrid;
        showGridButton->SetText (m_IsSnapToGrid ? "HIDE GRID" : "SHOW GRID");
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::OnGridIncrease ()
    {
        m_BaseGridSize /= 2.0f;

        m_BaseGridSize = std::max (1.0f, std::min (m_BaseGridSize, 1024.0f));
        m_GridSize = std::max (1.0f, m_BaseGridSize * m_MainLoop->GetSceneManager ().GetCamera ().GetScale ().X);
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::OnGridDecrease ()
    {
        m_BaseGridSize *= 2.0f;

        m_BaseGridSize = std::max (1.0f, std::min (m_BaseGridSize, 1024.0f));
        m_GridSize = std::max (1.0f, m_BaseGridSize * m_MainLoop->GetSceneManager ().GetCamera ().GetScale ().X);
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::OnFlagPoint () { m_FlagPointWindow->Show (); }

    //--------------------------------------------------------------------------------------------------

    void Editor::OnTriggerArea () { m_TriggerAreaWindow->Show (); }

    //--------------------------------------------------------------------------------------------------

    void Editor::SwitchCursorMode ()
    {
        if (m_CursorMode != CursorMode::EditPhysBodyMode)
        {
            Camera& camera = m_MainLoop->GetSceneManager ().GetCamera ();
            Point scale = camera.GetScale ();
            Point screenSize = m_MainLoop->GetScreen ()->GetWindowSize ();
            Rect bounds;

            if (m_EditorActorMode.GetSelectedActor ())
            {
                bounds = m_EditorActorMode.GetSelectedActor ()->Bounds;
            }

            camera.SetTranslate (screenSize.Width * 0.5 - bounds.Pos.X * scale.X,
                                 screenSize.Height * 0.5 - bounds.Pos.Y * scale.Y);

            m_CursorMode = CursorMode::EditPhysBodyMode;
        }
        else
        {
            m_CursorMode = CursorMode::TileSelectMode;
        }

        SetDrawUITiles (m_CursorMode == CursorMode::TileSelectMode);
        flagPointButton->SetHidden (m_CursorMode == CursorMode::EditPhysBodyMode);
        triggerAreaButton->SetHidden (m_CursorMode == CursorMode::EditPhysBodyMode);
        speechButton->SetHidden (m_CursorMode == CursorMode::EditPhysBodyMode);
        actorButton->SetHidden (m_CursorMode == CursorMode::EditPhysBodyMode);
        scriptsBox->SetHidden (m_CursorMode == CursorMode::EditPhysBodyMode);
        scriptReloadButton->SetHidden (m_CursorMode == CursorMode::EditPhysBodyMode);
        newPolyButton->SetHidden (m_CursorMode != CursorMode::EditPhysBodyMode);
        saveBodyButton->SetHidden (m_CursorMode != CursorMode::EditPhysBodyMode);
        removeBodyButton->SetHidden (m_CursorMode != CursorMode::EditPhysBodyMode);

        selectModeButton->SetText (m_CursorMode != CursorMode::EditPhysBodyMode ? "PHYS MODE" : "EXIT PHYS");
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::OnRemoveBody ()
    {
        if (m_EditorActorMode.GetSelectedActor ())
        {
            m_EditorActorMode.GetSelectedActor ()->PhysPoints.clear ();

            if (m_EditorPhysMode.GetPhysPoly ())
            {
                m_EditorPhysMode.GetPhysPoly ()->clear ();
            }

            m_EditorPhysMode.SetPhysPoint (nullptr);
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::OnNewPoly ()
    {
        m_EditorPhysMode.SetPhysPoly (nullptr);
        m_EditorPhysMode.SetPhysPoint (nullptr);

        if (m_EditorActorMode.GetSelectedActor ())
        {
            m_EditorActorMode.GetSelectedActor ()->PhysPoints.push_back ({});
            m_EditorPhysMode.SetPhysPoly (
                &m_EditorActorMode.GetSelectedActor ()
                     ->PhysPoints[m_EditorActorMode.GetSelectedActor ()->PhysPoints.size () - 1]);
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::OnSpeech ()
    {
        m_SpeechWindow->UpdateSpeechesTree ();
        m_SpeechWindow->Show ();
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::OnActor ()
    {
        m_ActorWindow->Show ();

        if (m_EditorActorMode.GetSelectedActor ())
        {
            m_ActorWindow->SelectActor (m_EditorActorMode.GetSelectedActor ()->Name);
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::UpdateScriptsBox ()
    {
        scriptsBox->Clear ();

        std::vector<ScriptMetaData>& scripts = m_MainLoop->GetSceneManager ().GetActiveScene ()->GetScripts ();

        for (auto& sc : scripts)
        {
            scriptsBox->AddItem ((std::string ("> ") + sc.Name));
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::OnReloadScript ()
    {
        if (scriptsBox->GetSelectedRow ())
        {
            std::vector<ScriptMetaData>& scripts = m_MainLoop->GetSceneManager ().GetActiveScene ()->GetScripts ();
            std::string name = scripts[scriptsBox->GetSelectedRowIndex ()].Name;

            m_MainLoop->GetSceneManager ().GetActiveScene ()->ReloadScript (name);

            if (strlen (g_ScriptErrorBuffer) != 0)
            {
                m_InfoWindow->Show (g_ScriptErrorBuffer);
                memset (g_ScriptErrorBuffer, 0, sizeof (g_ScriptErrorBuffer));
            }
            else
            {
#ifdef _MSC_VER
                std::optional<ScriptMetaData> metaScript
                    = m_MainLoop->GetSceneManager ().GetActiveScene ()->GetScriptByName (name);
#else
                std::experimental::optional<ScriptMetaData> metaScript
                    = m_MainLoop->GetSceneManager ().GetActiveScene ()->GetScriptByName (name);
#endif

                if (metaScript)
                {
                    (*metaScript).ScriptObj->Run ("void AfterLeaveScene ()");
                    (*metaScript).ScriptObj->Run ("void Start ()");
                    (*metaScript).ScriptObj->Run ("void BeforeEnterScene ()");
                }
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::MarkPlayerPosition ()
    {
        ALLEGRO_MOUSE_STATE state;
        al_get_mouse_state (&state);

        Point point = CalculateCursorPoint (state.x, state.y);

        m_MainLoop->GetSceneManager ().GetPlayer ().SetPosition (point);
        m_MainLoop->GetSceneManager ().GetPlayer ().TemplateBounds.Pos
            = m_MainLoop->GetSceneManager ().GetPlayer ().GetPosition ();
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::RenderUI ()
    {
        Point translate = m_MainLoop->GetSceneManager ().GetCamera ().GetTranslate ();
        Point scale = m_MainLoop->GetSceneManager ().GetCamera ().GetScale ();

        avgFPSLabel->SetText (Gwk::Utility::Format ("    AVG: %.2f ms", 1000.0f / m_MainLoop->GetScreen ()->GetFPS ()));
        avgFPSLabel->SizeToContents ();

        fpsLabel->SetText (Gwk::Utility::Format ("    FPS: %.1f", m_MainLoop->GetScreen ()->GetFPS ()));
        fpsLabel->SizeToContents ();

        Entity* selectedEntity = nullptr;

        if (m_EditorActorMode.GetSelectedActor ())
        {
            selectedEntity = m_EditorActorMode.GetSelectedActor ();
        }

        idLabel->SetText (std::string ("       ID: " + (selectedEntity ? ToString (selectedEntity->ID) : "-")));
        idLabel->SizeToContents ();

        xPosLabel->SetText (std::string ("        X: " + ToString (translate.X * (1 / scale.X))));
        xPosLabel->SizeToContents ();

        yPosLabel->SetText (std::string ("        Y: " + ToString (translate.Y * (1 / scale.Y))));
        yPosLabel->SizeToContents ();

        widthLabel->SetText (std::string (
            "       W: " + ToString (m_EditorActorMode.GetSelectedAtlasRegion ().Bounds.GetSize ().Width)));
        widthLabel->SizeToContents ();

        heightLabel->SetText (std::string (
            "        H: " + ToString (m_EditorActorMode.GetSelectedAtlasRegion ().Bounds.GetSize ().Height)));
        heightLabel->SizeToContents ();

        angleLabel->SetText (
            std::string ("        A: " + (selectedEntity ? ToString (selectedEntity->Rotation) : "-")));
        angleLabel->SizeToContents ();

        zOrderLabel->SetText (std::string ("ZORD: " + (selectedEntity ? ToString (selectedEntity->ZOrder) : "-")));
        zOrderLabel->SizeToContents ();

        scaleLabel->SetText (std::string ("         S: " + ToString (scale.X)));
        scaleLabel->SizeToContents ();

        snapLabel->SetText (std::string ("SNAP: " + ToString (m_IsSnapToGrid ? "YES" : "NO")));
        snapLabel->SizeToContents ();

        gridLabel->SetText (std::string (" GRID: " + ToString (m_BaseGridSize)));
        gridLabel->SizeToContents ();

        m_MainCanvas->RenderCanvas ();
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::SetDrawUITiles (bool draw)
    {
        m_EditorActorMode.SetDrawTiles (draw);
        tilesetCombo->SetHidden (!m_EditorActorMode.IsDrawTiles ());
        leftPrevTileButton->SetHidden (!m_EditorActorMode.IsDrawTiles ());
        leftNextTileButton->SetHidden (!m_EditorActorMode.IsDrawTiles ());
        rightPrevTileButton->SetHidden (!m_EditorActorMode.IsDrawTiles ());
        rightNextTileButton->SetHidden (!m_EditorActorMode.IsDrawTiles ());
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::ScreenResize ()
    {
        const Point screenSize = m_MainLoop->GetScreen ()->GetWindowSize ();
        m_MainCanvas->SetSize (screenSize.Width, screenSize.Height);

        avgFPSLabel->SetPos (m_MainCanvas->Width () - 120.0f, 10);
        fpsLabel->SetPos (m_MainCanvas->Width () - 120.0f, avgFPSLabel->Bottom () + 5);
        idLabel->SetPos (m_MainCanvas->Width () - 120.0f, fpsLabel->Bottom () + 10);
        xPosLabel->SetPos (m_MainCanvas->Width () - 120.0f, idLabel->Bottom () + 5);
        yPosLabel->SetPos (m_MainCanvas->Width () - 120.0f, xPosLabel->Bottom () + 5);
        widthLabel->SetPos (m_MainCanvas->Width () - 120.0f, yPosLabel->Bottom () + 5);
        heightLabel->SetPos (m_MainCanvas->Width () - 120.0f, widthLabel->Bottom () + 5);
        angleLabel->SetPos (m_MainCanvas->Width () - 120.0f, heightLabel->Bottom () + 5);
        zOrderLabel->SetPos (m_MainCanvas->Width () - 120.0f, angleLabel->Bottom () + 5);
        scaleLabel->SetPos (m_MainCanvas->Width () - 120.0f, zOrderLabel->Bottom () + 5);
        snapLabel->SetPos (m_MainCanvas->Width () - 120.0f, scaleLabel->Bottom () + 5);
        gridLabel->SetPos (m_MainCanvas->Width () - 120.0f, snapLabel->Bottom () + 5);
        scriptsBox->SetBounds (m_MainCanvas->Width () - 150.0f, gridLabel->Bottom () + 10, 140, 100);
        scriptReloadButton->SetPos (m_MainCanvas->Width () - 130.0f, scriptsBox->Bottom () + 5);

        float beginning = screenSize.Width * 0.5 - (TILES_COUNT - 1) * 0.5 * TILE_SIZE - TILE_SIZE * 0.5;

        tilesetCombo->SetPos (beginning - 140, m_MainCanvas->Bottom () - 35);
        leftPrevTileButton->SetPos (beginning - 35, m_MainCanvas->Bottom () - TILE_SIZE + 5);
        leftNextTileButton->SetPos (beginning - 35, leftPrevTileButton->Bottom () + 2);
        rightPrevTileButton->SetPos (beginning + TILES_COUNT * TILE_SIZE + 5, m_MainCanvas->Bottom () - TILE_SIZE + 5);
        rightNextTileButton->SetPos (beginning + TILES_COUNT * TILE_SIZE + 5, rightPrevTileButton->Bottom () + 2);
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::ProcessMouseButtonDoubleClick (ALLEGRO_MOUSE_EVENT& event)
    {
        if (m_CursorMode != EditPhysBodyMode && m_EditorActorMode.GetSelectedActor ())
        {
            OnActor ();
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::ProcessMouseButtonDown (ALLEGRO_MOUSE_EVENT& event)
    {
        m_IsMousePan = event.button == 3;

        if (event.button == 1)
        {
            if (m_CursorMode == CursorMode::TileSelectMode && m_EditorFlagPointMode.GetFlagPoint () == "")
            {
                m_EditorActorMode.SetSelectedActor (nullptr);

                Rect r;
                m_EditorActorMode.SetSelectedActor (
                    m_EditorActorMode.GetActorUnderCursor (event.x, event.y, std::move (r)));

                if (m_EditorActorMode.GetSelectedActor ())
                {
                    m_CursorMode = CursorMode::TileEditMode;
                    m_EditorActorMode.SetRotation (m_EditorActorMode.GetSelectedActor ()->Rotation);

                    if (m_EditorActorMode.GetSelectedActor ()->PhysPoints.empty ())
                    {
                        m_EditorActorMode.GetSelectedActor ()->PhysPoints.push_back ({});
                    }

                    m_EditorPhysMode.SetPhysPoly (&m_EditorActorMode.GetSelectedActor ()->PhysPoints[0]);

                    selectModeButton->Show ();
                }
            }
            else if (m_CursorMode == CursorMode::TileEditMode && !g_IsToolBoxTileSelected)
            {
                m_EditorActorMode.SetSelectedActor (nullptr);

                Rect r;

                Actor* newSelectedActor = m_EditorActorMode.GetActorUnderCursor (event.x, event.y, std::move (r));

                if (newSelectedActor != m_EditorActorMode.GetSelectedActor () || !newSelectedActor)
                {
                    if (newSelectedActor)
                    {
                        m_CursorMode = CursorMode::TileEditMode;
                        m_EditorActorMode.SetSelectedActor (newSelectedActor);
                        m_EditorActorMode.SetRotation (m_EditorActorMode.GetSelectedActor ()->Rotation);

                        if (m_EditorActorMode.GetSelectedActor ()->PhysPoints.empty ())
                        {
                            m_EditorActorMode.GetSelectedActor ()->PhysPoints.push_back ({});
                        }

                        m_EditorPhysMode.SetPhysPoly (&m_EditorActorMode.GetSelectedActor ()->PhysPoints[0]);

                        selectModeButton->Show ();
                    }
                    else
                    {
                        m_CursorMode = CursorMode::TileSelectMode;
                        m_EditorActorMode.SetSelectedActor (nullptr);

                        selectModeButton->Hide ();
                    }
                }
            }
            else if (m_CursorMode == CursorMode::EditPhysBodyMode)
            {
                m_EditorPhysMode.InsertPhysPointAtCursor (event.x, event.y);
            }
            else if (m_CursorMode == CursorMode::EditFlagPointsMode)
            {
                m_EditorFlagPointMode.InsertFlagPointAtCursor (event.x, event.y);
            }
            else if (m_CursorMode == CursorMode::EditTriggerAreaMode)
            {
                m_EditorTriggerAreaMode.InsertTriggerAreaAtCursor (event.x, event.y);
            }

            m_EditorFlagPointMode.SetFlagPoint (m_EditorFlagPointMode.GetFlagPointUnderCursor (event.x, event.y));

            m_EditorTriggerAreaMode.SetTriggerPoint (
                m_EditorTriggerAreaMode.GetTriggerPointUnderCursor (event.x, event.y));
            m_EditorTriggerAreaMode.SetTriggerArea (
                m_EditorTriggerAreaMode.GetTriggerAreaUnderCursor (event.x, event.y));

            if (m_EditorTriggerAreaMode.GetTriggerPoint () && m_EditorTriggerAreaMode.GetTriggerArea ())
            {
                m_CursorMode = CursorMode::EditTriggerAreaMode;
            }
        }

        if (event.button == 2)
        {
            bool flagPointRemoved = m_EditorFlagPointMode.RemoveFlagPointUnderCursor (event.x, event.y);
            bool triggerPointRemoved = m_EditorTriggerAreaMode.RemoveTriggerPointUnderCursor (event.x, event.y);
            bool physPointRemoved = false;

            m_EditorFlagPointMode.SetFlagPoint ("");

            if (m_CursorMode == CursorMode::EditPhysBodyMode)
            {
                physPointRemoved = m_EditorPhysMode.RemovePhysPointUnderCursor (event.x, event.y);
            }
            else if (!flagPointRemoved && !triggerPointRemoved)
            {
                m_CursorMode = CursorMode::TileSelectMode;
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::ProcessMouseButtonUp (ALLEGRO_MOUSE_EVENT& event)
    {
        m_IsMousePan = false;

        if (m_EditorPhysMode.GetPhysPoint () && m_EditorActorMode.GetSelectedActor () && event.button == 1)
        {
            m_EditorActorMode.GetSelectedActor ()->UpdatePhysPolygon ();

            if (!m_EditorPhysMode.GetPhysPoint () && m_EditorPhysMode.GetPhysPoly ()
                && !(*m_EditorPhysMode.GetPhysPoly ()).empty ())
            {
                m_EditorPhysMode.SetPhysPoint (&(*m_EditorPhysMode.GetPhysPoly ())[0]);
            }
        }

        if (m_EditorTriggerAreaMode.GetTriggerArea () && event.button == 1)
        {
            m_EditorTriggerAreaMode.GetTriggerArea ()->UpdatePolygons (
                &m_MainLoop->GetPhysicsManager ().GetTriangulator ());

            if (!m_EditorTriggerAreaMode.GetTriggerPoint () && m_EditorTriggerAreaMode.GetTriggerArea ()
                && !m_EditorTriggerAreaMode.GetTriggerArea ()->Points.empty ())
            {
                m_EditorTriggerAreaMode.SetTriggerPoint (&m_EditorTriggerAreaMode.GetTriggerArea ()->Points[0]);
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::ProcessMouseAxes (ALLEGRO_MOUSE_EVENT& event)
    {
        if (!m_EditorFlagPointMode.MoveSelectedFlagPoint () && !m_EditorTriggerAreaMode.MoveSelectedTriggerPoint ())
        {
            if (m_CursorMode == CursorMode::TileEditMode)
            {
                if (m_EditorActorMode.GetSelectedActor ())
                {
                    m_EditorActorMode.MoveSelectedActor ();
                }
            }
            else if (m_CursorMode == CursorMode::EditPhysBodyMode)
            {
                m_EditorPhysMode.MoveSelectedPhysPoint ();
            }
        }

        if (IsEditorCanvasNotCovered ())
        {
            HandleCameraMovement (event);
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::OnTilesetSelected (Gwk::Controls::Base* control)
    {
        Gwk::Controls::Label* selItem = tilesetCombo->GetSelectedItem ();

        m_EditorActorMode.ChangeAtlas (selItem->GetText ());
    }

    //--------------------------------------------------------------------------------------------------
} // namespace aga
