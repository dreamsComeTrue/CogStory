// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "EditorActorWindow.h"
#include "ActorFactory.h"
#include "Atlas.h"
#include "AtlasManager.h"
#include "Editor.h"
#include "EditorScriptWindow.h"
#include "EditorComponentWindow.h"
#include "EditorWindows.h"
#include "MainLoop.h"
#include "Screen.h"
#include "actors/NPCActor.h"
#include "actors/EnemyActor.h"
#include "actors/TileActor.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    Gwk::Controls::Property::Text* idProperty;
    Gwk::Controls::Property::Text* nameProperty;
    Gwk::Controls::Property::ComboBox* typeProperty;
    Gwk::Controls::Property::Text* positionProperty;
    Gwk::Controls::Property::Text* rotationProperty;
    Gwk::Controls::Property::Text* zOrderProperty;
    Gwk::Controls::Property::ComboBox* imageProperty;
    Gwk::Controls::Property::ComboBox* imagePathProperty;

    EditorActorWindow::EditorActorWindow (Editor* editor, Gwk::Controls::Canvas* canvas)
        : m_Editor (editor)
        , m_SelectedAtlas (nullptr)
        , m_SelectedAtlasRegion ("")
        , m_SelectedType (0)
    {
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

            m_ApperanceSection = m_ActorProperties->Add ("Apperance");

            m_ImagePathComboBox = new Gwk::Controls::Property::ComboBox (m_ApperanceSection);

            for (ResourceID resID : GetGfxPacks ())
            {
                std::string name = GetBaseName (GetResource (resID).Name);
                m_ImagePathComboBox->GetComboBox ()->AddItem (name, name);
            }

            m_ApperanceSection->Add ("Path", m_ImagePathComboBox);

            m_ImageComboBox = new Gwk::Controls::Property::ComboBox (m_ApperanceSection);
            m_ImageComboBox->GetComboBox ()->onSelection.Add (this, &EditorActorWindow::OnImageSelected);

            m_ApperanceSection->Add ("Image", m_ImageComboBox);

            m_ImagePathComboBox->GetComboBox ()->onSelection.Add (this, &EditorActorWindow::OnImagePathSelected);
            OnImagePathSelected (m_ImagePathComboBox);

            m_ScriptSection = m_ActorProperties->Add ("Scripts");
            m_ComponentSection = m_ActorProperties->Add ("Components");

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
            imageProperty = static_cast<Gwk::Controls::Property::ComboBox*> (
                static_cast<Gwk::Controls::PropertyRow*> (m_ApperanceSection->Find ("Image"))->GetProperty ());
            imagePathProperty = static_cast<Gwk::Controls::Property::ComboBox*> (
                static_cast<Gwk::Controls::PropertyRow*> (m_ApperanceSection->Find ("Path"))->GetProperty ());
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

        Gwk::Controls::Button* addComponentButton = new Gwk::Controls::Button (center);
        addComponentButton->SetText ("COMPONENTS");
        addComponentButton->SetPos (m_ActorProperties->Right () + 10, addScriptButton->Bottom () + 5);
        addComponentButton->onPress.Add (this, &EditorActorWindow::OnAddComponent);

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
        imagePathProperty->SetPropertyValue ("", false);
        imageProperty->SetPropertyValue ("", false);

        m_SceneWindow->SetPosition (Gwk::Position::Center);
        m_SceneWindow->SetHidden (false);
        m_SceneWindow->MakeModal (true);

        m_SelectedAtlas = nullptr;
        m_SelectedAtlasRegion = "";
    }

    //--------------------------------------------------------------------------------------------------

    void EditorActorWindow::OnSave ()
    {
        Gwk::String namePropertyTxt = nameProperty->GetPropertyValue ();
        Gwk::String typePropertyTxt = typeProperty->GetPropertyValue ();

        if (namePropertyTxt != "" && typePropertyTxt != "")
        {
            Gwk::Controls::Base::List& childNodes = m_ActorsTree->GetChildNodes ();
            std::string actorName;
            std::string selectedNodeName;

            for (Gwk::Controls::Base* control : childNodes)
            {
                Gwk::Controls::TreeNode* node = (Gwk::Controls::TreeNode*)control;

                if (node->IsSelected ())
                {
                    actorName = node->GetText ();
                    selectedNodeName = node->GetText ();
                    break;
                }
            }

            if (namePropertyTxt != "")
            {
                actorName = namePropertyTxt;
            }

            Gwk::String idStr = idProperty->GetPropertyValue ();
            int id = -1;

            if (idStr != "")
            {
                id = ToInteger (idStr);
            }

            sscanf (positionProperty->GetPropertyValue ().c_str (), "%f,%f", &m_Position.X, &m_Position.Y);
            sscanf (rotationProperty->GetPropertyValue ().c_str (), "%f", &m_Rotation);
            sscanf (zOrderProperty->GetPropertyValue ().c_str (), "%d", &m_ZOrder);

            Actor* retActor = m_Editor->GetEditorActorMode ().AddOrUpdateActor (id, actorName, typePropertyTxt,
                                                                                m_Position, m_Rotation, m_ZOrder);

            if (m_SelectedAtlas && m_SelectedAtlasRegion != "")
            {
                retActor->Bounds.SetSize (m_SelectedAtlas->GetRegion (m_SelectedAtlasRegion).Bounds.Size);
                retActor->SetAtlas (m_SelectedAtlas);
                retActor->SetAtlasRegionName (m_SelectedAtlasRegion);
            }

            if (retActor)
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
        Gwk::Controls::TreeNode* node = FindNode (m_ActorsTree, name);

        if (node)
        {
            node->SetSelected (true);
        }
    }

    //--------------------------------------------------------------------------------------------------

    void EditorActorWindow::OnImagePathSelected (Gwk::Controls::Base* control)
    {
        Gwk::String packName = m_ImagePathComboBox->GetPropertyValue ();

        m_SelectedAtlas = m_Editor->GetMainLoop ()->GetAtlasManager ().GetAtlas (packName);
        m_SelectedAtlasRegion = "";

        std::vector<AtlasRegion> regions = m_SelectedAtlas->GetRegions ();

        m_ImageComboBox->GetComboBox ()->ClearItems ();
        m_ImageComboBox->GetComboBox ()->AddItem ("", "");

        for (AtlasRegion region : regions)
        {
            m_ImageComboBox->GetComboBox ()->AddItem (region.Name, region.Name);
        }

        m_ImageComboBox->GetComboBox ()->SelectItemByName ("", false);
    }

    //--------------------------------------------------------------------------------------------------

    void EditorActorWindow::OnImageSelected (Gwk::Controls::Base* control)
    {
        m_SelectedAtlasRegion = m_ImageComboBox->GetPropertyValue ();
    }

    //--------------------------------------------------------------------------------------------------

    Gwk::Controls::TreeNode* EditorActorWindow::FindNode (Gwk::Controls::TreeNode* node, const std::string& name)
    {
        Gwk::Controls::Base::List children = node->GetChildNodes ();
        for (Gwk::Controls::Base* n : children)
        {
            Gwk::Controls::TreeNode* childNode = static_cast<Gwk::Controls::TreeNode*> (n);

            if (childNode->GetText () == name)
            {
                return childNode;
            }

            Gwk::Controls::TreeNode* foundNode = FindNode (childNode, name);

            if (foundNode)
            {
                return foundNode;
            }
        }

        return nullptr;
    }

    //--------------------------------------------------------------------------------------------------

    void EditorActorWindow::UpdateActorsTree ()
    {
        m_ActorsTree->Clear ();

        Gwk::Controls::TreeNode* actorsNode = m_ActorsTree->AddNode ("Actors");
        actorsNode->onSelect.Add (this, &EditorActorWindow::OnActorSelect);

        Gwk::Controls::TreeNode* npcNode = actorsNode->AddNode ("NPC");
        npcNode->onSelect.Add (this, &EditorActorWindow::OnActorSelect);

        Gwk::Controls::TreeNode* enemyNode = actorsNode->AddNode ("Enemy");
        enemyNode->onSelect.Add (this, &EditorActorWindow::OnActorSelect);

        Gwk::Controls::TreeNode* tilesNode = m_ActorsTree->AddNode ("Tiles");
        tilesNode->onSelect.Add (this, &EditorActorWindow::OnActorSelect);

        std::vector<Actor*>& actors = m_Editor->GetMainLoop ()->GetSceneManager ().GetActiveScene ()->GetActors ();

        for (Actor* actor : actors)
        {
            Gwk::Controls::TreeNode* node;

            std::string name = actor->Name + std::string (" [") + ToString (actor->ID) + std::string ("]");

            if (actor->GetTypeName () == NPCActor::TypeName)
            {
                node = npcNode->AddNode (name);
            }
            else if (actor->GetTypeName () == EnemyActor::TypeName)
            {
                node = enemyNode->AddNode (name);
            }
            else if (actor->GetTypeName () == TileActor::TypeName)
            {
                node = tilesNode->AddNode (name);
            }

            if (node)
            {
                node->onSelect.Add (this, &EditorActorWindow::OnActorSelect);
            }
        }

        m_ActorsTree->ExpandAll ();

        m_SelectedActor = nullptr;
    }

    //--------------------------------------------------------------------------------------------------

    std::string EditorActorWindow::GetImagePath (Actor* actor)
    {
        std::string imagePath = "";

        if (actor->GetTypeName () == TileActor::TypeName)
        {
            imagePath = static_cast<TileActor*> (actor)->Tileset;
        }
        else if (actor->GetTypeName () == NPCActor::TypeName)
        {
            imagePath = actor->GetAtlas ()->GetName ();
        }
        else if (actor->GetTypeName () == EnemyActor::TypeName)
        {
            imagePath = actor->GetAtlas ()->GetName ();
        }

        return imagePath;
    }

    //--------------------------------------------------------------------------------------------------

    void EditorActorWindow::OnActorSelect (Gwk::Controls::Base* control)
    {
        Gwk::Controls::TreeNode* node = (Gwk::Controls::TreeNode*)control;

        Actor* selectedActor = nullptr;

        if (node != nullptr && node->IsSelected ())
        {
            std::vector<Actor*>& actors = m_Editor->GetMainLoop ()->GetSceneManager ().GetActiveScene ()->GetActors ();

            for (Actor* actor : actors)
            {
                std::string name = actor->Name + std::string (" [") + ToString (actor->ID) + std::string ("]");

                if (name == node->GetText ())
                {
                    selectedActor = actor;

                    break;
                }
            }
        }

        m_SelectedActor = selectedActor;

        m_Editor->GetEditorActorMode ().SetSelectedActor (selectedActor);
        m_Editor->GetEditorActorMode ().SetActor (selectedActor);

        idProperty->SetPropertyValue (selectedActor != nullptr ? ToString (selectedActor->ID) : "", false);
        nameProperty->SetPropertyValue (selectedActor != nullptr ? selectedActor->Name : "", false);
        typeProperty->SetPropertyValue (selectedActor != nullptr ? selectedActor->GetTypeName () : "", false);
        positionProperty->SetPropertyValue (
            selectedActor != nullptr
                ? Gwk::Utility::Format ("%f,%f", selectedActor->Bounds.Pos.X, selectedActor->Bounds.Pos.Y)
                : Gwk::Utility::Format ("%f,%f", 0.f, 0.f),
            false);
        rotationProperty->SetPropertyValue (selectedActor != nullptr
                                                ? Gwk::Utility::Format ("%f", selectedActor->Rotation)
                                                : Gwk::Utility::Format ("%f", 0.f),
                                            false);
        zOrderProperty->SetPropertyValue (selectedActor != nullptr ? ToString (selectedActor->ZOrder) : "0", false);
        imagePathProperty->SetPropertyValue (selectedActor != nullptr ? GetImagePath (selectedActor) : "", false);

        m_ScriptSection->Clear ();
        m_ComponentSection->Clear ();

        if (selectedActor)
        {
            for (ScriptMetaData& scriptData : selectedActor->GetScripts ())
            {
                AddScriptEntry (scriptData.Name, scriptData.Path);
            }

            std::map<std::string, Component*>& components = selectedActor->GetComponents ();

            for (std::map<std::string, Component*>::iterator it = components.begin (); it != components.end (); ++it)
            {
                AddComponentEntry (it->first, "MovementComponent");
            }
        }

        OnImagePathSelected (m_ImagePathComboBox);

        imageProperty->SetPropertyValue (selectedActor != nullptr ? selectedActor->GetAtlasRegionName () : "", false);

        m_SelectedAtlas
            = m_Editor->GetMainLoop ()->GetAtlasManager ().GetAtlas (imagePathProperty->GetPropertyValue ());
        m_SelectedAtlasRegion = imageProperty->GetPropertyValue ();
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

    void EditorActorWindow::AddComponentEntry (const std::string& name, const std::string& typeName)
    {
        Gwk::Controls::Property::LabelButton* node
            = new Gwk::Controls::Property::LabelButton (m_ComponentSection, typeName, "X");
        node->TextLabel->SetMouseInputEnabled (false);
        node->TextLabel->SetKeyboardInputEnabled (false);
        node->FuncButton->onPress.Add (this, &EditorActorWindow::OnRemoveComponent);

        m_ComponentSection->Add (name, node);

        if (m_SelectedActor)
        {
            Component* component = ActorFactory::GetActorComponent (m_SelectedActor, typeName);

            if (component)
            {
                m_SelectedActor->AddComponent (name, component);
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    void EditorActorWindow::OnAddComponent ()
    {
        EditorComponentWindow* componentWindow = m_Editor->GetComponentWindow ();

        std::function<bool(void)> AcceptFunc = [&] {
            EditorComponentWindow* componentWindow = m_Editor->GetComponentWindow ();

            if (componentWindow->GetName () != "" && componentWindow->GetTypeName () != "")
            {
                AddComponentEntry (componentWindow->GetName (), componentWindow->GetTypeName ());

                return true;
            }

            return false;
        };

        if (nameProperty != nullptr && nameProperty->GetPropertyValue () != "")
        {
            componentWindow->Show (AcceptFunc, nullptr);
        }
        else
        {
            m_Editor->GetEditorInfoWindow ()->Show ("Select Actor first!");
        }
    }

    //--------------------------------------------------------------------------------------------------

    void EditorActorWindow::OnRemoveComponent (Gwk::Controls::Base* control)
    {
        Gwk::Controls::Button* button = (Gwk::Controls::Button*)control;
        Gwk::Controls::Base::List& childNodes = m_ComponentSection->GetChildren ();

        for (Gwk::Controls::Base* control : childNodes)
        {
            Gwk::Controls::PropertyRow* node = (Gwk::Controls::PropertyRow*)control;
            Gwk::Controls::Property::LabelButton* property
                = (Gwk::Controls::Property::LabelButton*)node->GetProperty ();

            if (property->FuncButton == button)
            {
                if (m_SelectedActor)
                {
                    m_SelectedActor->RemoveComponent (node->GetLabel ()->GetText ());
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

    void EditorActorWindow::RenderActorImage ()
    {
        const Point winSize = m_Editor->GetMainLoop ()->GetScreen ()->GetWindowSize ();

        const int margin = 10;
        const int previewSize = 96;

        al_draw_filled_rectangle (winSize.Width - previewSize - margin, winSize.Height - previewSize - margin,
                                  winSize.Width - margin, winSize.Height - margin, COLOR_BLACK);
        al_draw_rectangle (winSize.Width - previewSize - margin, winSize.Height - previewSize - margin,
                           winSize.Width - margin, winSize.Height - margin, COLOR_GREEN, 2);

        if (m_SelectedAtlas && m_SelectedAtlasRegion != "")
        {
            AtlasRegion& region = m_SelectedAtlas->GetRegion (m_SelectedAtlasRegion);

            al_draw_scaled_bitmap (m_SelectedAtlas->GetImage (), region.Bounds.GetPos ().X, region.Bounds.GetPos ().Y,
                                   region.Bounds.GetSize ().Width, region.Bounds.GetSize ().Height,
                                   winSize.Width - previewSize - margin, winSize.Height - previewSize - margin,
                                   previewSize, previewSize, 0);
        }
    }

    //--------------------------------------------------------------------------------------------------
}
