// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "EditorActorWindow.h"
#include "ActorFactory.h"
#include "Editor.h"
#include "EditorScriptWindow.h"
#include "EditorWindows.h"
#include "MainLoop.h"
#include "Screen.h"

namespace aga
{
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
        removeButton->SetPos (m_ActorProperties->Right () + 10, saveButton->Bottom () + 5);
        removeButton->onPress.Add (this, &EditorActorWindow::OnRemove);

        Gwk::Controls::Button* addScriptButton = new Gwk::Controls::Button (center);
        addScriptButton->SetText ("ADD SCRIPT");
        addScriptButton->SetPos (m_ActorProperties->Right () + 10, removeButton->Bottom () + 50);
        addScriptButton->onPress.Add (this, &EditorActorWindow::OnAddScript);

        Gwk::Controls::Button* acceptButton = new Gwk::Controls::Button (center);
        acceptButton->SetText ("ACCEPT");
        acceptButton->SetPos (m_ActorProperties->Right () + 10, m_SceneWindow->Height () - 85);
        acceptButton->onPress.Add (this, &EditorActorWindow::OnAccept);

        Gwk::Controls::Button* cacncelButton = new Gwk::Controls::Button (center);
        cacncelButton->SetText ("CANCEL");
        cacncelButton->SetPos (m_ActorProperties->Right () + 10, acceptButton->Bottom () + 5);
        cacncelButton->onPress.Add (this, &EditorActorWindow::OnCancel);

        UpdateActorsTree ();
    }

    //--------------------------------------------------------------------------------------------------

    EditorActorWindow::~EditorActorWindow () { SAFE_DELETE (m_SceneWindow); }

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

            bool ret
                = m_Editor->GetEditorActorMode ().AddOrUpdateActor (oldName, typePropertyTxt, m_Position, m_Rotation);

            if (ret)
            {
                m_Editor->GetEditorActorMode ().Clear ();

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
            m_Editor->GetEditorActorMode ().RemoveActor (nameProperty->GetPropertyValue ());
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

        std::vector<Actor*>& actors = m_Editor->GetMainLoop ()->GetSceneManager ().GetActiveScene ()->GetActors ();

        for (Actor* actor : actors)
        {
            Gwk::Controls::TreeNode* node = m_ActorsTree->AddNode (actor->Name);
            node->onSelect.Add (this, &EditorActorWindow::OnActorSelect);
        }

        m_ActorsTree->ExpandAll ();

        m_SelectedActor = nullptr;
    }

    //--------------------------------------------------------------------------------------------------

    void EditorActorWindow::OnActorSelect (Gwk::Controls::Base* control)
    {
        Gwk::Controls::TreeNode* node = (Gwk::Controls::TreeNode*)control;

        if (node != nullptr && node->IsSelected ())
        {
            std::vector<Actor*>& actors = m_Editor->GetMainLoop ()->GetSceneManager ().GetActiveScene ()->GetActors ();

            for (Actor* actor : actors)
            {
                if (actor->Name == node->GetText ())
                {
                    m_SelectedActor = actor;

                    m_Editor->GetEditorActorMode ().SetActor (actor);

                    idProperty->SetPropertyValue (ToString (actor->ID), false);
                    nameProperty->SetPropertyValue (actor->Name, false);
                    typeProperty->SetPropertyValue (actor->GetTypeName (), false);
                    positionProperty->SetPropertyValue (
                        Gwk::Utility::Format ("%f,%f", actor->Bounds.Pos.X, actor->Bounds.Pos.Y), false);
                    rotationProperty->SetPropertyValue (Gwk::Utility::Format ("%f", actor->Rotation), false);
                    zOrderProperty->SetPropertyValue (ToString (actor->ZOrder), false);

                    m_ScriptSection->Clear ();

                    for (ScriptMetaData& scriptData : actor->GetScripts ())
                    {
                        AddScriptEntry (scriptData.Name, scriptData.Path);
                    }

                    break;
                }
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    void EditorActorWindow::AddScriptEntry (const std::string& name, const std::string& path)
    {
        Gwk::Controls::Property::LabelButton* node
            = new Gwk::Controls::Property::LabelButton (m_ScriptSection, path, "X");
        node->FuncButton->onPress.Add (this, &EditorActorWindow::OnRemoveScript);

        m_ScriptSection->Add (name, node);

        if (m_SelectedActor)
        {
            m_SelectedActor->AttachScript (name, path);
        }
    }

    //--------------------------------------------------------------------------------------------------

    void EditorActorWindow::OnAddScript ()
    {
        EditorScriptWindow* scriptWindow = m_Editor->GetScriptWindow ();

        std::function<bool(void)> AcceptFunc = [&] {
            EditorScriptWindow* scriptWindow = m_Editor->GetScriptWindow ();

            if (scriptWindow->GetName () != "" && scriptWindow->GetPath () != "")
            {
                AddScriptEntry (scriptWindow->GetName (), scriptWindow->GetPath ());

                return true;
            }

            return false;
        };

        if (nameProperty != nullptr && nameProperty->GetPropertyValue () != "")
        {
            scriptWindow->Show (AcceptFunc, nullptr);
        }
        else
        {
            m_Editor->GetEditorInfoWindow ()->Show ("Select Actor first!");
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
            Gwk::Controls::Property::LabelButton* property
                = (Gwk::Controls::Property::LabelButton*)node->GetProperty ();

            if (property->FuncButton == button)
            {
                if (m_SelectedActor)
                {
                    m_SelectedActor->RemoveScript (node->GetLabel ()->GetText ());
                }

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
