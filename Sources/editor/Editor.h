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

    struct OpenSceneWindow;
    struct SaveSceneWindow;
    struct FlagPointWindow;
    struct TriggerAreaWindow;
    struct SpeechWindow;
    struct InfoWindow;

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

    struct OpenSceneWindow : public Gwk::Event::Handler
    {
        OpenSceneWindow (Editor* editor, Gwk::Controls::Canvas* canvas, const std::string& fileName)
        {
            m_Editor = editor;
            m_FileName = fileName;

            m_SceneWindow = new Gwk::Controls::WindowControl (canvas);
            m_SceneWindow->SetTitle ("Open Scene");
            m_SceneWindow->SetSize (450, 120);
            m_SceneWindow->CloseButtonPressed ();

            //   openSceneWindow->SetDeleteOnClose (true);

            Gwk::Controls::Label* pathLabel = new Gwk::Controls::Label (m_SceneWindow);
            pathLabel->SetPos (20, 10);
            pathLabel->SetText ("File path:");
            pathLabel->SizeToContents ();

            Gwk::Controls::TextBox* pathTextBox = new Gwk::Controls::TextBox (m_SceneWindow);
            pathTextBox->SetText (m_FileName);
            pathTextBox->SetTextColor (Gwk::Colors::White);
            pathTextBox->SetWidth (300);
            pathTextBox->SetPos (20, 30);
            pathTextBox->onTextChanged.Add (this, &OpenSceneWindow::OnEdit);

            Gwk::Controls::Button* browseButton = new Gwk::Controls::Button (m_SceneWindow);
            browseButton->SetText ("BROWSE");
            browseButton->SetPos (330, 30);
            browseButton->onPress.Add (this, &OpenSceneWindow::OnOpen);

            Gwk::Controls::Button* okButton = new Gwk::Controls::Button (m_SceneWindow);
            okButton->SetText ("OPEN");
            okButton->SetPos (120, 60);
            okButton->onPress.Add (this, &OpenSceneWindow::OnOpen);

            Gwk::Controls::Button* cancelButton = new Gwk::Controls::Button (m_SceneWindow);
            cancelButton->SetText ("CANCEL");
            cancelButton->SetPos (okButton->GetPos ().x + okButton->GetSize ().x + 10, 60);
            cancelButton->onPress.Add (this, &OpenSceneWindow::OnCancel);
        }

        void Show ()
        {
            m_SceneWindow->SetPosition (Gwk::Position::Center);
            m_SceneWindow->SetHidden (false);
            m_SceneWindow->MakeModal (true);
        }

        void OnOpen (Gwk::Controls::Base*)
        {
            m_Editor->LoadScene (m_FileName);
            m_SceneWindow->CloseButtonPressed ();
        }

        void OnCancel () { m_SceneWindow->CloseButtonPressed (); }

        void OnEdit (Gwk::Controls::Base* control)
        {
            Gwk::Controls::TextBox* textbox = (Gwk::Controls::TextBox*)(control);
            m_FileName = textbox->GetText ();
        }

    private:
        Editor* m_Editor;
        Gwk::Controls::WindowControl* m_SceneWindow;
        std::string m_FileName;
    };

    struct SaveSceneWindow : public Gwk::Event::Handler
    {
        SaveSceneWindow (Editor* editor, Gwk::Controls::Canvas* canvas, const std::string& fileName)
        {
            m_Editor = editor;
            m_FileName = fileName;

            m_SceneWindow = new Gwk::Controls::WindowControl (canvas);
            m_SceneWindow->SetTitle ("Save Scene");
            m_SceneWindow->SetSize (450, 120);
            m_SceneWindow->CloseButtonPressed ();

            //   openSceneWindow->SetDeleteOnClose (true);

            Gwk::Controls::Label* pathLabel = new Gwk::Controls::Label (m_SceneWindow);
            pathLabel->SetPos (20, 10);
            pathLabel->SetText ("File path:");
            pathLabel->SizeToContents ();

            Gwk::Controls::TextBox* pathTextBox = new Gwk::Controls::TextBox (m_SceneWindow);
            pathTextBox->SetText (m_FileName);
            pathTextBox->SetTextColor (Gwk::Colors::White);
            pathTextBox->SetWidth (300);
            pathTextBox->SetPos (20, 30);
            pathTextBox->onTextChanged.Add (this, &SaveSceneWindow::OnEdit);

            Gwk::Controls::Button* browseButton = new Gwk::Controls::Button (m_SceneWindow);
            browseButton->SetText ("BROWSE");
            browseButton->SetPos (330, 30);
            browseButton->onPress.Add (this, &SaveSceneWindow::OnSave);

            Gwk::Controls::Button* okButton = new Gwk::Controls::Button (m_SceneWindow);
            okButton->SetText ("SAVE");
            okButton->SetPos (120, 60);
            okButton->onPress.Add (this, &SaveSceneWindow::OnSave);

            Gwk::Controls::Button* cancelButton = new Gwk::Controls::Button (m_SceneWindow);
            cancelButton->SetText ("CANCEL");
            cancelButton->SetPos (okButton->GetPos ().x + okButton->GetSize ().x + 10, 60);
            cancelButton->onPress.Add (this, &SaveSceneWindow::OnCancel);
        }

        void Show ()
        {
            m_SceneWindow->SetPosition (Gwk::Position::Center);
            m_SceneWindow->SetHidden (false);
            m_SceneWindow->MakeModal (true);
        }

        void OnSave (Gwk::Controls::Base*)
        {
            m_Editor->SaveScene (m_FileName);
            m_SceneWindow->CloseButtonPressed ();
        }

        void OnCancel () { m_SceneWindow->CloseButtonPressed (); }

        void OnEdit (Gwk::Controls::Base* control)
        {
            Gwk::Controls::TextBox* textbox = (Gwk::Controls::TextBox*)(control);
            m_FileName = textbox->GetText ();
        }

    private:
        Editor* m_Editor;
        Gwk::Controls::WindowControl* m_SceneWindow;
        std::string m_FileName;
    };

    struct FlagPointWindow : public Gwk::Event::Handler
    {
        FlagPointWindow (Editor* editor, Gwk::Controls::Canvas* canvas)
        {
            m_Editor = editor;

            m_SceneWindow = new Gwk::Controls::WindowControl (canvas);
            m_SceneWindow->SetTitle ("Flag Point");
            m_SceneWindow->SetSize (450, 120);
            m_SceneWindow->CloseButtonPressed ();

            Gwk::Controls::Label* pathLabel = new Gwk::Controls::Label (m_SceneWindow);
            pathLabel->SetPos (20, 10);
            pathLabel->SetText ("Name:");
            pathLabel->SizeToContents ();

            Gwk::Controls::TextBox* pathTextBox = new Gwk::Controls::TextBox (m_SceneWindow);
            pathTextBox->SetText (m_Editor->m_EditorFlagPointMode.m_FlagPointName);
            pathTextBox->SetTextColor (Gwk::Colors::White);
            pathTextBox->SetWidth (350);
            pathTextBox->SetPos (20, 30);
            pathTextBox->onTextChanged.Add (this, &FlagPointWindow::OnEdit);

            Gwk::Controls::Button* okButton = new Gwk::Controls::Button (m_SceneWindow);
            okButton->SetText ("ACCEPT");
            okButton->SetPos (120, 60);
            okButton->onPress.Add (this, &FlagPointWindow::OnAccept);

            Gwk::Controls::Button* cancelButton = new Gwk::Controls::Button (m_SceneWindow);
            cancelButton->SetText ("CANCEL");
            cancelButton->SetPos (okButton->GetPos ().x + okButton->GetSize ().x + 10, 60);
            cancelButton->onPress.Add (this, &FlagPointWindow::OnCancel);
        }

        void Show ()
        {
            m_Editor->m_EditorFlagPointMode.m_AskFlagPoint = false;

            m_SceneWindow->SetPosition (Gwk::Position::Center);
            m_SceneWindow->SetHidden (false);
            m_SceneWindow->MakeModal (true);
        }

        void OnAccept (Gwk::Controls::Base*)
        {
            m_Editor->m_CursorMode = CursorMode::EditFlagPointsMode;
            m_SceneWindow->CloseButtonPressed ();
        }

        void OnCancel ()
        {
            m_Editor->m_CursorMode = CursorMode::TileSelectMode;
            m_Editor->m_EditorFlagPointMode.m_FlagPointName = "";
            m_SceneWindow->CloseButtonPressed ();
        }

        void OnEdit (Gwk::Controls::Base* control)
        {
            Gwk::Controls::TextBox* textbox = (Gwk::Controls::TextBox*)(control);
            m_Editor->m_EditorFlagPointMode.m_FlagPointName = textbox->GetText ();
        }

    private:
        Editor* m_Editor;
        Gwk::Controls::WindowControl* m_SceneWindow;
    };

    struct TriggerAreaWindow : public Gwk::Event::Handler
    {
        TriggerAreaWindow (Editor* editor, Gwk::Controls::Canvas* canvas)
        {
            m_Editor = editor;

            m_SceneWindow = new Gwk::Controls::WindowControl (canvas);
            m_SceneWindow->SetTitle ("Trigger Area");
            m_SceneWindow->SetSize (450, 120);
            m_SceneWindow->CloseButtonPressed ();

            Gwk::Controls::Label* pathLabel = new Gwk::Controls::Label (m_SceneWindow);
            pathLabel->SetPos (20, 10);
            pathLabel->SetText ("Name:");
            pathLabel->SizeToContents ();

            Gwk::Controls::TextBox* pathTextBox = new Gwk::Controls::TextBox (m_SceneWindow);
            pathTextBox->SetText (m_Editor->m_EditorTriggerAreaMode.m_TriggerAreaName);
            pathTextBox->SetTextColor (Gwk::Colors::White);
            pathTextBox->SetWidth (350);
            pathTextBox->SetPos (20, 30);
            pathTextBox->onTextChanged.Add (this, &TriggerAreaWindow::OnEdit);

            Gwk::Controls::Button* okButton = new Gwk::Controls::Button (m_SceneWindow);
            okButton->SetText ("ACCEPT");
            okButton->SetPos (120, 60);
            okButton->onPress.Add (this, &TriggerAreaWindow::OnAccept);

            Gwk::Controls::Button* cancelButton = new Gwk::Controls::Button (m_SceneWindow);
            cancelButton->SetText ("CANCEL");
            cancelButton->SetPos (okButton->GetPos ().x + okButton->GetSize ().x + 10, 60);
            cancelButton->onPress.Add (this, &TriggerAreaWindow::OnCancel);
        }

        void Show ()
        {
            m_Editor->m_EditorTriggerAreaMode.m_TriggerAreaName = "";

            m_SceneWindow->SetPosition (Gwk::Position::Center);
            m_SceneWindow->SetHidden (false);
            m_SceneWindow->MakeModal (true);
        }

        void OnAccept (Gwk::Controls::Base*)
        {
            m_Editor->m_CursorMode = CursorMode::EditTriggerAreaMode;
            m_Editor->m_EditorTriggerAreaMode.NewTriggerArea ();
            m_SceneWindow->CloseButtonPressed ();
        }

        void OnCancel ()
        {
            m_Editor->m_CursorMode = CursorMode::TileSelectMode;
            m_Editor->m_EditorTriggerAreaMode.m_TriggerAreaName = "";
            m_SceneWindow->CloseButtonPressed ();
        }

        void OnEdit (Gwk::Controls::Base* control)
        {
            Gwk::Controls::TextBox* textbox = (Gwk::Controls::TextBox*)(control);
            m_Editor->m_EditorTriggerAreaMode.m_TriggerAreaName = textbox->GetText ();
        }

    private:
        Editor* m_Editor;
        Gwk::Controls::WindowControl* m_SceneWindow;
    };

    struct SpeechWindow : public Gwk::Event::Handler
    {
        SpeechWindow (Editor* editor, Gwk::Controls::Canvas* canvas);

        void Show ()
        {
            m_SceneWindow->SetPosition (Gwk::Position::Center);
            m_SceneWindow->SetHidden (false);
            m_SceneWindow->MakeModal (true);
        }

        void OnSave ();
        void OnRemove ();
        void OnOutcome ();

        void OnAccept () { m_SceneWindow->CloseButtonPressed (); }

        void OnCancel () { m_SceneWindow->CloseButtonPressed (); }

        void OnSpeechSelect (Gwk::Controls::Base*);

        void OnNameEdit () { m_Editor->m_EditorSpeechMode.m_Speech.Name = m_NameTextBox->GetText (); }

        void OnLangSelected ()
        {
            Gwk::Controls::Label* selItem = m_LanguageCombo->GetSelectedItem ();

            if (selItem->GetText () == "EN")
            {
                m_LangIndex = 0;
            }
            else if (selItem->GetText () == "PL")
            {
                m_LangIndex = 1;
            }

            m_TextData->SetText (m_Editor->m_EditorSpeechMode.m_Speech.Text[m_LangIndex]);
        }

        void OnTextChanged () { m_Editor->m_EditorSpeechMode.m_Speech.Text[m_LangIndex] = m_TextData->GetText (); }

        void UpdateOutcomes ();
        void UpdateSpeechesTree ();

        void OnOutcomeIDTextChanged (Gwk::Controls::Base* control);
        void OnOutcomeDataTextChanged (Gwk::Controls::Base* control);
        void OnOutcomeActionChanged (Gwk::Controls::Base* control);
        void OnUpOutcome (Gwk::Controls::Base* control);
        void OnDownOutcome (Gwk::Controls::Base* control);
        void OnRemoveOutcome (Gwk::Controls::Base* control);

    private:
        Editor* m_Editor;

        int m_LangIndex;

        Gwk::Controls::WindowControl* m_SceneWindow;
        Gwk::Controls::TreeControl* m_SpeechesTree;
        Gwk::Controls::TextBox* m_NameTextBox;
        Gwk::Controls::ComboBox* m_LanguageCombo;
        Gwk::Controls::TextBoxMultiline* m_TextData;
        Gwk::Controls::ScrollControl* m_OutcomesContainer;
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
