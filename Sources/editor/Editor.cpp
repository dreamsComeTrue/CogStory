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
#include "Screen.h"

#include <Gwork/Input/Allegro5.h>
#include <Gwork/Platform.h>
#include <Gwork/Renderers/Allegro5.h>
#include <Gwork/Skins/TexturedBase.h>

using json = nlohmann::json;

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    const char* configFileName = "editor_config.json";
    bool askNewScene = false;
    std::string menuFileName = "0_home/0_0_home.scn";

    //--------------------------------------------------------------------------------------------------
    //--------------------------------------------------------------------------------------------------

    Editor::Editor (MainLoop* mainLoop)
      : m_EditorTileMode (this)
      , m_EditorPhysMode (this)
      , m_EditorFlagPointMode (this)
      , m_EditorTriggerAreaMode (this)
      , m_EditorSpeechMode (this)
      , m_EditorActorMode (this)
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

    Gwk::Renderer::Allegro* guiRenderer;
    Gwk::Controls::Canvas* mainCanvas;
    Gwk::Skin::TexturedBase* guiSkin;

    Gwk::Input::Allegro guiInput;

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

    Gwk::Controls::Button* tileModeButton;
    Gwk::Controls::Button* newPolyButton;

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

        Gwk::Platform::SetPlatformWindow (m_MainLoop->GetScreen ()->GetDisplay ());

        Gwk::Platform::RelativeToExecutablePaths paths ("../Data/");
        Gwk::Renderer::AllegroResourceLoader loader (paths);

        guiRenderer = new Gwk::Renderer::Allegro (loader);

        guiSkin = new Gwk::Skin::TexturedBase (guiRenderer);
        guiSkin->SetRender (guiRenderer);
        guiSkin->Init ("UISkin.png");

        // The fonts work differently in Allegro - it can't use
        // system fonts. So force the skin to use a local one.
        guiSkin->SetDefaultFont ("fonts/OpenSans.ttf", 12);

        // Create a Canvas (it's root, on which all other Gwork panels are created)
        const Point screenSize = m_MainLoop->GetScreen ()->GetWindowSize ();

        mainCanvas = new Gwk::Controls::Canvas (guiSkin);
        mainCanvas->SetSize (screenSize.Width, screenSize.Height);
        mainCanvas->SetDrawBackground (false);

        guiInput.Initialize (mainCanvas);

        //  Add GUI Controls
        sceneNameLabel = new Gwk::Controls::Label (mainCanvas);
        sceneNameLabel->SetTextColor (Gwk::Color (0, 255, 0, 255));
        sceneNameLabel->SetPos (20, 10);
        sceneNameLabel->SetText ("SCENE: " + m_MainLoop->GetSceneManager ().GetActiveScene ()->GetName ());
        sceneNameLabel->SizeToContents ();

        newSceneButton = new Gwk::Controls::Button (mainCanvas);
        newSceneButton->SetText ("NEW SCENE");
        newSceneButton->SetPos (20, 30);
        newSceneButton->onPress.Add (this, &Editor::OnNewScene);

        openSceneButton = new Gwk::Controls::Button (mainCanvas);
        openSceneButton->SetText ("OPEN SCENE");
        openSceneButton->SetPos (20, newSceneButton->Bottom () + 5);
        openSceneButton->onPress.Add (this, &Editor::OnOpenScene);

        saveSceneButton = new Gwk::Controls::Button (mainCanvas);
        saveSceneButton->SetText ("SAVE SCENE");
        saveSceneButton->SetPos (20, openSceneButton->Bottom () + 5);
        saveSceneButton->onPress.Add (this, &Editor::OnSaveScene);

        //  Diaglos & windows
        {
            m_OpenSceneWindow = new EditorOpenSceneWindow (this, mainCanvas, "0_home/0_0_home.scn");
            m_SaveSceneWindow = new EditorSaveSceneWindow (this, mainCanvas, "0_home/0_0_home.scn");
            m_FlagPointWindow = new EditorFlagPointWindow (this, mainCanvas);
            m_TriggerAreaWindow = new EditorTriggerAreaWindow (this, mainCanvas);
            m_SpeechWindow = new EditorSpeechWindow (this, mainCanvas);
            m_ActorWindow = new EditorActorWindow (this, mainCanvas);
            m_InfoWindow = new EditorInfoWindow (this, mainCanvas);
            m_QuestionWindow = new EditorQuestionWindow (this, mainCanvas);
        }

        resetMoveButton = new Gwk::Controls::Button (mainCanvas);
        resetMoveButton->SetText ("RESET MOVE");
        resetMoveButton->SetPos (20, saveSceneButton->Bottom () + 40);
        resetMoveButton->onPress.Add (this, &Editor::OnResetTranslate);

        resetScaleButton = new Gwk::Controls::Button (mainCanvas);
        resetScaleButton->SetText ("RESET SCALE");
        resetScaleButton->SetPos (20, resetMoveButton->Bottom () + 5);
        resetScaleButton->onPress.Add (this, &Editor::OnResetScale);

        showGridButton = new Gwk::Controls::Button (mainCanvas);
        showGridButton->SetText ("SHOW GRID");
        showGridButton->SetPos (20, resetScaleButton->Bottom () + 5);
        showGridButton->onPress.Add (this, &Editor::OnShowGrid);

        increaseGridButton = new Gwk::Controls::Button (mainCanvas);
        increaseGridButton->SetText ("+++");
        increaseGridButton->SetWidth (45);
        increaseGridButton->SetPos (20, showGridButton->Bottom () + 5);
        increaseGridButton->onPress.Add (this, &Editor::OnGridIncrease);

        decreaseGridButton = new Gwk::Controls::Button (mainCanvas);
        decreaseGridButton->SetText ("---");
        decreaseGridButton->SetWidth (45);
        decreaseGridButton->SetPos (increaseGridButton->GetPos ().x + increaseGridButton->GetSize ().x + 10,
                                    showGridButton->Bottom () + 5);
        decreaseGridButton->onPress.Add (this, &Editor::OnGridDecrease);

        flagPointButton = new Gwk::Controls::Button (mainCanvas);
        flagPointButton->SetText ("FLAG POINT");
        flagPointButton->SetPos (20, decreaseGridButton->Bottom () + 40);
        flagPointButton->onPress.Add (this, &Editor::OnFlagPoint);

        triggerAreaButton = new Gwk::Controls::Button (mainCanvas);
        triggerAreaButton->SetText ("TRIGGER AREA");
        triggerAreaButton->SetPos (20, flagPointButton->Bottom () + 5);
        triggerAreaButton->onPress.Add (this, &Editor::OnTriggerArea);

        tileModeButton = new Gwk::Controls::Button (mainCanvas);
        tileModeButton->SetText (m_CursorMode != CursorMode::EditPhysBodyMode ? "TILE MODE" : "PHYS MODE");
        tileModeButton->SetPos (20, triggerAreaButton->Bottom () + 10);
        tileModeButton->onPress.Add (this, &Editor::OnTileMode);
        tileModeButton->Hide ();

        newPolyButton = new Gwk::Controls::Button (mainCanvas);
        newPolyButton->SetText ("NEW POLY");
        newPolyButton->SetPos (20, tileModeButton->Bottom () + 5);
        newPolyButton->onPress.Add (this, &Editor::OnNewPoly);
        newPolyButton->Hide ();

        speechButton = new Gwk::Controls::Button (mainCanvas);
        speechButton->SetText ("SPEECH");
        speechButton->SetPos (20, newPolyButton->Bottom () + 10);
        speechButton->onPress.Add (this, &Editor::OnSpeech);

        actorButton = new Gwk::Controls::Button (mainCanvas);
        actorButton->SetText ("ACTOR");
        actorButton->SetPos (20, speechButton->Bottom () + 5);
        actorButton->onPress.Add (this, &Editor::OnActor);

        playButton = new Gwk::Controls::Button (mainCanvas);
        playButton->SetText ("PLAY");
        playButton->SetPos (20, actorButton->Bottom () + 20);
        playButton->onPress.Add (this, &Editor::OnPlay);

        tilesetCombo = new Gwk::Controls::ComboBox (mainCanvas);
        tilesetCombo->SetWidth (90);
        tilesetCombo->SetKeyboardInputEnabled (false);
        tilesetCombo->onSelection.Add (this, &Editor::OnTilesetSelected);

        std::map<std::string, Atlas*>& atlases = m_MainLoop->GetSceneManager ().GetAtlasManager ()->GetAtlases ();

        for (const auto& atlas : atlases)
        {
            tilesetCombo->AddItem (atlas.first, atlas.second->GetPath ());
        }

        leftPrevTileButton = new Gwk::Controls::Button (mainCanvas);
        leftPrevTileButton->SetWidth (30);
        leftPrevTileButton->SetText ("<<");
        leftPrevTileButton->onPress.Add (this, &Editor::OnPlay);

        leftNextTileButton = new Gwk::Controls::Button (mainCanvas);
        leftNextTileButton->SetWidth (30);
        leftNextTileButton->SetText (">>");
        leftNextTileButton->onPress.Add (this, &Editor::OnPlay);

        rightPrevTileButton = new Gwk::Controls::Button (mainCanvas);
        rightPrevTileButton->SetWidth (30);
        rightPrevTileButton->SetText ("<<");
        rightPrevTileButton->onPress.Add (this, &Editor::OnPlay);

        rightNextTileButton = new Gwk::Controls::Button (mainCanvas);
        rightNextTileButton->SetWidth (30);
        rightNextTileButton->SetText (">>");
        rightNextTileButton->onPress.Add (this, &Editor::OnPlay);

        playButton = new Gwk::Controls::Button (mainCanvas);
        playButton->SetText ("PLAY");
        playButton->SetPos (20, actorButton->Bottom () + 20);
        playButton->onPress.Add (this, &Editor::OnPlay);

        avgFPSLabel = new Gwk::Controls::Label (mainCanvas);
        avgFPSLabel->SetTextColor (Gwk::Color (0, 255, 0, 255));

        fpsLabel = new Gwk::Controls::Label (mainCanvas);
        fpsLabel->SetTextColor (Gwk::Color (0, 255, 0, 255));

        idLabel = new Gwk::Controls::Label (mainCanvas);
        idLabel->SetTextColor (Gwk::Color (0, 255, 0, 255));

        xPosLabel = new Gwk::Controls::Label (mainCanvas);
        xPosLabel->SetTextColor (Gwk::Color (0, 255, 0, 255));

        yPosLabel = new Gwk::Controls::Label (mainCanvas);
        yPosLabel->SetTextColor (Gwk::Color (0, 255, 0, 255));

        widthLabel = new Gwk::Controls::Label (mainCanvas);
        widthLabel->SetTextColor (Gwk::Color (0, 255, 0, 255));

        heightLabel = new Gwk::Controls::Label (mainCanvas);
        heightLabel->SetTextColor (Gwk::Color (0, 255, 0, 255));

        angleLabel = new Gwk::Controls::Label (mainCanvas);
        angleLabel->SetTextColor (Gwk::Color (0, 255, 0, 255));

        zOrderLabel = new Gwk::Controls::Label (mainCanvas);
        zOrderLabel->SetTextColor (Gwk::Color (0, 255, 0, 255));

        scaleLabel = new Gwk::Controls::Label (mainCanvas);
        scaleLabel->SetTextColor (Gwk::Color (0, 255, 0, 255));

        snapLabel = new Gwk::Controls::Label (mainCanvas);
        snapLabel->SetTextColor (Gwk::Color (0, 255, 0, 255));

        gridLabel = new Gwk::Controls::Label (mainCanvas);
        gridLabel->SetTextColor (Gwk::Color (0, 255, 0, 255));

        scriptsBox = new Gwk::Controls::ListBox (mainCanvas);
        scriptsBox->SetKeyboardInputEnabled (true);
        //        ctrl->onRowSelected.Add(this, &ThisClass::RowSelected);

        UpdateScriptsBox ();

        scriptReloadButton = new Gwk::Controls::Button (mainCanvas);
        scriptReloadButton->SetText ("RELOAD");
        scriptReloadButton->onPress.Add (this, &Editor::OnReloadScript);

        m_EditorTileMode.InitializeUI ();
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
        SAFE_DELETE (m_QuestionWindow);

        delete mainCanvas;
        // delete guiSkin;
        delete guiRenderer;

        return Lifecycle::Destroy ();
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::LoadConfig ()
    {
        try
        {
            std::ifstream file (configFileName);
            json j;
            file >> j;
            file.close ();

            m_IsSnapToGrid = j["show_grid"];
            m_MainLoop->GetSceneManager ().GetActiveScene ()->SetDrawPhysData (j["show_physics"]);
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
            j["show_physics"] = m_MainLoop->GetSceneManager ().GetActiveScene ()->IsDrawPhysData ();

            // write prettified JSON to another file
            std::ofstream out (configFileName);
            out << std::setw (4) << j.dump (4, ' ') << "\n";
        }
        catch (const std::exception&)
        {
        }
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
    bool g_IsToolBoxTileSelected = false;

    void Editor::ProcessEvent (ALLEGRO_EVENT* event, float)
    {
        if (m_EditorTileMode.m_IsDrawTiles && guiInput.ProcessMessage (*event))
        {
            return;
        }

        g_IsToolBoxTileSelected = false;
        if (event->type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN)
        {
            if (event->mouse.button == 1 && m_EditorTileMode.m_IsDrawTiles)
            {
                g_IsToolBoxTileSelected = m_EditorTileMode.ChooseTile (event->mouse.x, event->mouse.y);
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
                case ALLEGRO_KEY_SPACE:
                {
                    m_EditorTileMode.m_IsDrawTiles = !m_EditorTileMode.m_IsDrawTiles;
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
            ProcessMouseButtonDown (event->mouse);
        }
        else if (event->type == ALLEGRO_EVENT_MOUSE_BUTTON_UP)
        {
            ProcessMouseButtonUp (event->mouse);
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
              b.GetPos ().X, b.GetPos ().Y, b.GetBottomRight ().X, b.GetBottomRight ().Y, COLOR_RED, 2);
        }

        if (m_CursorMode == CursorMode::TileSelectMode)
        {
            Rect r;
            m_EditorTileMode.m_TileUnderCursor = m_EditorTileMode.GetTileUnderCursor (state.x, state.y, std::move (r));

            if (m_EditorTileMode.m_TileUnderCursor)
            {
                al_draw_rectangle (
                  r.GetPos ().X, r.GetPos ().Y, r.GetBottomRight ().X, r.GetBottomRight ().Y, COLOR_YELLOW, 2);
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

            std::string txt = "[" + ToString (finalX) + ", " + ToString (finalY) + "]";

            m_MainLoop->GetSceneManager ().GetMainLoop ()->GetScreen ()->GetFont ().DrawText (
              FONT_NAME_MAIN_SMALL, al_map_rgb (0, 255, 0), 120, 20, txt, ALLEGRO_ALIGN_LEFT);
        }

        finalX -= translate.X;
        finalY -= translate.Y;

        return { finalX, finalY };
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::OnNewScene (Gwk::Controls::Base*)
    {
        std::function<void(void)> YesFunc = [&] {
            m_MainLoop->GetSceneManager ().GetActiveScene ()->Reset ();

            ResetSettings ();
            UpdateScriptsBox ();

            sceneNameLabel->SetText (std::string ("SCENE: -"));
            sceneNameLabel->SizeToContents ();
        };

        m_QuestionWindow->Show ("Are you sure clearing current scene?", YesFunc, nullptr);
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
            Scene* scene = Scene::LoadScene (&m_MainLoop->GetSceneManager (), path);

            if (scene)
            {
                m_MainLoop->GetSceneManager ().RemoveScene (m_MainLoop->GetSceneManager ().GetActiveScene ());
                m_MainLoop->GetSceneManager ().SetActiveScene (scene);

                ResetSettings ();

                sceneNameLabel->SetText (
                  std::string ("SCENE: " + m_MainLoop->GetSceneManager ().GetActiveScene ()->GetName ()));
                sceneNameLabel->SizeToContents ();
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::SaveScene (const std::string& filePath)
    {
        std::string path = GetDataPath () + "scenes/" + filePath;
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

    void Editor::OnShowGrid ()
    {
        m_IsSnapToGrid = !m_IsSnapToGrid;
        showGridButton->SetText (m_IsSnapToGrid ? "HIDE GRID" : "SHOW GRID");
    }

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

    void Editor::OnFlagPoint () { m_FlagPointWindow->Show (); }

    //--------------------------------------------------------------------------------------------------

    void Editor::OnTriggerArea () { m_TriggerAreaWindow->Show (); }

    //--------------------------------------------------------------------------------------------------

    void Editor::OnTileMode ()
    {
        if (m_CursorMode != CursorMode::EditPhysBodyMode)
        {
            m_CursorMode = CursorMode::EditPhysBodyMode;

            newPolyButton->Show ();
        }
        else
        {
            m_CursorMode = CursorMode::TileSelectMode;

            newPolyButton->Hide ();
        }

        tileModeButton->SetText (m_CursorMode != CursorMode::EditPhysBodyMode ? "TILE MODE" : "PHYS MODE");
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::OnNewPoly ()
    {
        m_EditorPhysMode.m_PhysPoly = nullptr;
        m_EditorPhysMode.m_PhysPoint = nullptr;
        m_EditorTileMode.m_SelectedTile->PhysPoints.push_back ({});
        m_EditorPhysMode.m_PhysPoly =
          &m_EditorTileMode.m_SelectedTile->PhysPoints[m_EditorTileMode.m_SelectedTile->PhysPoints.size () - 1];
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::OnSpeech ()
    {
        m_SpeechWindow->UpdateSpeechesTree ();
        m_SpeechWindow->Show ();
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::OnActor () { m_ActorWindow->Show (); }

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
                std::experimental::optional<ScriptMetaData> metaScript =
                  m_MainLoop->GetSceneManager ().GetActiveScene ()->GetScriptByName (name);

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

    void Editor::RenderUI ()
    {
        Point translate = m_MainLoop->GetSceneManager ().GetCamera ().GetTranslate ();
        Point scale = m_MainLoop->GetSceneManager ().GetCamera ().GetScale ();

        avgFPSLabel->SetText (Gwk::Utility::Format ("    AVG: %.2f ms", 1000.0f / m_MainLoop->GetScreen ()->GetFPS ()));
        avgFPSLabel->SizeToContents ();

        fpsLabel->SetText (Gwk::Utility::Format ("    FPS: %.1f", m_MainLoop->GetScreen ()->GetFPS ()));
        fpsLabel->SizeToContents ();

        idLabel->SetText (std::string (
          "       ID: " + (m_EditorTileMode.m_SelectedTile ? ToString (m_EditorTileMode.m_SelectedTile->ID) : "-")));
        idLabel->SizeToContents ();

        xPosLabel->SetText (std::string ("        X: " + ToString (translate.X * (1 / scale.X))));
        xPosLabel->SizeToContents ();

        yPosLabel->SetText (std::string ("        Y: " + ToString (translate.Y * (1 / scale.Y))));
        yPosLabel->SizeToContents ();

        widthLabel->SetText (
          std::string ("       W: " + ToString (m_EditorTileMode.m_SelectedAtlasRegion.Bounds.GetSize ().Width)));
        widthLabel->SizeToContents ();

        heightLabel->SetText (
          std::string ("        H: " + ToString (m_EditorTileMode.m_SelectedAtlasRegion.Bounds.GetSize ().Height)));
        heightLabel->SizeToContents ();

        angleLabel->SetText (
          std::string ("        A: " +
                       (m_EditorTileMode.m_SelectedTile ? ToString (m_EditorTileMode.m_SelectedTile->Rotation) : "-")));
        angleLabel->SizeToContents ();

        zOrderLabel->SetText (std::string (
          "ZORD: " + (m_EditorTileMode.m_SelectedTile ? ToString (m_EditorTileMode.m_SelectedTile->ZOrder) : "-")));
        zOrderLabel->SizeToContents ();

        scaleLabel->SetText (std::string ("         S: " + ToString (scale.X)));
        scaleLabel->SizeToContents ();

        snapLabel->SetText (std::string ("SNAP: " + ToString (m_IsSnapToGrid ? "YES" : "NO")));
        snapLabel->SizeToContents ();

        gridLabel->SetText (std::string (" GRID: " + ToString (m_BaseGridSize)));
        gridLabel->SizeToContents ();

        mainCanvas->RenderCanvas ();
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::SetDrawUITiles (bool draw) { m_EditorTileMode.m_IsDrawTiles = draw; }

    //--------------------------------------------------------------------------------------------------

    void Editor::ScreenResize ()
    {
        const Point screenSize = m_MainLoop->GetScreen ()->GetWindowSize ();
        mainCanvas->SetSize (screenSize.Width, screenSize.Height);

        avgFPSLabel->SetPos (mainCanvas->Width () - 120.0f, 10);
        fpsLabel->SetPos (mainCanvas->Width () - 120.0f, avgFPSLabel->Bottom () + 5);
        idLabel->SetPos (mainCanvas->Width () - 120.0f, fpsLabel->Bottom () + 10);
        xPosLabel->SetPos (mainCanvas->Width () - 120.0f, idLabel->Bottom () + 5);
        yPosLabel->SetPos (mainCanvas->Width () - 120.0f, xPosLabel->Bottom () + 5);
        widthLabel->SetPos (mainCanvas->Width () - 120.0f, yPosLabel->Bottom () + 5);
        heightLabel->SetPos (mainCanvas->Width () - 120.0f, widthLabel->Bottom () + 5);
        angleLabel->SetPos (mainCanvas->Width () - 120.0f, heightLabel->Bottom () + 5);
        zOrderLabel->SetPos (mainCanvas->Width () - 120.0f, angleLabel->Bottom () + 5);
        scaleLabel->SetPos (mainCanvas->Width () - 120.0f, zOrderLabel->Bottom () + 5);
        snapLabel->SetPos (mainCanvas->Width () - 120.0f, scaleLabel->Bottom () + 5);
        gridLabel->SetPos (mainCanvas->Width () - 120.0f, snapLabel->Bottom () + 5);
        scriptsBox->SetBounds (mainCanvas->Width () - 150.0f, gridLabel->Bottom () + 10, 140, 100);
        scriptReloadButton->SetPos (mainCanvas->Width () - 130.0f, scriptsBox->Bottom () + 5);

        float beginning = screenSize.Width * 0.5 - (TILES_COUNT - 1) * 0.5 * TILE_SIZE - TILE_SIZE * 0.5;

        tilesetCombo->SetPos (beginning - 140, mainCanvas->Bottom () - 35);
        leftPrevTileButton->SetPos (beginning - 35, mainCanvas->Bottom () - TILE_SIZE + 5);
        leftNextTileButton->SetPos (beginning - 35, leftPrevTileButton->Bottom () + 2);
        rightPrevTileButton->SetPos (beginning + TILES_COUNT * TILE_SIZE + 5, mainCanvas->Bottom () - TILE_SIZE + 5);
        rightNextTileButton->SetPos (beginning + TILES_COUNT * TILE_SIZE + 5, rightPrevTileButton->Bottom () + 2);
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::ProcessMouseButtonDown (ALLEGRO_MOUSE_EVENT& event)
    {
        m_IsMousePan = event.button == 3;

        if (event.button == 1)
        {
            if (m_CursorMode == CursorMode::TileSelectMode && m_EditorFlagPointMode.m_FlagPoint == "")
            {
                Rect r;
                m_EditorTileMode.m_SelectedTile = m_EditorTileMode.GetTileUnderCursor (event.x, event.y, std::move (r));

                if (m_EditorTileMode.m_SelectedTile)
                {
                    tileModeButton->Show ();

                    m_CursorMode = CursorMode::TileEditMode;
                    m_EditorTileMode.m_Rotation = m_EditorTileMode.m_SelectedTile->Rotation;
                }
            }
            else if (m_CursorMode == CursorMode::TileEditMode && !g_IsToolBoxTileSelected)
            {
                Rect r;
                Tile* newSelectedTile = m_EditorTileMode.GetTileUnderCursor (event.x, event.y, std::move (r));

                if (newSelectedTile != m_EditorTileMode.m_SelectedTile || !newSelectedTile)
                {
                    if (newSelectedTile)
                    {
                        m_EditorTileMode.m_SelectedTile = newSelectedTile;
                        m_CursorMode = CursorMode::TileEditMode;
                        m_EditorTileMode.m_Rotation = m_EditorTileMode.m_SelectedTile->Rotation;

                        tileModeButton->Show ();
                    }
                    else
                    {
                        m_CursorMode = CursorMode::TileSelectMode;
                        m_EditorTileMode.m_SelectedTile = nullptr;

                        tileModeButton->Hide ();
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

            m_EditorFlagPointMode.m_FlagPoint = m_EditorFlagPointMode.GetFlagPointUnderCursor (event.x, event.y);

            m_EditorTriggerAreaMode.m_TriggerPoint =
              m_EditorTriggerAreaMode.GetTriggerPointUnderCursor (event.x, event.y);
            m_EditorTriggerAreaMode.m_TriggerArea =
              m_EditorTriggerAreaMode.GetTriggerAreaUnderCursor (event.x, event.y);

            if (m_EditorTriggerAreaMode.m_TriggerPoint && m_EditorTriggerAreaMode.m_TriggerArea)
            {
                m_CursorMode = CursorMode::EditTriggerAreaMode;
            }
        }

        if (event.button == 2)
        {
            bool flagPointRemoved = m_EditorFlagPointMode.RemoveFlagPointUnderCursor (event.x, event.y);
            bool triggerPointRemoved = m_EditorTriggerAreaMode.RemoveTriggerPointUnderCursor (event.x, event.y);
            bool physPointRemoved = false;

            m_EditorFlagPointMode.m_FlagPoint = "";

            if (m_CursorMode == CursorMode::EditPhysBodyMode)
            {
                physPointRemoved = m_EditorPhysMode.RemovePhysPointUnderCursor (event.x, event.y);
            }

            if (!flagPointRemoved && !triggerPointRemoved && !physPointRemoved)
            {
                m_CursorMode = CursorMode::TileSelectMode;
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::ProcessMouseButtonUp (ALLEGRO_MOUSE_EVENT& event)
    {
        m_IsMousePan = false;

        if (m_EditorPhysMode.m_PhysPoint && m_EditorTileMode.m_SelectedTile && event.button == 1)
        {
            m_EditorTileMode.m_SelectedTile->UpdatePhysPolygon ();

            if (!m_EditorPhysMode.m_PhysPoint && m_EditorPhysMode.m_PhysPoly &&
                !(*m_EditorPhysMode.m_PhysPoly).empty ())
            {
                m_EditorPhysMode.m_PhysPoint = &(*m_EditorPhysMode.m_PhysPoly)[0];
            }
        }

        if (m_EditorTriggerAreaMode.m_TriggerArea && event.button == 1)
        {
            m_EditorTriggerAreaMode.m_TriggerArea->UpdatePolygons (
              &m_MainLoop->GetPhysicsManager ().GetTriangulator ());

            if (!m_EditorTriggerAreaMode.m_TriggerPoint && m_EditorTriggerAreaMode.m_TriggerArea &&
                !m_EditorTriggerAreaMode.m_TriggerArea->Points.empty ())
            {
                m_EditorTriggerAreaMode.m_TriggerPoint = &m_EditorTriggerAreaMode.m_TriggerArea->Points[0];
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
                m_EditorTileMode.MoveSelectedTile ();
            }
            else if (m_CursorMode == CursorMode::EditPhysBodyMode)
            {
                m_EditorPhysMode.MoveSelectedPhysPoint ();
            }
        }

        HandleCameraMovement (event);
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::OnTilesetSelected (Gwk::Controls::Base* control)
    {
        Gwk::Controls::Label* selItem = tilesetCombo->GetSelectedItem ();

        m_EditorTileMode.ChangeAtlas (selItem->GetText ());
    }

    //--------------------------------------------------------------------------------------------------
}
