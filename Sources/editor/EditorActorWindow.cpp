// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "EditorActorWindow.h"
#include "ActorFactory.h"
#include "Editor.h"
#include "EditorWindows.h"
#include "MainLoop.h"
#include "Screen.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    EditorActorScriptWindow::EditorActorScriptWindow (Editor* editor, Gwk::Controls::Canvas* canvas)
    {
        m_Result = true;
        m_Editor = editor;

        m_SceneWindow = new Gwk::Controls::WindowControl (canvas);
        m_SceneWindow->SetTitle ("Script");
        m_SceneWindow->SetSize (490, 120);
        m_SceneWindow->CloseButtonPressed ();

        Gwk::Controls::Label* nameLabel = new Gwk::Controls::Label (m_SceneWindow);
        nameLabel->SetBounds (20, 10, m_SceneWindow->Width () - 40, m_SceneWindow->Height () - 40);
        nameLabel->SetText ("Name:");
        nameLabel->SizeToContents ();

        Gwk::Controls::TextBox* nameTextBox = new Gwk::Controls::TextBox (m_SceneWindow);
        nameTextBox->SetText (Name);
        nameTextBox->SetTextColor (Gwk::Colors::White);
        nameTextBox->SetWidth (300);
        nameTextBox->SetPos (nameLabel->Right () + 10, nameLabel->Y ());
        nameTextBox->onTextChanged.Add (this, &EditorActorScriptWindow::OnNameEdit);

        Gwk::Controls::Label* pathLabel = new Gwk::Controls::Label (m_SceneWindow);
        pathLabel->SetPos (20, nameLabel->Bottom () + 10);
        pathLabel->SetText ("Path:");
        pathLabel->SizeToContents ();

        Gwk::Controls::TextBox* pathTextBox = new Gwk::Controls::TextBox (m_SceneWindow);
        pathTextBox->SetText (Path);
        pathTextBox->SetTextColor (Gwk::Colors::White);
        pathTextBox->SetWidth (300);
        pathTextBox->SetPos (pathLabel->Right () + 19, pathLabel->Y ());
        pathTextBox->onTextChanged.Add (this, &EditorActorScriptWindow::OnPathEdit);

        Gwk::Controls::Button* browseButton = new Gwk::Controls::Button (m_SceneWindow);
        browseButton->SetText ("BROWSE");
        browseButton->SetPos (pathTextBox->Right () + 5, pathLabel->Y ());
        browseButton->onPress.Add (this, &EditorActorScriptWindow::OnBrowse);

        Gwk::Controls::Button* yesButton = new Gwk::Controls::Button (m_SceneWindow);
        yesButton->SetText ("ACCEPT");
        yesButton->SetPos (m_SceneWindow->Width () / 2 - 2 * 50 - 5, m_SceneWindow->Height () - 60);
        yesButton->onPress.Add (this, &EditorActorScriptWindow::OnAccept);

        Gwk::Controls::Button* noButton = new Gwk::Controls::Button (m_SceneWindow);
        noButton->SetText ("CANCEL");
        noButton->SetPos (yesButton->Right () + 10, m_SceneWindow->Height () - 60);
        noButton->onPress.Add (this, &EditorActorScriptWindow::OnCancel);
    }

    //--------------------------------------------------------------------------------------------------

    void EditorActorScriptWindow::Show (std::function<bool(void)> OnAcceptFunc, std::function<bool(void)> OnCancelFunc)
    {
        m_OnAcceptFunc = OnAcceptFunc;
        m_OnCancelFunc = OnCancelFunc;

        m_SceneWindow->SetPosition (Gwk::Position::Center);
        m_SceneWindow->SetHidden (false);
        m_SceneWindow->MakeModal (true);
    }

    //--------------------------------------------------------------------------------------------------

    void EditorActorScriptWindow::OnBrowse ()
    {
        ALLEGRO_FILECHOOSER* fileOpenDialog = al_create_native_file_dialog (GetCurrentDir ().c_str (), "Open script file", "*.*", 0);
        al_show_native_file_dialog (m_Editor->m_MainLoop->GetScreen ()->GetDisplay (), fileOpenDialog);

        al_destroy_native_file_dialog (fileOpenDialog);
    }

    //--------------------------------------------------------------------------------------------------

    void EditorActorScriptWindow::OnAccept ()
    {
        m_Result = true;

        if (m_OnAcceptFunc)
        {
            if (!m_OnAcceptFunc ())
            {
                return;
            }
        }

        m_SceneWindow->CloseButtonPressed ();
    }

    //--------------------------------------------------------------------------------------------------

    void EditorActorScriptWindow::OnCancel ()
    {
        m_Result = false;

        if (m_OnCancelFunc)
        {
            if (!m_OnCancelFunc ())
            {
                return;
            }
        }

        m_SceneWindow->CloseButtonPressed ();
    }

    //--------------------------------------------------------------------------------------------------

    void EditorActorScriptWindow::OnNameEdit (Gwk::Controls::Base* control)
    {
        Gwk::Controls::TextBox* nameTextBox = (Gwk::Controls::TextBox*)control;

        Name = nameTextBox->GetText ();
    }

    //--------------------------------------------------------------------------------------------------

    void EditorActorScriptWindow::OnPathEdit (Gwk::Controls::Base* control)
    {
        Gwk::Controls::TextBox* pathTextBox = (Gwk::Controls::TextBox*)control;

        Path = pathTextBox->GetText ();
    }

    //--------------------------------------------------------------------------------------------------

    bool EditorActorScriptWindow::GetResult () { return m_Result; }

    //--------------------------------------------------------------------------------------------------
    //--------------------------------------------------------------------------------------------------

    Gwk::Controls::Property::Text* idProperty;
    Gwk::Controls::Property::Text* nameProperty;
    Gwk::Controls::Property::ComboBox* typeProperty;
    Gwk::Controls::Property::Text* positionProperty;
    Gwk::Controls::Property::Text* rotationProperty;
    Gwk::Controls::Property::Text* zOrderProperty;

    EditorActorWindow::EditorActorWindow (Editor* editor, Gwk::Controls::Canvas* canvas)
        : m_Editor (editor)
    {
        m_SelectedType = 0;

        m_SceneWindow = new Gwk::Controls::WindowControl (canvas);
        m_SceneWindow->SetTitle ("Actor Editor");
        m_SceneWindow->SetSize (680, canvas->Height () - 150);
        m_SceneWindow->CloseButtonPressed ();

        Gwk::Controls::DockBase* dock = new Gwk::Controls::DockBase (m_SceneWindow);
        dock->Dock (Gwk::Position::Fill);

        m_ActorsTree = new Gwk::Controls::TreeControl (dock);
        m_ActorsTree->SetBounds (10, 10, 300, dock->Height ());
        m_ActorsTree->ExpandAll ();
        m_ActorsTree->Dock (Gwk::Position::Fill);
        m_ActorsTree->onSelect.Add (this, &EditorActorWindow::OnActorSelect);

        dock->GetLeft ()->GetTabControl ()->AddPage ("Actors", m_ActorsTree);
        m_ActorsTree->SetMargin (Gwk::Margin ());
        dock->GetLeft ()->GetTabControl ()->GetTab (0)->Hide ();

        int xOffset = 10;

        Gwk::Controls::Base* center = new Gwk::Controls::Base (dock);
        center->Dock (Gwk::Position::Fill);

        Gwk::Controls::Label* nameLabel = new Gwk::Controls::Label (center);
        nameLabel->SetPos (xOffset, 10);
        nameLabel->SetText ("Name:");
        nameLabel->SizeToContents ();

        m_ActorProperties = new Gwk::Controls::PropertyTree (center);
        m_ActorProperties->SetBounds (10, nameLabel->Y (), 340, 400);
        {
            m_GeneralSection = m_ActorProperties->Add ("General");
            m_GeneralSection->Add ("ID");
            m_GeneralSection->Add ("Name");

            Gwk::Controls::Property::ComboBox* typeComboBox = new Gwk::Controls::Property::ComboBox (m_GeneralSection);

            std::vector<std::string>& actorTypes = ActorFactory::GetActorTypes ();

            for (const std::string& type : actorTypes)
            {
                typeComboBox->GetComboBox ()->AddItem (type, type);
            }

            m_GeneralSection->Add ("Type", typeComboBox);

            m_TransformSection = m_ActorProperties->Add ("Transform");

            m_TransformSection->Add ("Position");
            m_TransformSection->Add ("Rotation");
            m_TransformSection->Add ("ZOrder");

            m_ScriptSection = m_ActorProperties->Add ("Scripts");

            idProperty = static_cast<Gwk::Controls::Property::Text*> (
                static_cast<Gwk::Controls::PropertyRow*> (m_GeneralSection->Find ("ID"))->GetProperty ());
            idProperty->m_textBox->SetKeyboardInputEnabled (false);
            idProperty->m_textBox->SetMouseInputEnabled (false);

            nameProperty = static_cast<Gwk::Controls::Property::Text*> (
                static_cast<Gwk::Controls::PropertyRow*> (m_GeneralSection->Find ("Name"))->GetProperty ());
            typeProperty = static_cast<Gwk::Controls::Property::ComboBox*> (
                static_cast<Gwk::Controls::PropertyRow*> (m_GeneralSection->Find ("Type"))->GetProperty ());
            positionProperty = static_cast<Gwk::Controls::Property::Text*> (
                static_cast<Gwk::Controls::PropertyRow*> (m_TransformSection->Find ("Position"))->GetProperty ());
            rotationProperty = static_cast<Gwk::Controls::Property::Text*> (
                static_cast<Gwk::Controls::PropertyRow*> (m_TransformSection->Find ("Rotation"))->GetProperty ());
            zOrderProperty = static_cast<Gwk::Controls::Property::Text*> (
                static_cast<Gwk::Controls::PropertyRow*> (m_TransformSection->Find ("ZOrder"))->GetProperty ());
        }

        m_ActorProperties->ExpandAll ();

        Gwk::Controls::Button* saveButton = new Gwk::Controls::Button (center);
        saveButton->SetText ("SAVE");
        saveButton->SetPos (m_ActorProperties->Right () + 10, m_ActorProperties->Y ());
        saveButton->onPress.Add (this, &EditorActorWindow::OnSave);

        Gwk::Controls::Button* removeButton = new Gwk::Controls::Button (center);
        removeButton->SetText ("REMOVE");
        removeButton->SetPos (m_ActorProperties->Right () + 10, saveButton->Bottom () + 10);
        removeButton->onPress.Add (this, &EditorActorWindow::OnRemove);

        Gwk::Controls::Button* addScriptButton = new Gwk::Controls::Button (center);
        addScriptButton->SetText ("ADD SCRIPT");
        addScriptButton->SetPos (m_ActorProperties->Right () + 10, removeButton->Bottom () + 50);
        addScriptButton->onPress.Add (this, &EditorActorWindow::OnAddScript);

        Gwk::Controls::Button* acceptButton = new Gwk::Controls::Button (center);
        acceptButton->SetText ("ACCEPT");
        acceptButton->SetPos (m_ActorProperties->Right () + 10, m_SceneWindow->Height () - 65);
        acceptButton->onPress.Add (this, &EditorActorWindow::OnAccept);

        m_ScriptWindow = new EditorActorScriptWindow (m_Editor, canvas);

        UpdateActorsTree ();
    }

    //--------------------------------------------------------------------------------------------------

    void EditorActorWindow::Show ()
    {
        UpdateActorsTree ();

        idProperty->SetPropertyValue ("", false);
        nameProperty->SetPropertyValue ("", false);
        positionProperty->SetPropertyValue ("", false);
        rotationProperty->SetPropertyValue ("", false);
        zOrderProperty->SetPropertyValue ("", false);

        m_SceneWindow->SetPosition (Gwk::Position::Center);
        m_SceneWindow->SetHidden (false);
        m_SceneWindow->MakeModal (true);
    }

    //--------------------------------------------------------------------------------------------------

    void EditorActorWindow::OnSave ()
    {
        Gwk::String namePropertyTxt = nameProperty->GetPropertyValue ();
        Gwk::String typePropertyTxt = typeProperty->GetPropertyValue ();

        if (namePropertyTxt != "" && typePropertyTxt != "")
        {
            Gwk::Controls::Base::List& childNodes = m_ActorsTree->GetChildNodes ();
            std::string oldName;
            std::string selectedNodeName;

            for (Gwk::Controls::Base* control : childNodes)
            {
                Gwk::Controls::TreeNode* node = (Gwk::Controls::TreeNode*)control;

                if (node->IsSelected ())
                {
                    oldName = node->GetText ();
                    selectedNodeName = node->GetText ();
                    break;
                }
            }

            if (namePropertyTxt != "")
            {
                oldName = namePropertyTxt;
            }

            sscanf (positionProperty->GetPropertyValue ().c_str (), "%f,%f", &m_Position.X, &m_Position.Y);
            sscanf (rotationProperty->GetPropertyValue ().c_str (), "%f", &m_Rotation);

            bool ret = m_Editor->m_EditorActorMode.AddOrUpdateActor (oldName, typePropertyTxt, m_Position, m_Rotation);

            if (ret)
            {
                m_Editor->m_EditorActorMode.Clear ();

                UpdateActorsTree ();
                SelectActor (selectedNodeName);
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    void EditorActorWindow::OnRemove ()
    {
        if (nameProperty->GetPropertyValue () != "")
        {
            m_Editor->m_EditorActorMode.RemoveActor (nameProperty->GetPropertyValue ());
            nameProperty->SetPropertyValue ("", false);

            UpdateActorsTree ();
        }
    }

    //--------------------------------------------------------------------------------------------------

    void EditorActorWindow::SelectActor (const std::string& name)
    {
        Gwk::Controls::Base::List children = m_ActorsTree->GetChildNodes ();
        for (Gwk::Controls::Base* node : children)
        {
            Gwk::Controls::TreeNode* treeNode = static_cast<Gwk::Controls::TreeNode*> (node);

            if (treeNode->GetText () == name)
            {
                treeNode->SetSelected (true);
                break;
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    void EditorActorWindow::UpdateActorsTree ()
    {
        m_ActorsTree->Clear ();

        std::vector<Actor*>& actors = m_Editor->m_MainLoop->GetSceneManager ().GetActiveScene ()->GetActors ();

        for (Actor* actor : actors)
        {
            Gwk::Controls::TreeNode* node = m_ActorsTree->AddNode (actor->Name);
            node->onSelect.Add (this, &EditorActorWindow::OnActorSelect);
        }

        m_ActorsTree->ExpandAll ();
    }

    //--------------------------------------------------------------------------------------------------

    void EditorActorWindow::OnActorSelect (Gwk::Controls::Base* control)
    {
        Gwk::Controls::TreeNode* node = (Gwk::Controls::TreeNode*)control;

        if (node != nullptr && node->IsSelected ())
        {
            std::vector<Actor*>& actors = m_Editor->m_MainLoop->GetSceneManager ().GetActiveScene ()->GetActors ();

            for (Actor* actor : actors)
            {
                if (actor->Name == node->GetText ())
                {
                    m_Editor->m_EditorActorMode.m_Actor = actor;

                    idProperty->SetPropertyValue (ToString (actor->ID), false);
                    nameProperty->SetPropertyValue (actor->Name, false);
                    typeProperty->SetPropertyValue (actor->GetTypeName (), false);
                    positionProperty->SetPropertyValue (
                        Gwk::Utility::Format ("%f,%f", actor->Bounds.Pos.X, actor->Bounds.Pos.Y), false);
                    rotationProperty->SetPropertyValue (Gwk::Utility::Format ("%f", actor->Rotation), false);
                    zOrderProperty->SetPropertyValue (ToString (actor->ZOrder), false);

                    break;
                }
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    void EditorActorWindow::OnAddScript ()
    {
        std::function<bool(void)> AcceptFunc = [&] {
            if (m_ScriptWindow->Name != "" && m_ScriptWindow->Path != "")
            {
                Gwk::Controls::Property::LabelButton* node = new Gwk::Controls::Property::LabelButton (m_ScriptSection, m_ScriptWindow->Path, "X");
                node->FuncButton->onPress.Add (this, &EditorActorWindow::OnRemoveScript);

                m_ScriptSection->Add (m_ScriptWindow->Name, node);

                return true;
            }

            return false;
        };

        if (nameProperty != nullptr && nameProperty->GetPropertyValue () != "")
        {
            m_ScriptWindow->Show (AcceptFunc, nullptr);
        }
        else
        {
            m_Editor->m_InfoWindow->Show ("Select Actor first!");
        }
    }

    //--------------------------------------------------------------------------------------------------

    void EditorActorWindow::OnRemoveScript (Gwk::Controls::Base* control)
    {
        Gwk::Controls::Button* button = (Gwk::Controls::Button*)control;
        Gwk::Controls::Base::List& childNodes = m_ScriptSection->GetChildren ();

        for (Gwk::Controls::Base* control : childNodes)
        {
            Gwk::Controls::PropertyRow* node = (Gwk::Controls::PropertyRow*)control;
            Gwk::Controls::Property::LabelButton* property = (Gwk::Controls::Property::LabelButton*)node->GetProperty ();

            if (property->FuncButton == button)
            {
                m_ScriptSection->GetChildren ().remove (control);
                break;
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    void EditorActorWindow::OnAccept () { m_SceneWindow->CloseButtonPressed (); }

    //--------------------------------------------------------------------------------------------------

    void EditorActorWindow::OnCancel () { m_SceneWindow->CloseButtonPressed (); }

    //--------------------------------------------------------------------------------------------------
}
