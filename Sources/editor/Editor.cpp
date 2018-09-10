// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "Editor.h"
#include "AtlasManager.h"
#include "EditorActorWindow.h"
#include "EditorComponentWindow.h"
#include "EditorFlagPointWindow.h"
#include "EditorOpenSceneWindow.h"
#include "EditorSaveSceneWindow.h"
#include "EditorSceneWindow.h"
#include "EditorScriptWindow.h"
#include "EditorSpeechWindow.h"
#include "EditorTriggerAreaWindow.h"
#include "EditorWindows.h"
#include "MainLoop.h"
#include "Resources.h"
#include "Screen.h"
#include "actors/TileActor.h"
#include "SceneLoader.h"

using json = nlohmann::json;

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    std::string configFileName = "editor_config.json";

    const float DOUBLE_CLICK_SPEED = 300;

    //--------------------------------------------------------------------------------------------------
    //--------------------------------------------------------------------------------------------------

    Editor::Editor (MainLoop *mainLoop)
            : m_EditorPhysMode (this), m_EditorFlagPointMode (this), m_EditorTriggerAreaMode (this),
              m_EditorSpeechMode (this), m_EditorActorMode (this), m_MainLoop (mainLoop), m_IsSnapToGrid (true),
              m_IsMousePan (false), m_BaseGridSize (16.0f), m_GridSize (16.0f),
              m_CursorMode (CursorMode::ActorSelectMode), m_LastTimeClicked (0.0f), m_IsRectSelection (false)
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

    Gwk::Controls::Label *sceneNameLabel;
    Gwk::Controls::Button *newSceneButton;
    Gwk::Controls::Button *openSceneButton;
    Gwk::Controls::Button *saveSceneButton;

    Gwk::Controls::Button *resetMoveButton;
    Gwk::Controls::Button *resetScaleButton;
    Gwk::Controls::Button *showGridButton;
    Gwk::Controls::Button *increaseGridButton;
    Gwk::Controls::Button *decreaseGridButton;

    Gwk::Controls::Button *sceneButton;
    Gwk::Controls::Button *flagPointButton;
    Gwk::Controls::Button *triggerAreaButton;

    Gwk::Controls::Button *selectModeButton;
    Gwk::Controls::Button *newPolyButton;
    Gwk::Controls::Button *saveBodyButton;
    Gwk::Controls::Button *removeBodyButton;

    Gwk::Controls::Button *speechButton;
    Gwk::Controls::Button *actorButton;

    Gwk::Controls::Button *playButton;
    Gwk::Controls::ComboBox *tilesetCombo;
    Gwk::Controls::Button *leftPrevTileButton;
    Gwk::Controls::Button *leftNextTileButton;
    Gwk::Controls::Button *rightPrevTileButton;
    Gwk::Controls::Button *rightNextTileButton;
    Gwk::Controls::Button *spriteSheetButton;

    Gwk::Controls::Label *avgFPSLabel;
    Gwk::Controls::Label *fpsLabel;
    Gwk::Controls::Label *idLabel;
    Gwk::Controls::Label *xPosLabel;
    Gwk::Controls::Label *yPosLabel;
    Gwk::Controls::Label *widthLabel;
    Gwk::Controls::Label *heightLabel;
    Gwk::Controls::Label *angleLabel;
    Gwk::Controls::Label *zOrderLabel;
    Gwk::Controls::Label *scaleLabel;
    Gwk::Controls::Label *snapLabel;
    Gwk::Controls::Label *gridLabel;
    Gwk::Controls::Label *tilesLabel;
    Gwk::Controls::Label *actorsLabel;
    Gwk::Controls::Label *cursorLabel;

    bool Editor::Initialize ()
    {
        Lifecycle::Initialize ();

        al_init_native_dialog_addon ();

        Gwk::Platform::SetPlatformWindow (m_MainLoop->GetScreen ()->GetDisplay ());

        Gwk::Platform::RelativeToExecutablePaths paths ("../../Data/");
        m_ResourceLoader = new Gwk::Renderer::AllegroResourceLoader (paths);

        m_GUIRenderer = new Gwk::Renderer::Allegro (*m_ResourceLoader);

        m_GuiSkin = new Gwk::Skin::TexturedBase (m_GUIRenderer);
        m_GuiSkin->Init ("gfx/" + GetResource (GFX_DEFAULT_SKIN).Name);

        // The fonts work differently in Allegro - it can't use
        // system fonts. So force the skin to use a local one.
        m_GuiSkin->SetDefaultFont ("fonts/" + GetResource (FONT_EDITOR).Name, 12);

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
            m_EditorSceneWindow = new EditorSceneWindow (this, m_MainCanvas);
            m_OpenSceneWindow = new EditorOpenSceneWindow (this, m_MainCanvas, "");
            m_SaveSceneWindow = new EditorSaveSceneWindow (this, m_MainCanvas, "");
            m_FlagPointWindow = new EditorFlagPointWindow (this, m_MainCanvas);
            m_TriggerAreaWindow = new EditorTriggerAreaWindow (this, m_MainCanvas);
            m_SpeechWindow = new EditorSpeechWindow (this, m_MainCanvas);
            m_ActorWindow = new EditorActorWindow (this, m_MainCanvas);
            m_InfoWindow = new EditorInfoWindow (this, m_MainCanvas);
            m_QuestionWindow = new EditorQuestionWindow (this, m_MainCanvas);
            m_InputWindow = new EditorInputWindow (this, m_MainCanvas);
            m_ScriptWindow = new EditorScriptWindow (this, m_MainCanvas);
            m_ComponentWindow = new EditorComponentWindow (this, m_MainCanvas);
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
        decreaseGridButton->SetPos (
                increaseGridButton->GetPos ().x + increaseGridButton->GetSize ().x + 10, showGridButton->Bottom () + 5);
        decreaseGridButton->onPress.Add (this, &Editor::OnGridDecrease);

        sceneButton = new Gwk::Controls::Button (m_MainCanvas);
        sceneButton->SetText ("SCENE EDIT");
        sceneButton->SetPos (20, decreaseGridButton->Bottom () + 40);
        sceneButton->onPress.Add (this, &Editor::OnSceneEdit);

        flagPointButton = new Gwk::Controls::Button (m_MainCanvas);
        flagPointButton->SetText ("FLAG POINT");
        flagPointButton->SetPos (20, sceneButton->Bottom () + 5);
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
        saveBodyButton->SetText ("OUTLINE BODY");
        saveBodyButton->SetPos (20, selectModeButton->Bottom () + 5);
        saveBodyButton->onPress.Add (this, &Editor::OutlineBody);
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

        actorButton = new Gwk::Controls::Button (m_MainCanvas);
        actorButton->SetText ("ACTOR [F2]");
        actorButton->SetPos (20, selectModeButton->Bottom () + 5);
        actorButton->onPress.Add (this, &Editor::OnActorSelected);

        speechButton = new Gwk::Controls::Button (m_MainCanvas);
        speechButton->SetText ("SPEECH [F3]");
        speechButton->SetPos (20, actorButton->Bottom () + 5);
        speechButton->onPress.Add (this, &Editor::OnSpeech);

        playButton = new Gwk::Controls::Button (m_MainCanvas);
        playButton->SetText ("PLAY [F1]");
        playButton->SetPos (20, newPolyButton->Bottom () + 20);
        playButton->onPress.Add (this, &Editor::OnPlay);

        tilesetCombo = new Gwk::Controls::ComboBox (m_MainCanvas);
        tilesetCombo->SetWidth (90);
        tilesetCombo->SetKeyboardInputEnabled (false);

        std::map<std::string, Atlas *> &atlases = m_MainLoop->GetAtlasManager ().GetAtlases ();

        for (const auto &atlas : atlases)
        {
            tilesetCombo->AddItem (atlas.first, atlas.second->GetPath ());
        }

        tilesetCombo->onSelection.Add (this, &Editor::OnTilesetSelected);

        m_EditorActorMode.ChangeAtlas ((*atlases.begin ()).first);

        leftNextTileButton = new Gwk::Controls::Button (m_MainCanvas);
        leftNextTileButton->SetWidth (30);
        leftNextTileButton->SetText (">");
        leftNextTileButton->onPress.Add (this, &Editor::OnScrollNextTiles);

        leftPrevTileButton = new Gwk::Controls::Button (m_MainCanvas);
        leftPrevTileButton->SetWidth (30);
        leftPrevTileButton->SetText ("<");
        leftPrevTileButton->onPress.Add (this, &Editor::OnScrollPrevTiles);

        rightNextTileButton = new Gwk::Controls::Button (m_MainCanvas);
        rightNextTileButton->SetWidth (30);
        rightNextTileButton->SetText (">>");
        rightNextTileButton->onPress.Add (this, &Editor::OnBigScrollNextTiles);

        rightPrevTileButton = new Gwk::Controls::Button (m_MainCanvas);
        rightPrevTileButton->SetWidth (30);
        rightPrevTileButton->SetText ("<<");
        rightPrevTileButton->onPress.Add (this, &Editor::OnBigScrollPrevTiles);

        spriteSheetButton = new Gwk::Controls::Button (m_MainCanvas);
        spriteSheetButton->SetWidth (50);
        spriteSheetButton->SetHeight (42);
        spriteSheetButton->SetText ("# [`]");
        spriteSheetButton->onPress.Add (this, &Editor::OnSpriteSheetEdit);

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

        tilesLabel = new Gwk::Controls::Label (m_MainCanvas);
        tilesLabel->SetTextColor (Gwk::Color (0, 255, 0, 255));

        actorsLabel = new Gwk::Controls::Label (m_MainCanvas);
        actorsLabel->SetTextColor (Gwk::Color (0, 255, 0, 255));

        cursorLabel = new Gwk::Controls::Label (m_MainCanvas);
        cursorLabel->SetTextColor (Gwk::Color (0, 255, 0, 255));

        LoadConfig ();
        ScreenResize ();

        al_identity_transform (&m_NewTransform);

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
        SAFE_DELETE (m_InputWindow);
        SAFE_DELETE (m_EditorSceneWindow);
        SAFE_DELETE (m_ScriptWindow);
        SAFE_DELETE (m_ComponentWindow);

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
        catch (const std::exception &)
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
        catch (const std::exception &)
        {
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::BeforeEnter ()
    {
        OnResetScale ();
        OnResetTranslate ();
        SetDrawUITiles (true);

        std::string fileName = m_MainLoop->GetSceneManager ().GetActiveScene ()->GetPath ();

        m_LastScenePath = fileName;
        m_OpenSceneWindow->SetFileName (fileName);
        m_SaveSceneWindow->SetFileName (fileName);
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
        return (!m_EditorSceneWindow->GetSceneWindow ()->Visible () && !m_SpeechWindow->GetSceneWindow ()->Visible ()
                && !m_TriggerAreaWindow->GetSceneWindow ()->Visible () &&
                !m_FlagPointWindow->GetSceneWindow ()->Visible ()
                && !m_ActorWindow->GetSceneWindow ()->Visible ());
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

    void Editor::TryToCloseWindows ()
    {
        if (m_OpenSceneWindow->GetSceneWindow ()->Visible ())
        {
            m_OpenSceneWindow->GetSceneWindow ()->CloseButtonPressed ();
        }

        if (m_SaveSceneWindow->GetSceneWindow ()->Visible ())
        {
            m_SaveSceneWindow->GetSceneWindow ()->CloseButtonPressed ();
        }

        if (m_EditorSceneWindow->GetSceneWindow ()->Visible ())
        {
            m_EditorSceneWindow->GetSceneWindow ()->CloseButtonPressed ();
        }

        if (m_ActorWindow->GetSceneWindow ()->Visible ())
        {
            m_ActorWindow->GetSceneWindow ()->CloseButtonPressed ();
        }

        if (m_SpeechWindow->GetSceneWindow ()->Visible ())
        {
            m_SpeechWindow->GetSceneWindow ()->CloseButtonPressed ();
        }

        if (m_FlagPointWindow->GetSceneWindow ()->Visible ())
        {
            m_FlagPointWindow->GetSceneWindow ()->CloseButtonPressed ();
        }

        if (m_TriggerAreaWindow->GetSceneWindow ()->Visible ())
        {
            m_TriggerAreaWindow->GetSceneWindow ()->CloseButtonPressed ();
        }

        OnCloseSpriteSheetEdit ();
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::ProcessEvent (ALLEGRO_EVENT *event, float)
    {
        if (event->type == ALLEGRO_EVENT_KEY_CHAR)
        {
            if (event->keyboard.keycode == ALLEGRO_KEY_ESCAPE)
            {
                TryToCloseWindows ();
            }
        }

        if (m_CursorMode != CursorMode::EditSpriteSheetMode && m_GUIInput.ProcessMessage (*event))
        {
            return;
        }

        if (event->type == ALLEGRO_EVENT_KEY_CHAR)
        {
            switch (event->keyboard.keycode)
            {
                case ALLEGRO_KEY_R:
                {
                    m_EditorActorMode.ChangeRotation (event->keyboard.modifiers == ALLEGRO_KEYMOD_SHIFT);
                    break;
                }

                case ALLEGRO_KEY_G:
                {
                    ChangeGridSize (event->keyboard.modifiers == ALLEGRO_KEYMOD_SHIFT);
                    break;
                }

                case ALLEGRO_KEY_Z:
                {
                    m_EditorActorMode.ChangeZOrder (event->keyboard.modifiers == ALLEGRO_KEYMOD_SHIFT);
                    break;
                }

                case ALLEGRO_KEY_X:
                {
                    m_EditorActorMode.RemoveSelectedActors ();
                    break;
                }

                case ALLEGRO_KEY_C:
                {
                    m_EditorActorMode.CopySelectedActors ();
                    break;
                }

                case ALLEGRO_KEY_S:
                {
                    if (event->keyboard.modifiers == ALLEGRO_KEYMOD_CTRL)
                    {
                        //  Save requested
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
                    OnPlay ();
                    break;
                }

                case ALLEGRO_KEY_F2:
                {
                    OnActorSelected ();
                    break;
                }

                case ALLEGRO_KEY_F3:
                {
                    OnSpeech ();
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
                    SwitchCursorMode ();
                    break;
                }

                case ALLEGRO_KEY_TILDE:
                {
                    if (m_CursorMode == CursorMode::EditSpriteSheetMode)
                    {
                        OnCloseSpriteSheetEdit ();
                    }
                    else
                    {
                        OnSpriteSheetEdit ();
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

    void Editor::HandleCameraMovement (const ALLEGRO_MOUSE_EVENT &event)
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
        Rect realSelectionRect;

        if (m_CursorMode == CursorMode::EditPhysBodyMode)
        {
            RenderPhysBodyMode (deltaTime);
        }
        else if (m_CursorMode == CursorMode::EditSpriteSheetMode)
        {
            m_MainLoop->GetSceneManager ().GetCamera ().Update (deltaTime);
            m_EditorActorMode.RenderSpriteSheet ();

            realSelectionRect = m_SelectionRect;
        }
        else
        {
            if (m_IsSnapToGrid)
            {
                DrawGrid ();
            }

            m_MainLoop->GetSceneManager ().Render (deltaTime);
            m_MainLoop->GetSceneManager ().GetCamera ().UseIdentityTransform ();

            if (IsEditorCanvasNotCovered ())
            {
                ALLEGRO_MOUSE_STATE state;
                al_get_mouse_state (&state);

                Rect r;
                Actor *actorUnderCursor
                        = m_EditorActorMode.GetActorUnderCursor (state.x, state.y, false, std::move (r));

                if (actorUnderCursor)
                {
                    al_draw_rectangle (r.GetTopLeft ().X, r.GetTopLeft ().Y, r.GetBottomRight ().X,
                                       r.GetBottomRight ().Y, COLOR_YELLOW, 2);
                }

                for (Actor *actor : m_EditorActorMode.GetSelectedActors ())
                {
                    Rect r = m_MainLoop->GetSceneManager ().GetActiveScene ()->GetRenderBounds (actor, true);

                    al_draw_rectangle (r.GetTopLeft ().X, r.GetTopLeft ().Y, r.GetBottomRight ().X,
                                       r.GetBottomRight ().Y, COLOR_RED, 2);
                }

                if (m_CursorMode == CursorMode::ActorSelectMode)
                {
                    m_EditorActorMode.SetActorUnderCursor (actorUnderCursor);
                }
            }

            m_EditorFlagPointMode.DrawFlagPoints ();
            m_EditorTriggerAreaMode.MarkSelectedTriggerAreas ();

            if (m_EditorActorMode.IsDrawTiles ())
            {
                m_EditorActorMode.DrawTiles ();
                RenderUI ();
            }

            if (m_ActorWindow->GetSceneWindow ()->Visible ())
            {
                m_ActorWindow->RenderActorImage ();
            }

            SpeechFrameManager &frameManager = m_MainLoop->GetSceneManager ().GetSpeechFrameManager ();
            frameManager.Update (deltaTime);
            frameManager.Render (deltaTime);

            realSelectionRect = m_MainLoop->GetSceneManager ().GetActiveScene ()->GetRenderBounds (m_SelectionRect);
        }

        //  Prevent drawing selection rect with 1-pixel size (click-down-up)
        if (realSelectionRect.GetSize ().Width > 2 || realSelectionRect.GetSize ().Height > 2)
        {
            al_draw_rectangle (realSelectionRect.GetTopLeft ().X, realSelectionRect.GetTopLeft ().Y,
                               realSelectionRect.GetBottomRight ().X, realSelectionRect.GetBottomRight ().Y,
                               COLOR_WHITE, 2);
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::RenderPhysBodyMode (float deltaTime)
    {
        Camera &camera = m_MainLoop->GetSceneManager ().GetCamera ();
        camera.Update (deltaTime);

        if (!m_EditorActorMode.GetSelectedActors ().empty ())
        {
            Actor *actor = m_EditorActorMode.GetSelectedActors ()[0];

            actor->Render (deltaTime);
            actor->DrawBounds ();
            actor->DrawPhysBody ();

            ALLEGRO_MOUSE_STATE state;
            al_get_mouse_state (&state);

            m_EditorPhysMode.DrawGuideLines ();
            m_EditorPhysMode.DrawPhysPoints (state.x, state.y);
        }

        camera.UseIdentityTransform ();

        RenderUI ();
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::DrawGrid ()
    {
        const ALLEGRO_COLOR DARK_GRAY{0.4f, 0.4f, 0.4f, 1.0f};
        const ALLEGRO_COLOR LIGHT_GRAY{0.5f, 0.5f, 0.5f, 1.0f};

        const Point screenSize = m_MainLoop->GetScreen ()->GetWindowSize ();
        Camera &camera = m_MainLoop->GetSceneManager ().GetCamera ();
        Point trans = camera.GetTranslate ();
        Point scale = camera.GetScale ();
        Point cameraCenter = camera.GetCenter ();

        cameraCenter.X = cameraCenter.X / m_GridSize * scale.X;
        cameraCenter.Y = cameraCenter.Y / m_GridSize * scale.Y;

        int halfSegmentsX = screenSize.Width * 0.5f / m_GridSize;
        int halfSegmentsY = screenSize.Height * 0.5f / m_GridSize;

        int spareSegments = 2;
        int horBeginX = cameraCenter.X - halfSegmentsX - spareSegments;
        int horEndX = cameraCenter.X + halfSegmentsX + spareSegments;

        for (int i = horBeginX;
             i < horEndX;
             ++i)
        {
            float xOffset = i * m_GridSize - trans.X;

            //  |
            int thickness = 1;
            ALLEGRO_COLOR color = DARK_GRAY;

            if (i == 0)
            {
                thickness = 2;
                color = LIGHT_GRAY;
            }

            al_draw_line (xOffset, 0, xOffset, screenSize.Height, color, thickness);
        }

        int horBeginY = cameraCenter.Y - halfSegmentsY - spareSegments;
        int horEndY = cameraCenter.Y + halfSegmentsY + spareSegments;

        for (int i = horBeginY;
             i < horEndY;
             ++i)
        {
            float yOffset = i * m_GridSize - trans.Y;

            //  --
            int thickness = 1;
            ALLEGRO_COLOR color = DARK_GRAY;

            if (i == 0)
            {
                thickness = 2;
                color = LIGHT_GRAY;
            }

            al_draw_line (0, yOffset, screenSize.Width, yOffset, color, thickness);
        }
    }

    //--------------------------------------------------------------------------------------------------

    bool Editor::IsMouseWithinRect (int mouseX, int mouseY, Rect rect)
    {
        Point translate = m_MainLoop->GetSceneManager ().GetCamera ().GetTranslate ();
        Point scale = m_MainLoop->GetSceneManager ().GetCamera ().GetScale ();

        return InsideRect ((mouseX + translate.X) * 1 / scale.X, (mouseY + translate.Y) * 1 / scale.Y, rect);
    }

    //--------------------------------------------------------------------------------------------------

    bool Editor::IsMouseWithinPointRect (int mouseX, int mouseY, Point point, int outsets)
    {
        Rect r = Rect{{point.X - outsets, point.Y - outsets},
                      {point.X + outsets, point.Y + outsets}};

        return IsMouseWithinRect (mouseX, mouseY, r);
    }

    //--------------------------------------------------------------------------------------------------

    Point Editor::CalculateWorldPoint (int pointX, int pointY)
    {
        Point scale = m_MainLoop->GetSceneManager ().GetCamera ().GetScale ();
        Point translate = m_MainLoop->GetSceneManager ().GetCamera ().GetTranslate ();

        float finalX = (pointX + translate.X);
        float finalY = (pointY + translate.Y);

        if (m_IsSnapToGrid)
        {
            finalX = std::floor (finalX / m_GridSize) * m_GridSize;
            finalY = std::floor (finalY / m_GridSize) * m_GridSize;
        }

        return {finalX / scale.X, finalY / scale.Y};
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::OnNewScene (Gwk::Controls::Base *)
    {
        std::function<void (void)> YesFunc = [&]
        {
            m_MainLoop->GetSceneManager ().GetActiveScene ()->Reset ();
            m_MainLoop->GetSceneManager ().GetActiveScene ()->SetName (m_InputWindow->GetText ());

            ResetSettings ();

            UpdateSceneNameLabel (
                    std::string ("SCENE: ") + m_MainLoop->GetSceneManager ().GetActiveScene ()->GetName ());
        };

        m_InputWindow->Show ("Are you sure clearing current scene?", "", YesFunc, nullptr);
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::OnOpenScene (Gwk::Controls::Base *)
    { m_OpenSceneWindow->Show (m_LastScenePath); }

    //--------------------------------------------------------------------------------------------------

    void Editor::OnSaveScene (Gwk::Controls::Base *)
    { m_SaveSceneWindow->Show (m_LastScenePath); }

    //--------------------------------------------------------------------------------------------------

    void Editor::LoadScene (const std::string &openFileName)
    {
        std::string path = GetDataPath () + "scenes/" + openFileName;

        if (IsFileExists (path))
        {
            m_MainLoop->GetSceneManager ().RemoveScene (m_MainLoop->GetSceneManager ().GetActiveScene ());
            m_MainLoop->GetSceneManager ().SetActiveScene (openFileName, false);

            ResetSettings ();

            UpdateSceneNameLabel (
                    std::string ("SCENE: " + m_MainLoop->GetSceneManager ().GetActiveScene ()->GetName ()));

            m_LastScenePath = m_OpenSceneWindow->GetFileName ();

            Log ("Scene loaded: %s", openFileName.c_str ());
        }
        else
        {
            Log ("Scene %s doesn't exist!", openFileName.c_str ());
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::SaveScene (const std::string &filePath)
    {
        std::string path = GetDataPath () + "scenes/" + filePath;
        SceneLoader::SaveScene (m_MainLoop->GetSceneManager ().GetActiveScene (), path);

        m_LastScenePath = m_SaveSceneWindow->GetFileName ();

        Log ("Scene saved: %s", filePath.c_str ());
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::ResetSettings ()
    {
        m_EditorActorMode.ResetSettings ();
        m_EditorPhysMode.ResetSettings ();

        OnResetScale ();
        OnResetTranslate ();

        m_MainLoop->GetSceneManager ().GetActiveScene ()->ResetAllActorsPositions ();
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::OnPlay ()
    { m_MainLoop->GetStateManager ().SetActiveState ("GAMEPLAY_STATE"); }

    //--------------------------------------------------------------------------------------------------

    void Editor::OnScrollPrevTiles ()
    { m_EditorActorMode.ScrollPrevTile (1); }

    //--------------------------------------------------------------------------------------------------

    void Editor::OnScrollNextTiles ()
    { m_EditorActorMode.ScrollNextTile (1); }

    //--------------------------------------------------------------------------------------------------

    void Editor::OnBigScrollPrevTiles ()
    { m_EditorActorMode.ScrollPrevTile (TILES_COUNT); }

    //--------------------------------------------------------------------------------------------------

    void Editor::OnSpriteSheetEdit ()
    {
        m_OldSnapToGrid = m_IsSnapToGrid;
        m_IsSnapToGrid = false;

        SetCursorMode (EditSpriteSheetMode);
        m_WorldTransform = m_MainLoop->GetSceneManager ().GetCamera ().GetCurrentTransform ();
        m_MainLoop->GetSceneManager ().GetCamera ().SetCurrentTransform (m_NewTransform);
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::OnCloseSpriteSheetEdit ()
    {
        if (GetCursorMode () == EditSpriteSheetMode)
        {
            m_IsSnapToGrid = m_OldSnapToGrid;
            m_NewTransform = m_MainLoop->GetSceneManager ().GetCamera ().GetCurrentTransform ();
            m_MainLoop->GetSceneManager ().GetCamera ().SetCurrentTransform (m_WorldTransform);
            SetCursorMode (ActorSelectMode);
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::OnBigScrollNextTiles ()
    { m_EditorActorMode.ScrollNextTile (TILES_COUNT); }

    //--------------------------------------------------------------------------------------------------

    void Editor::OnResetTranslate ()
    {
        const Point &windowSize = m_MainLoop->GetScreen ()->GetWindowSize ();
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

    void Editor::OnSceneEdit ()
    { m_EditorSceneWindow->Show (); }

    //--------------------------------------------------------------------------------------------------

    void Editor::OnFlagPoint ()
    { m_FlagPointWindow->Show (); }

    //--------------------------------------------------------------------------------------------------

    void Editor::OnTriggerArea ()
    { m_TriggerAreaWindow->Show (); }

    //--------------------------------------------------------------------------------------------------

    void Editor::SwitchCursorMode ()
    {
        if (m_CursorMode != CursorMode::EditPhysBodyMode)
        {
            if (!m_EditorActorMode.GetSelectedActors ().empty ())
            {
                Camera &camera = m_MainLoop->GetSceneManager ().GetCamera ();

                //  In case of messed up scaling...
                OnResetScale ();

                Point center = m_EditorActorMode.GetSelectedActors ()[0]->Bounds.GetCenter ();

                //  Ok, it works, but this strange division?
                camera.SetCenter (center * 0.5f);

                SetCursorMode (CursorMode::EditPhysBodyMode);
            }
        }
        else
        {
            SetCursorMode (CursorMode::ActorSelectMode);
        }

        SetDrawUITiles (m_CursorMode == CursorMode::ActorSelectMode);
        flagPointButton->SetHidden (m_CursorMode == CursorMode::EditPhysBodyMode);
        triggerAreaButton->SetHidden (m_CursorMode == CursorMode::EditPhysBodyMode);
        speechButton->SetHidden (m_CursorMode == CursorMode::EditPhysBodyMode);
        actorButton->SetHidden (m_CursorMode == CursorMode::EditPhysBodyMode);
        newPolyButton->SetHidden (m_CursorMode != CursorMode::EditPhysBodyMode);
        saveBodyButton->SetHidden (m_CursorMode != CursorMode::EditPhysBodyMode);
        removeBodyButton->SetHidden (m_CursorMode != CursorMode::EditPhysBodyMode);

        selectModeButton->SetText (m_CursorMode != CursorMode::EditPhysBodyMode ? "PHYS MODE" : "EXIT PHYS");
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::OnRemoveBody ()
    {
        if (!m_EditorActorMode.GetSelectedActors ().empty ())
        {
            m_EditorActorMode.GetSelectedActors ()[0]->PhysPoints.clear ();

            if (m_EditorPhysMode.GetPhysPoly ())
            {
                m_EditorPhysMode.GetPhysPoly ()->clear ();
            }

            m_EditorPhysMode.SetPhysPoint (nullptr);
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::OutlineBody ()
    {
        if (!m_EditorActorMode.GetSelectedActors ().empty ())
        {
            Actor *actor = m_EditorActorMode.GetSelectedActors ()[0];
            Rect rect = actor->Bounds;
            Point halfSize = rect.GetHalfSize ();

            std::vector<Point> points;

            points.push_back ({-halfSize.Width, -halfSize.Height});
            points.push_back ({halfSize.Width, -halfSize.Height});
            points.push_back ({halfSize.Width, halfSize.Height});
            points.push_back ({-halfSize.Width, halfSize.Height});

            actor->PhysPoints.push_back (points);
            actor->UpdatePhysPolygon ();

            m_EditorPhysMode.SetPhysPoint (&points[0]);
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::OnNewPoly ()
    {
        m_EditorPhysMode.SetPhysPoly (nullptr);
        m_EditorPhysMode.SetPhysPoint (nullptr);

        if (m_EditorActorMode.GetSelectedActors ().empty ())
        {
            Actor *actor = m_EditorActorMode.GetSelectedActors ()[0];

            actor->PhysPoints.push_back ({});
            m_EditorPhysMode.SetPhysPoly (&actor->PhysPoints[actor->PhysPoints.size () - 1]);
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::OnSpeech ()
    {
        m_SpeechWindow->UpdateSpeechesTree ();
        m_SpeechWindow->Show ();
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::OnActorSelected ()
    {
        m_ActorWindow->Show ();

        if (!m_EditorActorMode.GetSelectedActors ().empty ())
        {
            Actor *actor = m_EditorActorMode.GetSelectedActors ()[0];
            std::string name = actor->Name + std::string (" [") + ToString (actor->ID) + std::string ("]");

            m_ActorWindow->SelectActor (name);
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::MarkPlayerPosition ()
    {
        ALLEGRO_MOUSE_STATE state;
        al_get_mouse_state (&state);

        Point point = CalculateWorldPoint (state.x, state.y);

        m_MainLoop->GetSceneManager ().GetActiveScene ()->SetPlayerStartLocation (point);

        Player *player = m_MainLoop->GetSceneManager ().GetPlayer ();

        point -= player->Bounds.GetHalfSize ();

        player->SetPosition (point);
        player->TemplateBounds.Pos = player->GetPosition ();
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::RenderUI ()
    {
        Point translate = m_MainLoop->GetSceneManager ().GetCamera ().GetTranslate ();
        Point scale = m_MainLoop->GetSceneManager ().GetCamera ().GetScale ();

        ALLEGRO_MOUSE_STATE state;
        al_get_mouse_state (&state);

        avgFPSLabel->SetText (
                Gwk::Utility::Format ("        AVG: %.2f ms", 1000.0f / m_MainLoop->GetScreen ()->GetFPS ()));
        avgFPSLabel->SizeToContents ();

        fpsLabel->SetText (Gwk::Utility::Format ("         FPS: %.1f", m_MainLoop->GetScreen ()->GetFPS ()));
        fpsLabel->SizeToContents ();

        Entity *selectedEntity = nullptr;

        if (!m_EditorActorMode.GetSelectedActors ().empty ())
        {
            selectedEntity = m_EditorActorMode.GetSelectedActors ()[0];
        }

        idLabel->SetText (std::string ("            ID: " + (selectedEntity ? ToString (selectedEntity->ID) : "-")));
        idLabel->SizeToContents ();

        xPosLabel->SetText (std::string ("             X: " + ToString ((translate.X + state.x) * (1 / scale.X))));
        xPosLabel->SizeToContents ();

        yPosLabel->SetText (std::string ("             Y: " + ToString ((translate.Y + state.y) * (1 / scale.Y))));
        yPosLabel->SizeToContents ();

        AtlasRegion *atlasRegion = m_EditorActorMode.GetSelectedAtlasRegion ();
        Point size = Point::ZERO_POINT;

        if (atlasRegion)
        {
            size = atlasRegion->Bounds.GetSize ();
        }

        widthLabel->SetText (std::string ("            W: " + ToString (size.Width)));
        widthLabel->SizeToContents ();

        heightLabel->SetText (std::string ("             H: " + ToString (size.Height)));
        heightLabel->SizeToContents ();

        angleLabel->SetText (
                std::string ("             A: " + (selectedEntity ? ToString (selectedEntity->Rotation) : "-")));
        angleLabel->SizeToContents ();

        zOrderLabel->SetText (std::string ("     ZORD: " + (selectedEntity ? ToString (selectedEntity->ZOrder) : "-")));
        zOrderLabel->SizeToContents ();

        scaleLabel->SetText (std::string ("             S: " + ToString (scale.X)));
        scaleLabel->SizeToContents ();

        snapLabel->SetText (std::string ("     SNAP: " + ToString (m_IsSnapToGrid ? "YES" : "NO")));
        snapLabel->SizeToContents ();

        gridLabel->SetText (std::string ("      GRID: " + ToString (m_BaseGridSize)));
        gridLabel->SizeToContents ();

        Scene *activeScene = m_MainLoop->GetSceneManager ().GetActiveScene ();

        tilesLabel->SetText (std::string ("     TILES: " + ToString (activeScene->GetTiles ().size ())));
        tilesLabel->SizeToContents ();

        actorsLabel->SetText (std::string ("ACTORS: " + ToString (activeScene->GetActors ().size ())));
        actorsLabel->SizeToContents ();

        std::string cursorMode = "";

        switch (m_CursorMode)
        {
            case ActorSelectMode:
                cursorMode = "ACTOR";
                break;
            case EditPhysBodyMode:
                cursorMode = "PHYS";
                break;
            case EditFlagPointsMode:
                cursorMode = "FLAG";
                break;
            case EditTriggerAreaMode:
                cursorMode = "TRIGGER";
                break;
            case EditSpriteSheetMode:
                cursorMode = "SPRITE";
                break;
        };

        cursorLabel->SetText (std::string ("CURSOR: " + cursorMode));
        cursorLabel->SizeToContents ();

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
        spriteSheetButton->SetHidden (!m_EditorActorMode.IsDrawTiles ());
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
        tilesLabel->SetPos (m_MainCanvas->Width () - 120.0f, gridLabel->Bottom () + 5);
        actorsLabel->SetPos (m_MainCanvas->Width () - 120.0f, tilesLabel->Bottom () + 5);
        cursorLabel->SetPos (m_MainCanvas->Width () - 120.0f, actorsLabel->Bottom () + 5);

        float beginning = screenSize.Width * 0.5 - (TILES_COUNT - 1) * 0.5 * TILE_SIZE - TILE_SIZE * 0.5;

        tilesetCombo->SetPos (beginning - 140, m_MainCanvas->Bottom () - 35);
        leftNextTileButton->SetPos (beginning - 35, m_MainCanvas->Bottom () - TILE_SIZE + 5);
        leftPrevTileButton->SetPos (beginning - 35, leftNextTileButton->Bottom () + 2);
        rightNextTileButton->SetPos (beginning + TILES_COUNT * TILE_SIZE + 5, m_MainCanvas->Bottom () - TILE_SIZE + 5);
        rightPrevTileButton->SetPos (beginning + TILES_COUNT * TILE_SIZE + 5, rightNextTileButton->Bottom () + 2);
        spriteSheetButton->SetPos (rightNextTileButton->Right () + 5, rightNextTileButton->Y ());
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::ProcessMouseButtonDoubleClick (ALLEGRO_MOUSE_EVENT &event)
    {
        if (event.button == 1)
        {
            OnActorSelected ();
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::ProcessMouseButtonDown (ALLEGRO_MOUSE_EVENT &event)
    {
        m_IsMousePan = event.button == 3;

        if (event.button == 1)
        {
            if (m_EditorActorMode.IsDrawTiles ())
            {
                if (m_EditorActorMode.ChooseTile (event.x, event.y))
                {
                    OnCloseSpriteSheetEdit ();
                    m_EditorActorMode.AddActor (event.x, event.y);
                }
            }

            if (m_EditorActorMode.IsSpriteSheetChoosen ())
            {
                m_EditorActorMode.SetSpriteSheetChoosen (false);
            }

            ALLEGRO_KEYBOARD_STATE state;
            al_get_keyboard_state (&state);

            if (al_key_down (&state, ALLEGRO_KEY_LSHIFT))
            {
                Rect r;
                Actor *actorUnderCursor = m_EditorActorMode.GetActorUnderCursor (event.x, event.y, true, std::move (r));

                if (actorUnderCursor)
                {
                    if (m_EditorActorMode.IsActorSelected (actorUnderCursor))
                    {
                        m_EditorActorMode.RemoveActorFromSelection (actorUnderCursor);
                    }
                    else
                    {
                        m_EditorActorMode.AddActorToSelection (actorUnderCursor);
                        selectModeButton->Show ();
                    }
                }
                else
                {
                    m_EditorActorMode.ClearSelectedActors ();
                    selectModeButton->Hide ();
                }
            }
            else if (m_CursorMode == CursorMode::EditSpriteSheetMode)
            {
                m_IsRectSelection = true;
                m_SelectionRect.Pos = CalculateWorldPoint (event.x, event.y);
            }
            else if (m_CursorMode == CursorMode::ActorSelectMode)
            {
                Rect r;
                Actor *actorUnderCursor = m_EditorActorMode.GetActorUnderCursor (event.x, event.y, true, std::move (r));

                if (!actorUnderCursor)
                {
                    m_IsRectSelection = true;
                    m_SelectionRect.Pos = CalculateWorldPoint (event.x, event.y);

                    m_EditorActorMode.ClearSelectedActors ();
                }
                else
                {
                    if (!m_EditorActorMode.IsActorSelected (actorUnderCursor))
                    {
                        m_EditorActorMode.ClearSelectedActors ();
                        m_EditorActorMode.AddActorToSelection (actorUnderCursor);
                        selectModeButton->Show ();

                        if (!m_EditorActorMode.GetSelectedActors ().empty ())
                        {
                            Actor *actor = m_EditorActorMode.GetSelectedActors ()[0];
                            m_EditorActorMode.SetRotation (actor->Rotation);

                            if (actor->PhysPoints.empty ())
                            {
                                actor->PhysPoints.push_back ({});
                            }

                            m_EditorPhysMode.SetPhysPoly (&actor->PhysPoints[0]);
                        }
                    }
                    else
                    {
                        m_EditorActorMode.SetPrimarySelectedActor (actorUnderCursor);
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
                SetCursorMode (CursorMode::EditTriggerAreaMode);
            }
        }

        if (event.button == 2)
        {
            bool flagPointRemoved = m_EditorFlagPointMode.RemoveFlagPointUnderCursor (event.x, event.y);
            bool triggerPointRemoved = m_EditorTriggerAreaMode.RemoveTriggerPointUnderCursor (event.x, event.y);

            m_EditorFlagPointMode.SetFlagPoint ("");

            if (m_CursorMode == CursorMode::EditPhysBodyMode)
            {
                m_EditorPhysMode.RemovePhysPointUnderCursor (event.x, event.y);
            }
            else if (!flagPointRemoved && !triggerPointRemoved)
            {
                SetCursorMode (CursorMode::ActorSelectMode);
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::ProcessMouseButtonUp (ALLEGRO_MOUSE_EVENT &event)
    {
        if (m_CursorMode == CursorMode::EditSpriteSheetMode && event.button == 1 && m_EditorActorMode.IsDrawTiles ())
        {
            AddActorsFromSpritesheet (event.x, event.y);
        }
        else
        {
            SelectActorsWithinSelectionRect ();
        }

        m_IsMousePan = false;
        m_IsRectSelection = false;
        m_SelectionRect = {};

        if (m_CursorMode == CursorMode::EditPhysBodyMode && event.button == 1)
        {
            SelectPhysPoint ();
        }

        if (m_CursorMode == CursorMode::EditTriggerAreaMode && event.button == 1)
        {
            SelectTriggerAreaPoint ();
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::ProcessMouseAxes (ALLEGRO_MOUSE_EVENT &event)
    {
        if (!m_EditorFlagPointMode.MoveSelectedFlagPoint () && !m_EditorTriggerAreaMode.MoveSelectedTriggerPoint ())
        {
            if (m_CursorMode == CursorMode::EditPhysBodyMode)
            {
                m_EditorPhysMode.MoveSelectedPhysPoint ();
            }
            else
            {
                ALLEGRO_MOUSE_STATE state;
                al_get_mouse_state (&state);

                if (state.buttons == 1 || m_EditorActorMode.IsSpriteSheetChoosen ())
                {
                    m_EditorActorMode.MoveSelectedActors (state.x, state.y);
                }

                if (m_IsRectSelection)
                {
                    m_SelectionRect.SetBottomRight (CalculateWorldPoint (event.x, event.y));
                }
            }
        }

        if (IsEditorCanvasNotCovered ())
        {
            HandleCameraMovement (event);
        }

        m_LastMousePos = {event.x, event.y};
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::AddActorsFromSpritesheet (float x, float y)
    {
        if (m_EditorActorMode.ChooseTilesFromSpriteSheet (x, y))
        {
            m_EditorActorMode.ClearSelectedActors ();

            OnCloseSpriteSheetEdit ();
            m_EditorActorMode.AddActor (x, y);
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::SelectActorsWithinSelectionRect ()
    {
        if (m_SelectionRect.GetSize ().Width > 2 || m_SelectionRect.GetSize ().Height > 2)
        {
            std::vector<Actor *> &actors = m_MainLoop->GetSceneManager ().GetActiveScene ()->GetActors ();
            Rect selectionWorld = m_MainLoop->GetSceneManager ().GetActiveScene ()->GetRenderBounds (m_SelectionRect);

            for (Actor *actorIt : actors)
            {
                Rect r = m_MainLoop->GetSceneManager ().GetActiveScene ()->GetRenderBounds (actorIt);

                if (Intersect (r, selectionWorld))
                {
                    m_EditorActorMode.AddActorToSelection (actorIt);
                }
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::SelectPhysPoint ()
    {
        if (m_EditorPhysMode.GetPhysPoint () && !m_EditorActorMode.GetSelectedActors ().empty ())
        {
            m_EditorActorMode.GetSelectedActors ()[0]->UpdatePhysPolygon ();

            if (!m_EditorPhysMode.GetPhysPoint () && m_EditorPhysMode.GetPhysPoly ()
                && !(*m_EditorPhysMode.GetPhysPoly ()).empty ())
            {
                m_EditorPhysMode.SetPhysPoint (&(*m_EditorPhysMode.GetPhysPoly ())[0]);
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::SelectTriggerAreaPoint ()
    {
        if (m_EditorTriggerAreaMode.GetTriggerArea ())
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

    void Editor::OnTilesetSelected (Gwk::Controls::Base *)
    {
        Gwk::Controls::Label *selItem = tilesetCombo->GetSelectedItem ();

        m_EditorActorMode.ChangeAtlas (selItem->GetText ());
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::UpdateSceneNameLabel (const std::string &name)
    {
        sceneNameLabel->SetText (name);
        sceneNameLabel->SizeToContents ();
    }

    //--------------------------------------------------------------------------------------------------
} // namespace aga
