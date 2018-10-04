// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "EditorActorWindow.h"
#include "ActorFactory.h"
#include "Atlas.h"
#include "AtlasManager.h"
#include "Component.h"
#include "Editor.h"
#include "EditorComponentWindow.h"
#include "EditorScriptWindow.h"
#include "MainLoop.h"
#include "Resources.h"
#include "Screen.h"
#include "actors/EnemyActor.h"
#include "actors/NPCActor.h"
#include "actors/TileActor.h"

#include "imgui.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    EditorActorWindow::EditorActorWindow (Editor* editor)
        : m_Editor (editor)
        , m_SelectedAtlas (nullptr)
        , m_SelectedAtlasRegion ("")
        , m_SelectedActor (nullptr)
        , m_IsVisible (false)
        , m_SelectedImage (0)
        , m_SelectedImagePath (0)
        , m_SelectedAnimation (0)
    {
    }

    //--------------------------------------------------------------------------------------------------

    EditorActorWindow::~EditorActorWindow () {}

    //--------------------------------------------------------------------------------------------------

    void EditorActorWindow::Show ()
    {
        m_IsVisible = true;

        memset (m_ActorID, 0, ARRAY_SIZE (m_ActorID));
        memset (m_ActorName, 0, ARRAY_SIZE (m_ActorName));

        memset (m_ActorPosition, 0, ARRAY_SIZE (m_ActorPosition));
        m_ActorRotation = 0.f;
        m_ActorZOrder = 0;

        memset (m_ActorAnimation, 0, ARRAY_SIZE (m_ActorAnimation));

        m_ActorCollidable = false;
        m_ActorCollision = false;
        m_ActorOverlap = false;
        m_ActorFocusHeight = -1;

        UpdateComboBoxes ();

        m_SelectedAtlas = nullptr;
        m_SelectedAtlasRegion = "";

        m_SelectedImage = 0;
        m_SelectedImagePath = 0;
        m_SelectedAnimation = 0;

        if (m_SelectedActor)
        {
            OnActorSelect (m_SelectedActor->ID);
        }
    }

    //--------------------------------------------------------------------------------------------------

    void EditorActorWindow::OnSave ()
    {
        if (m_ActorName[0] != '\0')
        {
            std::string actorName;

            if (m_ActorName[0] != '\0')
            {
                actorName = m_ActorName;
            }

            int id = -1;

            if (m_ActorID[0] != '\0')
            {
                id = ToInteger (m_ActorID);
            }

            int blueprintID = -1;

            if (m_SelectedBlueprint > 0)
            {
                blueprintID = ToInteger (m_Blueprints[m_SelectedBlueprint]);
            }

            Point position;

            sscanf (m_ActorPosition, "%.2f %.2f", &position.X, &position.Y);

            Actor* retActor
                = m_Editor->GetEditorActorMode ().AddOrUpdateActor (id, actorName, m_ActorTypes[m_SelectedActorType],
                    blueprintID, position, m_ActorRotation, m_ActorZOrder, m_ActorFocusHeight);

            if (m_SelectedAtlas && m_SelectedAtlasRegion != "")
            {
                retActor->Bounds.SetSize (m_SelectedAtlas->GetRegion (m_SelectedAtlasRegion).Bounds.Size);
                retActor->SetAtlas (m_SelectedAtlas);
                retActor->SetAtlasRegionName (m_SelectedAtlasRegion);
            }

            if (retActor)
            {
                m_Editor->GetEditorActorMode ().Clear ();

                UpdateComboBoxes ();
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    void EditorActorWindow::SelectActor (Actor* actor)
    {
        m_SelectedActor = actor;
        OnActorSelect (m_SelectedActor->ID);
    }

    //--------------------------------------------------------------------------------------------------

    void EditorActorWindow::UpdateImageCombos ()
    {
        //  Paths

        m_ImagePaths.clear ();

        for (ResourceID resID : GetGfxPacks ())
        {
            std::string name = GetBaseName (GetResource (resID).Name);
            m_ImagePaths.push_back (name);
        }

        m_SelectedAtlas = m_Editor->GetMainLoop ()->GetAtlasManager ().GetAtlas (m_ImagePaths[m_SelectedImagePath]);

        //  Images

        m_Images.clear ();
        m_Images.push_back ("");

        std::vector<AtlasRegion> regions = m_SelectedAtlas->GetRegions ();
        for (AtlasRegion region : regions)
        {
            m_Images.push_back (region.Name);
        }

        m_SelectedAtlasRegion = m_Images[m_SelectedImage];

        //  Animations

        m_Animations.clear ();

        m_Animations.push_back ("");

        for (AnimationData& data : g_AnimationData)
        {
            m_Animations.push_back (data.Name);
        }
    }

    //--------------------------------------------------------------------------------------------------

    void EditorActorWindow::UpdateComboBoxes ()
    {
        m_Blueprints.clear ();
        m_SelectedBlueprint = 0;

        std::vector<Actor*>& actors = m_Editor->GetMainLoop ()->GetSceneManager ().GetActiveScene ()->GetActors ();
        m_Blueprints.push_back ("[NONE]");

        for (Actor* actor : actors)
        {
            m_Blueprints.push_back (std::to_string (actor->ID));
        }

        m_ActorTypes.clear ();
        m_SelectedActorType = 0;

        std::vector<std::string>& actorTypes = ActorFactory::GetActorTypes ();

        for (const std::string& type : actorTypes)
        {
            m_ActorTypes.push_back (type);
        }
    }

    //--------------------------------------------------------------------------------------------------

    void EditorActorWindow::OnActorSelect (int id)
    {
        m_SelectedActor = nullptr;

        memset (m_ActorID, 0, ARRAY_SIZE (m_ActorID));
        memset (m_ActorName, 0, ARRAY_SIZE (m_ActorName));

        std::vector<Actor*>& actors = m_Editor->GetMainLoop ()->GetSceneManager ().GetActiveScene ()->GetActors ();

        for (Actor* actor : actors)
        {
            if (actor->ID == id)
            {
                m_SelectedActor = actor;

                break;
            }
        }

        strcpy (m_ActorID, ToString (m_SelectedActor->ID).c_str ());
        strcpy (m_ActorName, m_SelectedActor->Name.c_str ());

        std::vector<std::string>& actorTypes = ActorFactory::GetActorTypes ();
        for (int i = 0; i < actorTypes.size (); ++i)
        {
            if (m_SelectedActor->GetTypeName () == actorTypes[i])
            {
                m_SelectedActorType = i;
                break;
            }
        }

        m_SelectedBlueprint = 0;
        for (int i = 1; i < actors.size (); ++i)
        {
            if (m_SelectedActor->BlueprintID == actors[i]->ID)
            {
                m_SelectedBlueprint = i;
                break;
            }
        }

        sprintf (m_ActorPosition, "%.2f %.2f", m_SelectedActor->Bounds.Pos.X, m_SelectedActor->Bounds.Pos.Y);
        m_ActorRotation = m_SelectedActor->Rotation;
        m_ActorZOrder = m_SelectedActor->ZOrder;

        m_ActorFocusHeight = m_SelectedActor->GetFocusHeight ();
        m_ActorCollision = m_SelectedActor->IsCollisionEnabled ();
        m_ActorCollidable = m_SelectedActor->IsCollidable ();
        m_ActorOverlap = m_SelectedActor->IsCheckOverlap ();

        m_Editor->GetEditorActorMode ().SetSelectedActor (m_SelectedActor);
        m_Editor->GetEditorActorMode ().SetActor (m_SelectedActor);

        FillComponentsList ();

        m_SelectedAtlas
            = m_Editor->GetMainLoop ()->GetAtlasManager ().GetAtlas (m_SelectedActor->GetAtlas ()->GetName ());
        m_SelectedAtlasRegion = m_SelectedActor->GetAtlasRegionName ();

        std::vector<ResourceID> packs = GetGfxPacks ();
        for (int i = 0; i < packs.size (); ++i)
        {
            std::string name = GetBaseName (GetResource (packs[i]).Name);
            std::string path = m_SelectedActor->GetAtlas ()->GetName ();

            if (TrimString (path) == name)
            {
                m_SelectedImagePath = i;
                break;
            }
        }

        if (m_SelectedAtlas)
        {
            std::vector<AtlasRegion>& regions = m_SelectedAtlas->GetRegions ();
            for (int i = 0; i < regions.size (); ++i)
            {
                std::string name = regions[i].Name;

                if (m_SelectedAtlasRegion == name)
                {
                    //  First element is blank!
                    m_SelectedImage = i + 1;
                    break;
                }
            }
        }

        UpdateImageCombos ();
    }

    //--------------------------------------------------------------------------------------------------

    void EditorActorWindow::FillComponentsList ()
    {
        if (m_SelectedActor)
        {
            for (ScriptMetaData& scriptData : m_SelectedActor->GetScripts ())
            {
                AddScriptEntry (scriptData.Name, scriptData.Path);
            }

            std::map<std::string, Component*>& components = m_SelectedActor->GetComponents ();

            for (std::map<std::string, Component*>::iterator it = components.begin (); it != components.end (); ++it)
            {
                AddComponentEntry (it->first, it->second->GetTypeName ());
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    void EditorActorWindow::AddScriptEntry (const std::string& name, const std::string& path)
    {
        if (m_SelectedActor)
        {
            m_SelectedActor->AttachScript (name, path);
        }
    }

    //--------------------------------------------------------------------------------------------------

    void EditorActorWindow::OnAddScript ()
    {
        if (m_ActorName[0] != '\0')
        {
            m_Editor->GetScriptWindow ()->Show (
                [&](std::string name, std::string path) { AddScriptEntry (name, path); }, nullptr);
        }
        else
        {
            ImGui::OpenPopup ("Actor Window Alert");
        }
    }

    //--------------------------------------------------------------------------------------------------

    void EditorActorWindow::AddComponentEntry (const std::string& name, const std::string& typeName)
    {
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
        if (m_ActorName[0] != '\0')
        {
            m_Editor->GetComponentWindow ()->Show (
                [&](std::string name, std::string typeName) { AddComponentEntry (name, typeName); }, nullptr);
        }
        else
        {
            ImGui::OpenPopup ("Actor Window Alert");
        }
    }

    //--------------------------------------------------------------------------------------------------

    void EditorActorWindow::RenderUI ()
    {
        ImGui::SetNextWindowSize (ImVec2 (600, 550), ImGuiCond_Always);

        if (ImGui::BeginPopupModal ("Actor Editor", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            std::vector<Actor*>& actors = m_Editor->GetMainLoop ()->GetSceneManager ().GetActiveScene ()->GetActors ();

            ImGui::BeginChild (
                "Child1", ImVec2 (200, ImGui::GetWindowSize ().y - 50), false, ImGuiWindowFlags_HorizontalScrollbar);
            ImGui::BeginGroup ();
            {
                if (ImGui::TreeNodeEx ("NPC", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    for (Actor* actor : actors)
                    {
                        if (actor->GetTypeName () == NPCActor::TypeName)
                        {
                            std::string name
                                = actor->Name + std::string (" [") + std::to_string (actor->ID) + std::string ("]");

                            if (ImGui::Selectable (name.c_str (), false))
                            {
                                OnActorSelect (actor->ID);
                            }
                        }
                    }

                    ImGui::TreePop ();
                }

                if (ImGui::TreeNodeEx ("Enemy", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    for (Actor* actor : actors)
                    {
                        if (actor->GetTypeName () == EnemyActor::TypeName)
                        {
                            std::string name
                                = actor->Name + std::string (" [") + std::to_string (actor->ID) + std::string ("]");

                            if (ImGui::Selectable (name.c_str (), false))
                            {
                                OnActorSelect (actor->ID);
                            }
                        }
                    }

                    ImGui::TreePop ();
                }

                if (ImGui::TreeNodeEx ("Tiles", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    for (Actor* actor : actors)
                    {
                        if (actor->GetTypeName () == TileActor::TypeName)
                        {
                            std::string name
                                = actor->Name + std::string (" [") + std::to_string (actor->ID) + std::string ("]");

                            if (ImGui::Selectable (name.c_str (), false))
                            {
                                OnActorSelect (actor->ID);
                            }
                        }
                    }

                    ImGui::TreePop ();
                }
            }
            ImGui::EndGroup ();
            ImGui::EndChild ();

            ImGui::SameLine ();

            ImGui::BeginChild (
                "Child2", ImVec2 (250, ImGui::GetWindowSize ().y - 50), false, ImGuiWindowFlags_HorizontalScrollbar);
            ImGui::BeginGroup ();
            {
                int headerStyle = ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_DefaultOpen;

                if (ImGui::CollapsingHeader ("General", headerStyle))
                {
                    ImGui::Columns (2, "mycolumns1");

                    ImGui::Text ("ID");
                    ImGui::NextColumn ();
                    ImGui::PushItemWidth (120.f);
                    ImGui::Text (m_ActorID);
                    ImGui::PopItemWidth ();
                    ImGui::NextColumn ();

                    ImGui::Text ("Name");
                    ImGui::NextColumn ();
                    ImGui::PushItemWidth (120.f);
                    ImGui::InputText ("##actorName", m_ActorName, IM_ARRAYSIZE (m_ActorName));
                    ImGui::PopItemWidth ();
                    ImGui::NextColumn ();

                    ImGui::Text ("Type");
                    ImGui::NextColumn ();
                    ImGui::PushItemWidth (120.f);
                    ImGui::Combo ("##actorType", &m_SelectedActorType, m_ActorTypes);
                    ImGui::PopItemWidth ();
                    ImGui::NextColumn ();

                    ImGui::Text ("Blueprint");
                    ImGui::NextColumn ();
                    ImGui::PushItemWidth (120.f);
                    ImGui::Combo ("##actorBlueprint", &m_SelectedBlueprint, m_Blueprints);
                    ImGui::PopItemWidth ();

                    ImGui::Columns (1);
                }

                if (ImGui::CollapsingHeader ("Transform", headerStyle))
                {
                    ImGui::Columns (2, "mycolumns2");

                    ImGui::Text ("Position");
                    ImGui::NextColumn ();
                    ImGui::PushItemWidth (120.f);
                    ImGui::InputText ("##actorPosition", m_ActorPosition, IM_ARRAYSIZE (m_ActorPosition));
                    ImGui::PopItemWidth ();
                    ImGui::NextColumn ();

                    ImGui::Text ("Rotation");
                    ImGui::NextColumn ();
                    ImGui::PushItemWidth (120.f);
                    ImGui::InputFloat ("##actorRotation", &m_ActorRotation);
                    ImGui::PopItemWidth ();
                    ImGui::NextColumn ();

                    ImGui::Text ("ZOrder");
                    ImGui::NextColumn ();
                    ImGui::PushItemWidth (120.f);
                    ImGui::InputInt ("##actorZOrder", &m_ActorZOrder);
                    ImGui::PopItemWidth ();

                    ImGui::Columns (1);
                }

                if (ImGui::CollapsingHeader ("Apperance", headerStyle))
                {
                    ImGui::Columns (2, "mycolumns3");

                    ImGui::Text ("Path");
                    ImGui::NextColumn ();
                    ImGui::PushItemWidth (120.f);
                    if (ImGui::Combo ("##actorPath", &m_SelectedImagePath, m_ImagePaths))
                    {
                        UpdateImageCombos ();
                        m_SelectedImage = 0;
                    }
                    ImGui::PopItemWidth ();
                    ImGui::NextColumn ();

                    ImGui::Text ("Image");
                    ImGui::NextColumn ();
                    ImGui::PushItemWidth (120.f);
                    if (ImGui::Combo ("##actorImage", &m_SelectedImage, m_Images))
                    {
                        UpdateImageCombos ();
                    }
                    ImGui::PopItemWidth ();
                    ImGui::NextColumn ();

                    ImGui::Text ("Animation");
                    ImGui::NextColumn ();
                    ImGui::PushItemWidth (120.f);
                    ImGui::Combo ("##actorAnimation", &m_SelectedAnimation, m_Animations);
                    ImGui::PopItemWidth ();

                    ImGui::Columns (1);
                }

                if (ImGui::CollapsingHeader ("Others", headerStyle))
                {
                    ImGui::Columns (2, "mycolumns4");

                    ImGui::Text ("Focus Height");
                    ImGui::NextColumn ();
                    ImGui::PushItemWidth (120.f);
                    if (ImGui::InputFloat ("##actorFocusHeight", &m_ActorFocusHeight))
                    {
                        m_SelectedActor->SetFocusHeight (m_ActorFocusHeight);
                    }
                    ImGui::PopItemWidth ();
                    ImGui::NextColumn ();

                    ImGui::Text ("Collision");
                    ImGui::NextColumn ();
                    ImGui::PushItemWidth (120.f);
                    if (ImGui::Checkbox ("##actorCollision", &m_ActorCollision))
                    {
                        m_SelectedActor->SetCollisionEnabled (m_ActorCollision);
                    }
                    ImGui::PopItemWidth ();
                    ImGui::NextColumn ();

                    ImGui::Text ("Collidable");
                    ImGui::NextColumn ();
                    ImGui::PushItemWidth (120.f);
                    if (ImGui::Checkbox ("##actorCollidable", &m_ActorCollidable))
                    {
                        m_SelectedActor->SetCollidable (m_ActorCollidable);
                    }
                    ImGui::PopItemWidth ();
                    ImGui::NextColumn ();

                    ImGui::Text ("Overlap");
                    ImGui::NextColumn ();
                    ImGui::PushItemWidth (120.f);
                    if (ImGui::Checkbox ("##actorOverlap", &m_ActorOverlap))
                    {
                        m_SelectedActor->SetCheckOverlap (m_ActorOverlap);
                    }
                    ImGui::PopItemWidth ();
                }

                ImGui::Columns (1);

                if (ImGui::CollapsingHeader ("Scripts", headerStyle))
                {
                    ImGui::Columns (2, "mycolumns5");

                    if (m_SelectedActor)
                    {
                        std::vector<ScriptMetaData>& scripts = m_SelectedActor->GetScripts ();

                        for (ScriptMetaData& scriptData : scripts)
                        {
                            ImGui::Text (scriptData.Name.c_str ());
                            ImGui::NextColumn ();
                            ImGui::Text (scriptData.Path.c_str ());
                            ImGui::SameLine ();

                            if (ImGui::Button ("X"))
                            {
                                m_SelectedActor->RemoveScript (scriptData.Name);
                            }

                            ImGui::NextColumn ();
                        }
                    }
                }

                ImGui::Columns (1);

                if (ImGui::CollapsingHeader ("Components", headerStyle))
                {
                    ImGui::Columns (2, "mycolumns6");

                    if (m_SelectedActor)
                    {
                        std::map<std::string, Component*>& components = m_SelectedActor->GetComponents ();

                        for (std::map<std::string, Component*>::iterator it = components.begin ();
                             it != components.end (); ++it)
                        {
                            ImGui::Text (it->first.c_str ());
                            ImGui::NextColumn ();
                            ImGui::Text (it->second->GetTypeName ().c_str ());
                            ImGui::SameLine ();

                            if (ImGui::Button ("X"))
                            {
                                m_SelectedActor->RemoveComponent (it->first);
                            }

                            ImGui::NextColumn ();
                        }
                    }
                }
            }
            ImGui::EndGroup ();
            ImGui::EndChild ();

            ImGui::Columns (1);

            ImGui::SameLine ();

            ImGui::BeginGroup ();
            {
                ImVec2 buttonSize = ImVec2 (100.f, 18.f);

                if (ImGui::Button ("SAVE", buttonSize))
                {
                    OnSave ();
                }

                if (ImGui::Button ("REMOVE", buttonSize))
                {
                    m_Editor->GetEditorActorMode ().RemoveActor (atoi (m_ActorID));
                }

                ImGui::Separator ();

                if (ImGui::Button ("SCRIPTS", buttonSize))
                {
                    OnAddScript ();
                }

                m_Editor->GetScriptWindow ()->Render ();

                if (ImGui::Button ("COMPONENTS", buttonSize))
                {
                    OnAddComponent ();
                }

                m_Editor->GetComponentWindow ()->Render ();

                //  Alert window
                bool open = true;
                if (ImGui::BeginPopupModal ("Actor Window Alert", &open))
                {
                    ImGui::Text ("Select actor first!");
                    if (ImGui::Button ("OK", ImVec2 (130, 18)))
                    {
                        ImGui::CloseCurrentPopup ();
                    }

                    ImGui::EndPopup ();
                }
                //  Alert window

                ImGui::Separator ();

                if (ImGui::Button ("ACCEPT", buttonSize))
                {
                    ImGui::CloseCurrentPopup ();
                    m_IsVisible = false;
                }

                if (ImGui::Button ("CANCEL", buttonSize) || m_Editor->IsCloseCurrentPopup ())
                {
                    ImGui::CloseCurrentPopup ();
                    m_IsVisible = false;

                    m_Editor->SetCloseCurrentPopup (false);
                }
            }
            ImGui::EndGroup ();

            ImGui::EndPopup ();

            RenderActorImage ();
        }
    }

    //--------------------------------------------------------------------------------------------------

    void EditorActorWindow::RenderActorImage ()
    {
        const Point winSize = m_Editor->GetMainLoop ()->GetScreen ()->GetWindowSize ();

        const int margin = 10;
        const int previewSize = 96;

        al_draw_filled_rectangle (winSize.Width - previewSize - margin, winSize.Height - previewSize - margin * 6,
            winSize.Width - margin, winSize.Height - margin * 6, COLOR_BLACK);
        al_draw_rectangle (winSize.Width - previewSize - margin, winSize.Height - previewSize - margin * 6,
            winSize.Width - margin, winSize.Height - margin * 6, COLOR_GREEN, 2);

        if (m_SelectedAtlas && m_SelectedAtlasRegion != "")
        {
            AtlasRegion& region = m_SelectedAtlas->GetRegion (m_SelectedAtlasRegion);

            al_draw_scaled_bitmap (m_SelectedAtlas->GetImage (), region.Bounds.GetPos ().X, region.Bounds.GetPos ().Y,
                region.Bounds.GetSize ().Width, region.Bounds.GetSize ().Height, winSize.Width - previewSize - margin,
                winSize.Height - previewSize - margin * 6, previewSize, previewSize, 0);
        }
    }

    //--------------------------------------------------------------------------------------------------
}
