// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __EDITOR_H__
#define __EDITOR_H__

#include "Common.h"
#include "EditorFlagPointMode.h"
#include "EditorPhysMode.h"
#include "EditorSpeechMode.h"
#include "EditorTileMode.h"
#include "EditorTriggerAreaMode.h"
#include "Scene.h"

#include <Gwork/Controls.h>
#include <Gwork/Controls/WindowControl.h>
#include <Gwork/Input/Allegro5.h>
#include <Gwork/Platform.h>
#include <Gwork/Renderers/Allegro5.h>
#include <Gwork/Skins/TexturedBase.h>

namespace aga
{
    extern const int TILES_COUNT;

    class OpenSceneWindow;
    class SaveSceneWindow;
    class FlagPointWindow;
    class TriggerAreaWindow;
    class SpeechWindow;
    class InfoWindow;

    class MainLoop;
    struct Tile;

    enum CursorMode
    {
        TileSelectMode,
        TileEditMode,
        EditPhysBodyMode,
        EditFlagPointsMode,
        EditTriggerAreaMode,
        SpeechMode
    };

    class Editor : public Lifecycle, public Gwk::Event::Handler
    {
        friend class OpenSceneWindow;
        friend class SaveSceneWindow;
        friend class FlagPointWindow;
        friend class TriggerAreaWindow;
        friend class SpeechWindow;

        friend class EditorTileMode;
        friend class EditorPhysMode;
        friend class EditorFlagPointMode;
        friend class EditorTriggerAreaMode;
        friend class EditorSpeechMode;

    public:
        Editor (MainLoop* mainLoop);
        virtual ~Editor ();
        bool Initialize ();
        bool Destroy ();

        bool Update (float deltaTime);
        void ProcessEvent (ALLEGRO_EVENT* event, float deltaTime);
        void Render (float deltaTime);

        void OnResetTranslate ();
        void OnResetScale ();

        void SetDrawUITiles (bool draw);

    private:
        void LoadConfig ();
        void SaveConfig ();

        void DrawGrid ();

        void ChangeGridSize (bool clockwise);

        void HandleCameraMovement (const ALLEGRO_MOUSE_EVENT& event);

        void OnNewScene (Gwk::Controls::Base* control);
        void OnOpenScene (Gwk::Controls::Base* control);
        void OnSaveScene (Gwk::Controls::Base* control);

        void LoadScene (const std::string& filePath);
        void SaveScene (const std::string& filePath);

        void OnPlay ();
        void MenuItemPlay ();

        void OnExit ();

        void OnShowGrid ();
        void OnGridIncrease ();
        void OnGridDecrease ();

        void OnFlagPoint ();
        void OnTriggerArea ();

        void OnTileMode ();
        void OnNewPoly ();

        void OnSpeech ();

        void OnReloadScript ();

        Point CalculateCursorPoint (int mouseX, int mouseY);

        bool IsMouseWithinPointRect (int mouseX, int mouseY, Point point, int outsets);

        void RenderUI ();

        void ResetSettings ();

    private:
        MainLoop* m_MainLoop;
        EditorTileMode m_EditorTileMode;
        EditorPhysMode m_EditorPhysMode;
        EditorFlagPointMode m_EditorFlagPointMode;
        EditorTriggerAreaMode m_EditorTriggerAreaMode;
        EditorSpeechMode m_EditorSpeechMode;

        CursorMode m_CursorMode;

        bool m_IsSnapToGrid;
        int m_BaseGridSize;
        int m_GridSize;

        bool m_IsMousePan;
        bool m_IsMouseWheel;

        OpenSceneWindow* m_OpenSceneWindow;
        SaveSceneWindow* m_SaveSceneWindow;
        FlagPointWindow* m_FlagPointWindow;
        TriggerAreaWindow* m_TriggerAreaWindow;
        SpeechWindow* m_SpeechWindow;
        InfoWindow* m_InfoWindow;
    };

    struct InfoWindow : public Gwk::Event::Handler
    {
        InfoWindow (Editor* editor, Gwk::Controls::Canvas* canvas)
        {
            m_Editor = editor;

            m_SceneWindow = new Gwk::Controls::WindowControl (canvas);
            m_SceneWindow->SetTitle ("Info");
            m_SceneWindow->SetSize (450, 120);
            m_SceneWindow->CloseButtonPressed ();

            m_Label = new Gwk::Controls::Label (m_SceneWindow);
            m_Label->SetBounds (20, 10, m_SceneWindow->Width () - 40, m_SceneWindow->Height () - 40);
            m_Label->SetText (m_Text);

            Gwk::Controls::Button* okButton = new Gwk::Controls::Button (m_SceneWindow);
            okButton->SetText ("OK");
            okButton->SetPos (m_SceneWindow->Width () / 2 - 50, m_SceneWindow->Height () - 60);
            okButton->onPress.Add (this, &InfoWindow::OnAccept);
        }

        void Show (const std::string& text)
        {
            m_Text = text;

            m_Label->SetText (m_Text);
            m_Label->SetWrap (true);
            m_SceneWindow->SetPosition (Gwk::Position::Center);
            m_SceneWindow->SetHidden (false);
            m_SceneWindow->MakeModal (true);
        }

        void OnAccept () { m_SceneWindow->CloseButtonPressed (); }

    private:
        Editor* m_Editor;
        Gwk::Controls::WindowControl* m_SceneWindow;
        Gwk::Controls::Label* m_Label;
        std::string m_Text;
    };
}

#endif //   __EDITOR_H__
