// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "EditorSceneWindow.h"
#include "Editor.h"
#include "EditorScriptWindow.h"
#include "EditorWindows.h"
#include "MainLoop.h"
#include "Screen.h"
#include "Script.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    EditorSceneWindow::EditorSceneWindow (Editor* editor, Gwk::Controls::Canvas* canvas)
        : m_Editor (editor)
        , m_SelectedNode (nullptr)
    {
        m_SceneWindow = new Gwk::Controls::WindowControl (canvas);
        m_SceneWindow->SetTitle ("Scene Settings");
        m_SceneWindow->SetSize (630, 330);
        m_SceneWindow->CloseButtonPressed ();

        Gwk::Controls::Label* nameLabel = new Gwk::Controls::Label (m_SceneWindow);
        nameLabel->SetBounds (20, 10, m_SceneWindow->Width () - 40, m_SceneWindow->Height () - 40);
        nameLabel->SetText ("Name:");
        nameLabel->SizeToContents ();

        m_NameTextBox = new Gwk::Controls::TextBox (m_SceneWindow);
        m_NameTextBox->SetText ("");
        m_NameTextBox->SetTextColor (Gwk::Colors::White);
        m_NameTextBox->SetWidth (300);
        m_NameTextBox->SetPos (nameLabel->Right () + 10, nameLabel->Y ());
        m_NameTextBox->onTextChanged.Add (this, &EditorSceneWindow::OnNameEdit);

        Gwk::Controls::Label* colorLabel = new Gwk::Controls::Label (m_SceneWindow);
        colorLabel->SetPos (m_NameTextBox->Right () + 20, m_NameTextBox->Y ());
        colorLabel->SetText ("Color:");
        colorLabel->SizeToContents ();

        m_ColorPicker = new Gwk::Controls::ColorPicker (m_SceneWindow);
        m_ColorPicker->SetPos (colorLabel->X () - 10, colorLabel->Bottom () + 5);
        m_ColorPicker->onColorChanged.Add (this, &EditorSceneWindow::OnColorChanged);

        m_ScriptsBox = new Gwk::Controls::PropertyTree (m_SceneWindow);
        m_ScriptsBox->SetBounds (20, m_NameTextBox->Bottom () + 5, 350, 150);
        {
            m_ScriptSection = m_ScriptsBox->Add ("Scripts");
        }

        m_ScriptsBox->onSelect.Add (this, &EditorSceneWindow::OnScriptSelected);

        m_ScriptsBox->ExpandAll ();

        UpdateScriptsBox ();

        Gwk::Controls::Button* addScriptButton = new Gwk::Controls::Button (m_SceneWindow);
        addScriptButton->SetText ("ADD SCRIPT");
        addScriptButton->SetPos (m_ScriptsBox->X (), m_ScriptsBox->Bottom () + 5);
        addScriptButton->SetWidth (m_ScriptsBox->Width ());
        addScriptButton->onPress.Add (this, &EditorSceneWindow::OnAddScript);

        Gwk::Controls::Button* acceptButton = new Gwk::Controls::Button (m_SceneWindow);
        acceptButton->SetText ("ACCEPT");
        acceptButton->SetPos (m_SceneWindow->Width () - 130, m_SceneWindow->Height () - 90);
        acceptButton->onPress.Add (this, &EditorSceneWindow::OnAccept);

        Gwk::Controls::Button* cancelButton = new Gwk::Controls::Button (m_SceneWindow);
        cancelButton->SetText ("CANCEL");
        cancelButton->SetPos (acceptButton->X (), acceptButton->Bottom () + 5);
        cancelButton->onPress.Add (this, &EditorSceneWindow::OnCancel);
    }

    //--------------------------------------------------------------------------------------------------

    EditorSceneWindow ::~EditorSceneWindow () { SAFE_DELETE (m_SceneWindow); }

    //--------------------------------------------------------------------------------------------------

    void EditorSceneWindow::Show ()
    {
        m_SelectedNode = nullptr;

        m_SceneWindow->SetPosition (Gwk::Position::Center);
        m_SceneWindow->SetHidden (false);
        m_SceneWindow->MakeModal (true);

        m_NameTextBox->SetText (m_Editor->GetMainLoop ()->GetSceneManager ().GetActiveScene ()->GetName ());

        ALLEGRO_COLOR color = m_Editor->GetMainLoop ()->GetSceneManager ().GetActiveScene ()->GetBackgroundColor ();
        m_ColorPicker->SetColor (Gwk::Color (color.r * 255.f, color.g * 255.f, color.b * 255.f, color.a * 255.f));

        UpdateScriptsBox ();
    }

    //--------------------------------------------------------------------------------------------------

    void EditorSceneWindow::UpdateScriptsBox ()
    {
        m_ScriptSection->Clear ();

        std::vector<ScriptMetaData>& scripts
            = m_Editor->GetMainLoop ()->GetSceneManager ().GetActiveScene ()->GetScripts ();

        for (ScriptMetaData& script : scripts)
        {
            Gwk::Controls::Property::LabelTwoButton* node
                = new Gwk::Controls::Property::LabelTwoButton (m_ScriptSection, script.Path, "R", "X");
            node->FuncButton1->onPress.Add (this, &EditorSceneWindow::OnReloadScript);
            node->FuncButton2->onPress.Add (this, &EditorSceneWindow::OnRemoveScript);

            m_ScriptSection->Add (script.Name, node);
        }
    }

    //--------------------------------------------------------------------------------------------------

    void EditorSceneWindow::OnNameEdit () {}

    //--------------------------------------------------------------------------------------------------

    void EditorSceneWindow::OnAddScript ()
    {
        std::function<bool(void)> AcceptFunc = [&] {
            EditorScriptWindow* scriptWindow = m_Editor->GetScriptWindow ();

            if (scriptWindow->GetName () != "" && scriptWindow->GetPath () != "")
            {
                m_Editor->GetMainLoop ()->GetSceneManager ().GetActiveScene ()->AttachScript (
                    scriptWindow->GetName (), scriptWindow->GetPath ());
                UpdateScriptsBox ();

                return true;
            }

            return false;
        };

        m_Editor->GetScriptWindow ()->Show (AcceptFunc, nullptr);
    }

    //--------------------------------------------------------------------------------------------------

    void EditorSceneWindow::OnRemoveScript (Gwk::Controls::Base* control)
    {
        Gwk::Controls::Property::LabelTwoButton* property = GetScriptEntryByControl (control);

        if (property)
        {
            m_Editor->GetMainLoop ()->GetSceneManager ().GetActiveScene ()->RemoveScript (
                property->GetPropertyValue ());
            m_ScriptSection->GetChildren ().remove (control);
        }
    }

    //--------------------------------------------------------------------------------------------------

    void EditorSceneWindow::OnReloadScript (Gwk::Controls::Base* control)
    {
        Gwk::Controls::Property::LabelTwoButton* property = GetScriptEntryByControl (control);

        if (property)
        {
            Gwk::Controls::PropertyRow* node = (Gwk::Controls::PropertyRow*)property->GetParent ();
            Gwk::String selectedName = node->GetLabel ()->GetText ();

            Scene* scene = m_Editor->GetMainLoop ()->GetSceneManager ().GetActiveScene ();
            std::vector<ScriptMetaData>& scripts = scene->GetScripts ();

            for (ScriptMetaData& script : scripts)
            {
                if (script.Name == selectedName)
                {
                    scene->ReloadScript (script.Name);

                    if (strlen (g_ScriptErrorBuffer) != 0)
                    {
                        m_Editor->GetEditorInfoWindow ()->Show (g_ScriptErrorBuffer);
                        memset (g_ScriptErrorBuffer, 0, sizeof (g_ScriptErrorBuffer));
                    }
                    else
                    {
#ifdef _MSC_VER
                        std::optional<ScriptMetaData> metaScript = scene->GetScriptByName (script.Name);
#else
                        std::experimental::optional<ScriptMetaData> metaScript = scene->GetScriptByName (script.Name);
#endif

                        if (metaScript)
                        {
                            (*metaScript).ScriptObj->Run ("void Start ()");
                        }
                    }
                }
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    void EditorSceneWindow::OnColorChanged ()
    {
        Gwk::Color color = m_ColorPicker->GetColor ();
        m_Editor->GetMainLoop ()->GetSceneManager ().GetActiveScene ()->SetBackgroundColor (
            color.r / 255.f, color.g / 255.f, color.b / 255.f, color.a / 255.f);
    }

    //--------------------------------------------------------------------------------------------------

    void EditorSceneWindow::OnScriptSelected (Gwk::Controls::Base* control)
    {
        Gwk::Controls::TreeNode* node = (Gwk::Controls::TreeNode*)control;

        if (node != nullptr && node->IsSelected ())
        {
            m_SelectedNode = node;
        }
    }

    //--------------------------------------------------------------------------------------------------

    Gwk::Controls::Property::LabelTwoButton* EditorSceneWindow::GetScriptEntryByControl (Gwk::Controls::Base* control)
    {
        Gwk::Controls::Button* button = (Gwk::Controls::Button*)control;
        Gwk::Controls::Base::List& childNodes = m_ScriptSection->GetChildren ();

        for (Gwk::Controls::Base* control : childNodes)
        {
            Gwk::Controls::PropertyRow* node = (Gwk::Controls::PropertyRow*)control;
            Gwk::Controls::Property::LabelTwoButton* property
                = (Gwk::Controls::Property::LabelTwoButton*)node->GetProperty ();

            if (property->FuncButton1 == button || property->FuncButton2 == button)
            {
                return property;
            }
        }

        return nullptr;
    }

    //--------------------------------------------------------------------------------------------------

    void EditorSceneWindow::OnAccept ()
    {
        m_Editor->GetMainLoop ()->GetSceneManager ().GetActiveScene ()->SetName (m_NameTextBox->GetText ());

        m_SceneWindow->CloseButtonPressed ();
    }

    //--------------------------------------------------------------------------------------------------

    void EditorSceneWindow::OnCancel () { m_SceneWindow->CloseButtonPressed (); }

    //--------------------------------------------------------------------------------------------------
}
