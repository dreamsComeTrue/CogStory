// Copyright 2017-2019 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "EditorActorWindow.h"
#include "ActorFactory.h"
#include "Component.h"
#include "Editor.h"
#include "EditorComponentWindow.h"
#include "EditorScriptWindow.h"
#include "EditorSpeechWindow.h"
#include "MainLoop.h"
#include "Player.h"
#include "Resources.h"
#include "Screen.h"
#include "actors/EnemyActor.h"
#include "actors/NPCActor.h"
#include "actors/TileActor.h"

#include "imgui.h"
#include "imgui_internal.h"

//--------------------------------------------------------------------------------------------------

namespace ImGui
{
	static auto vector_getter = [](void* vec, int idx, const char** out_text) {
		auto& vector = *static_cast<std::vector<std::string>*> (vec);
		if (idx < 0 || idx >= static_cast<int> (vector.size ()))
		{
			return false;
		}
		*out_text = vector.at (idx).c_str ();
		return true;
	};

	bool Combo (const char* label, int* currIndex, std::vector<std::string>& values)
	{
		if (values.empty ())
		{
			return false;
		}
		return Combo (label, currIndex, vector_getter, static_cast<void*> (&values), values.size ());
	}
}

//--------------------------------------------------------------------------------------------------

namespace aga
{
	//--------------------------------------------------------------------------------------------------

	EditorActorWindow::EditorActorWindow (Editor* editor)
		: m_Editor (editor)
		, m_IsVisible (false)
		, m_SelectedActor (nullptr)
		, m_SelectedAtlas (nullptr)
		, m_SelectedAtlasRegion ("")
		, m_SelectedImage (0)
		, m_SelectedImagePath (0)
		, m_SelectedAnimation (0)
		, m_SelectedSpeech (0)
		, m_ActionSpeechHandling (false)
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
		memset (m_ActorOverlapSize, 0, ARRAY_SIZE (m_ActorOverlapSize));
		memset (m_ActorRotation, 0, ARRAY_SIZE (m_ActorRotation));
		m_ActorZOrder = 0;

		memset (m_ActorAnimation, 0, ARRAY_SIZE (m_ActorAnimation));

		m_ActorCollidable = false;
		m_ActorCollision = false;
		m_ActorOverlap = false;

		memset (m_ActorFocusHeight, 0, ARRAY_SIZE (m_ActorFocusHeight));

		UpdateComboBoxes ();

		m_SelectedAtlas = nullptr;
		m_SelectedAtlasRegion = "";

		m_SelectedImage = 0;
		m_SelectedImagePath = 0;
		m_SelectedAnimation = 0;
		m_SelectedSpeech = 0;
		m_ActionSpeechHandling = false;

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

			std::vector<std::string> posParts = SplitString (m_ActorPosition, ' ');
			Point position (
				static_cast<float> (atof (posParts[0].c_str ())), static_cast<float> (atof (posParts[1].c_str ())));

			std::vector<std::string> sizeParts = SplitString (m_ActorOverlapSize, ' ');
			Point size (
				static_cast<float> (atof (sizeParts[0].c_str ())), static_cast<float> (atof (sizeParts[1].c_str ())));

			for (int i = 0; i < 20; ++i)
			{
				if (m_ActorFocusHeight[i] == ',')
				{
					m_ActorFocusHeight[i] = '.';
				}
			}

			float focusHeight = static_cast<float> (atof (m_ActorFocusHeight));

			Actor* retActor = m_Editor->GetEditorActorMode ().AddOrUpdateActor (id, m_ActorName,
				m_ActorTypes[m_SelectedActorType], blueprintID, position, size,
				static_cast<float> (atof (m_ActorRotation)), m_ActorZOrder, focusHeight);

			if (retActor)
			{
				if (m_SelectedAtlas && m_SelectedAtlasRegion != "")
				{
					retActor->Bounds.SetSize (m_SelectedAtlas->GetRegion (m_SelectedAtlasRegion).Bounds.Size);
					retActor->SetAtlas (m_SelectedAtlas);
					retActor->SetAtlasRegionName (m_SelectedAtlasRegion);
				}

				retActor->SetAnimation (ActorFactory::GetAnimation (m_Animations[m_SelectedAnimation]));
				retActor->SetCurrentAnimation (ANIM_IDLE_NAME);
				retActor->SetActionSpeech (m_Speeches[m_SelectedSpeech]);
				retActor->SetActionSpeechHandling (m_ActionSpeechHandling);

				Player* player = m_Editor->GetMainLoop ()->GetSceneManager ().GetPlayer ();
				player->RemoveActionSpeech (retActor);

				if (retActor->GetActionSpeech () != "")
				{
					player->RegisterActionSpeech (retActor, retActor->GetActionSpeech ());
				}
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

		std::map<std::string, Animation>& animations = ActorFactory::GetAnimations ();

		for (auto& kv : animations)
		{
			m_Animations.push_back (kv.first);
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

		UpdateSpeeches ();
	}

	//--------------------------------------------------------------------------------------------------

	void EditorActorWindow::UpdateSpeeches ()
	{
		m_Speeches.clear ();
		m_Speeches.push_back ("");

		std::map<long, SpeechData>& speeches
			= m_Editor->GetMainLoop ()->GetSceneManager ().GetActiveScene ()->GetSpeeches ();

		for (std::map<long, SpeechData>::iterator it = speeches.begin (); it != speeches.end (); ++it)
		{
			m_Speeches.push_back (it->second.Name);
		}
	}

	//--------------------------------------------------------------------------------------------------

	void EditorActorWindow::OnActorSelect (int id)
	{
		m_SelectedActor = nullptr;
		m_SelectedAtlas = nullptr;
		m_SelectedAtlasRegion = "";

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
		for (size_t i = 0; i < actorTypes.size (); ++i)
		{
			if (m_SelectedActor->GetTypeName () == actorTypes[i])
			{
				m_SelectedActorType = i;
				break;
			}
		}

		m_SelectedBlueprint = 0;
		for (size_t i = 1; i < actors.size (); ++i)
		{
			if (m_SelectedActor->BlueprintID == actors[i]->ID)
			{
				m_SelectedBlueprint = i;
				break;
			}
		}

		sprintf (m_ActorPosition, "%.2f %.2f", m_SelectedActor->Bounds.Pos.X, m_SelectedActor->Bounds.Pos.Y);
		sprintf (m_ActorOverlapSize, "%.2f %.2f", m_SelectedActor->OverlapSize.X, m_SelectedActor->OverlapSize.Y);
		sprintf (m_ActorRotation, "%.2f", m_SelectedActor->Rotation);
		m_ActorZOrder = m_SelectedActor->ZOrder;

		sprintf (m_ActorFocusHeight, "%.2f", m_SelectedActor->GetFocusHeight ());

		m_ActorCollision = m_SelectedActor->IsCollisionEnabled ();
		m_ActorCollidable = m_SelectedActor->IsCollidable ();
		m_ActorOverlap = m_SelectedActor->IsCheckOverlap ();

		m_Editor->GetEditorActorMode ().SetSelectedActor (m_SelectedActor);
		m_Editor->GetEditorActorMode ().SetActor (m_SelectedActor);

		FillComponentsList ();

		if (m_SelectedActor->GetAtlas ())
		{
			m_SelectedAtlas
				= m_Editor->GetMainLoop ()->GetAtlasManager ().GetAtlas (m_SelectedActor->GetAtlas ()->GetName ());
			m_SelectedAtlasRegion = m_SelectedActor->GetAtlasRegionName ();

			std::vector<ResourceID> packs = GetGfxPacks ();
			for (size_t i = 0; i < packs.size (); ++i)
			{
				std::string name = GetBaseName (GetResource (packs[i]).Name);
				std::string path = m_SelectedActor->GetAtlas ()->GetName ();

				if (TrimString (path) == name)
				{
					m_SelectedImagePath = i;
					break;
				}
			}
		}

		if (m_SelectedAtlas)
		{
			std::vector<AtlasRegion>& regions = m_SelectedAtlas->GetRegions ();
			for (size_t i = 0; i < regions.size (); ++i)
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

		for (size_t i = 0; i < m_Animations.size (); ++i)
		{
			if (m_SelectedActor->GetAnimation ().GetName () == m_Animations[i])
			{
				m_SelectedAnimation = i;
				break;
			}
		}

		for (size_t i = 0; i < m_Speeches.size (); ++i)
		{
			if (m_SelectedActor->GetActionSpeech () == m_Speeches[i])
			{
				m_SelectedSpeech = i;
				break;
			}
		}

		m_ActionSpeechHandling = m_SelectedActor->IsActionSpeechHandling ();

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
		Scene* scene = m_Editor->GetMainLoop ()->GetSceneManager ().GetActiveScene ();

		if (scene)
		{
			ImGui::SetNextWindowSize (ImVec2 (800, 650), ImGuiCond_Always);

			if (ImGui::BeginPopupModal ("Actor Editor", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
			{
				ImGui::BeginChild ("Child1", ImVec2 (240, ImGui::GetWindowSize ().y - 40), false,
					ImGuiWindowFlags_HorizontalScrollbar);
				ImGui::BeginGroup ();
				{
					if (ImGui::TreeNodeEx ("NPC", ImGuiTreeNodeFlags_DefaultOpen))
					{
						RenderActorGroup (NPCActor::TypeName);
						ImGui::TreePop ();
					}

					if (ImGui::TreeNodeEx ("Enemy", ImGuiTreeNodeFlags_DefaultOpen))
					{
						RenderActorGroup (EnemyActor::TypeName);
						ImGui::TreePop ();
					}

					if (ImGui::TreeNodeEx ("Tiles", ImGuiTreeNodeFlags_DefaultOpen))
					{
						RenderActorGroup (TileActor::TypeName);
						ImGui::TreePop ();
					}
				}
				ImGui::EndGroup ();
				ImGui::EndChild ();

				ImGui::SameLine ();

				ImGui::BeginChild ("Child2", ImVec2 (400, ImGui::GetWindowSize ().y - 40), false,
					ImGuiWindowFlags_HorizontalScrollbar);
				ImGui::BeginGroup ();
				{
					int headerStyle
						= ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_DefaultOpen;

					float controlWidth = 190.f;

					if (ImGui::CollapsingHeader ("General", headerStyle))
					{
						ImGui::Columns (2, "mycolumns1");

						ImGui::Text ("ID");
						ImGui::NextColumn ();
						ImGui::PushItemWidth (controlWidth);
						ImGui::Text (m_ActorID);
						ImGui::PopItemWidth ();
						ImGui::NextColumn ();

						ImGui::Text ("Name");
						ImGui::NextColumn ();
						ImGui::PushItemWidth (controlWidth);
						ImGui::InputText ("##actorName", m_ActorName, ARRAY_SIZE (m_ActorName));
						ImGui::PopItemWidth ();
						ImGui::NextColumn ();

						ImGui::Text ("Type");
						ImGui::NextColumn ();
						ImGui::PushItemWidth (controlWidth);
						ImGui::Combo ("##actorType", &m_SelectedActorType, m_ActorTypes);
						ImGui::PopItemWidth ();
						ImGui::NextColumn ();

						ImGui::Text ("Blueprint");
						ImGui::NextColumn ();
						ImGui::PushItemWidth (controlWidth);
						ImGui::Combo ("##actorBlueprint", &m_SelectedBlueprint, m_Blueprints);
						ImGui::PopItemWidth ();

						ImGui::Columns (1);
					}

					if (ImGui::CollapsingHeader ("Transform", headerStyle))
					{
						ImGui::Columns (2, "mycolumns2");

						ImGui::Text ("Position");
						ImGui::NextColumn ();
						ImGui::PushItemWidth (controlWidth);
						ImGui::InputText ("##actorPosition", m_ActorPosition, ARRAY_SIZE (m_ActorPosition));
						ImGui::PopItemWidth ();
						ImGui::NextColumn ();

						ImGui::Text ("Rotation");
						ImGui::NextColumn ();
						ImGui::PushItemWidth (controlWidth);
						ImGui::InputText ("##actorRotation", m_ActorRotation, ARRAY_SIZE (m_ActorRotation));
						ImGui::PopItemWidth ();
						ImGui::NextColumn ();

						ImGui::Text ("ZOrder");
						ImGui::NextColumn ();
						ImGui::PushItemWidth (controlWidth);
						ImGui::InputInt ("##actorZOrder", &m_ActorZOrder);
						ImGui::PopItemWidth ();

						ImGui::Columns (1);
					}

					if (ImGui::CollapsingHeader ("Apperance", headerStyle))
					{
						ImGui::Columns (2, "mycolumns3");

						ImGui::Text ("Path");
						ImGui::NextColumn ();
						ImGui::PushItemWidth (controlWidth);
						if (ImGui::Combo ("##actorPath", &m_SelectedImagePath, m_ImagePaths))
						{
							UpdateImageCombos ();
							m_SelectedImage = 0;
						}
						ImGui::PopItemWidth ();
						ImGui::NextColumn ();

						ImGui::Text ("Image");
						ImGui::NextColumn ();
						ImGui::PushItemWidth (controlWidth);
						if (ImGui::Combo ("##actorImage", &m_SelectedImage, m_Images))
						{
							UpdateImageCombos ();
						}
						ImGui::PopItemWidth ();
						ImGui::NextColumn ();

						ImGui::Text ("Animation");
						ImGui::NextColumn ();
						ImGui::PushItemWidth (controlWidth);
						ImGui::Combo ("##actorAnimation", &m_SelectedAnimation, m_Animations);
						ImGui::PopItemWidth ();

						ImGui::Columns (1);
					}

					if (ImGui::CollapsingHeader ("Others", headerStyle))
					{
						ImGui::Columns (2, "mycolumns4");

						ImGui::Text ("Focus Height");
						ImGui::NextColumn ();
						ImGui::PushItemWidth (controlWidth);
						ImGui::InputText ("##actorFocusHeight", m_ActorFocusHeight, ARRAY_SIZE (m_ActorFocusHeight));
						ImGui::PopItemWidth ();
						ImGui::NextColumn ();

						ImGui::Text ("Collision");
						ImGui::NextColumn ();
						ImGui::PushItemWidth (controlWidth);
						if (ImGui::Checkbox ("##actorCollision", &m_ActorCollision))
						{
							m_SelectedActor->SetCollisionEnabled (m_ActorCollision);
						}
						ImGui::PopItemWidth ();
						ImGui::NextColumn ();

						ImGui::Text ("Collidable");
						ImGui::NextColumn ();
						ImGui::PushItemWidth (controlWidth);
						if (ImGui::Checkbox ("##actorCollidable", &m_ActorCollidable))
						{
							m_SelectedActor->SetCollidable (m_ActorCollidable);
						}
						ImGui::PopItemWidth ();
						ImGui::NextColumn ();

						ImGui::Text ("Overlap");
						ImGui::NextColumn ();
						ImGui::PushItemWidth (controlWidth);
						if (ImGui::Checkbox ("##actorOverlap", &m_ActorOverlap))
						{
							m_SelectedActor->SetCheckOverlap (m_ActorOverlap);
						}
						ImGui::PopItemWidth ();
						ImGui::NextColumn ();

						ImGui::Text ("Overlap Size");
						ImGui::NextColumn ();
						ImGui::PushItemWidth (controlWidth);
						ImGui::InputText ("##actorOverlapSize", m_ActorOverlapSize, ARRAY_SIZE (m_ActorOverlapSize));
						ImGui::PopItemWidth ();
						ImGui::NextColumn ();

						ImGui::Text ("Action Speech");
						ImGui::NextColumn ();
						ImGui::PushItemWidth (controlWidth);
						ImGui::Combo ("##actionSpeech", &m_SelectedSpeech, m_Speeches);

						ImGui::PopItemWidth ();
						ImGui::NextColumn ();

						ImGui::Text ("Action Speech Handling");
						ImGui::NextColumn ();
						ImGui::PushItemWidth (controlWidth);
						ImGui::Checkbox ("##actionSpeechHandling", &m_ActionSpeechHandling);
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
					ImVec2 buttonSize = ImVec2 (120.f, 18.f);

					if (ImGui::Button ("SAVE", buttonSize))
					{
						OnSave ();
					}

					if (ImGui::Button ("REMOVE", buttonSize))
					{
						m_Editor->GetEditorActorMode ().RemoveActor (atoi (m_ActorID));
					}

					ImGui::NewLine ();

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

					if (ImGui::Button ("SPEECH", buttonSize))
					{
						m_Editor->GetSpeechWindow ()->Show ();

						ImGui::OpenPopup ("Speech Editor");
					}

					m_Editor->GetSpeechWindow ()->RenderUI ();

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

					ImGui::NewLine ();

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
			}
		}
	}

	//--------------------------------------------------------------------------------------------------

	bool SortByName (std::pair<int, std::string> a, std::pair<int, std::string> b)
	{
		if ((a.second != "" && b.second != "") && (a.second != b.second))
		{
			return a.second < b.second;
		}
		else
		{
			return a.first < b.first;
		}
	}

	//--------------------------------------------------------------------------------------------------

	void EditorActorWindow::RenderActorGroup (const std::string& groupName)
	{
		Scene* scene = m_Editor->GetMainLoop ()->GetSceneManager ().GetActiveScene ();
		std::vector<Actor*>& actors = scene->GetActors ();

		std::vector<std::pair<int, std::string>> items;

		for (Actor* actor : actors)
		{
			if (actor->GetTypeName () == groupName)
			{
				items.push_back (std::make_pair (actor->ID, actor->Name));
			}
		}

		std::sort (items.begin (), items.end (), SortByName);

		for (std::pair<int, std::string> item : items)
		{
			std::string name = item.second + std::string (" [") + std::to_string (item.first) + std::string ("]");

			if (ImGui::Selectable (name.c_str (), false))
			{
				OnActorSelect (item.first);
			}
		}
	}

	//--------------------------------------------------------------------------------------------------

	void EditorActorWindow::RenderActorImage ()
	{
		const Point winSize = m_Editor->GetMainLoop ()->GetScreen ()->GetRealWindowSize ();

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
				region.Bounds.GetSize ().Width, region.Bounds.GetSize ().Height, winSize.Width - previewSize - margin,
				winSize.Height - previewSize - margin, previewSize, previewSize, 0);
		}
	}

	//--------------------------------------------------------------------------------------------------

	bool EditorActorWindow::IsVisible () const { return m_IsVisible; }

	//--------------------------------------------------------------------------------------------------
}
