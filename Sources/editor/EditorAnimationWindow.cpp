// Copyright 2017-2019 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "EditorAnimationWindow.h"
#include "ActorFactory.h"
#include "Editor.h"
#include "MainLoop.h"
#include "Resources.h"
#include "Screen.h"

#include "imgui.h"

using json = nlohmann::json;

namespace aga
{
	//--------------------------------------------------------------------------------------------------

	EditorAnimationWindow::EditorAnimationWindow (Editor* editor)
		: m_Editor (editor)
		, m_SelectedAtlas (nullptr)
		, m_SelectedAtlasRegion ("")
		, m_SelectedAnimationGroupIndex (0)
		, m_SelectedName (0)
		, m_SelectedImage (0)
		, m_SelectedImagePath (0)
		, m_AnimSpeed (100)
		, m_IsVisible (false)
		, m_CurrentFrameIndex (0)
		, m_FrameTimeLeft (0.f)
		, m_CellX (0)
		, m_CellY (0)
	{
	}

	//--------------------------------------------------------------------------------------------------

	EditorAnimationWindow ::~EditorAnimationWindow () {}

	//--------------------------------------------------------------------------------------------------

	void EditorAnimationWindow::Show (std::function<void(std::string, std::string)> OnAcceptFunc,
		std::function<void(std::string, std::string)> OnCancelFunc)
	{
		UpdateImageCombos ();

		m_OnAcceptFunc = OnAcceptFunc;
		m_OnCancelFunc = OnCancelFunc;

		m_IsVisible = true;

		UpdateAnimations ();
		UpdateNames ();
		ClearInputs ();
	}

	//--------------------------------------------------------------------------------------------------

	void EditorAnimationWindow::OnSave ()
	{
		std::map<std::string, AnimationData>& animations = m_Animation.GetAnimations ();
		bool found = false;

		for (auto& kv : animations)
		{
			if (kv.first == m_AnimationName)
			{
				found = true;
				break;
			}
		}

		AnimationData frames;
		frames.SetName (m_AnimationName);
		frames.SetPlaySpeed (m_AnimSpeed);

		for (AnimationFrameEntry& entry : m_Frames)
		{
			frames.AddFrame (entry);
		}

		if (!found)
		{
			m_Animation.AddAnimationData (m_AnimationName, frames);
		}
		else
		{
			m_Animation.SetAnimationData (m_AnimationName, frames);
		}

		SaveAnimToFile ();
	}

	//--------------------------------------------------------------------------------------------------

	void EditorAnimationWindow::OnDelete ()
	{
		m_Animation.RemoveAnimation (m_AnimationName);
		SaveAnimToFile ();
		ClearInputs ();
	}

	//--------------------------------------------------------------------------------------------------

	void EditorAnimationWindow::SaveAnimToFile ()
	{
		std::string animName = m_AnimationGroup;

		// Save to file
		if (animName != "")
		{
			json j;

			j["animation"] = m_Animation.GetName ();
			j["animations"] = json::array ({});

			std::map<std::string, AnimationData>& animations = m_Animation.GetAnimations ();

			for (auto& kv : animations)
			{
				if (kv.first == "")
				{
					continue;
				}

				json animObj = json::object ({});

				animObj["name"] = kv.first;
				animObj["speed"] = kv.second.GetPlaySpeed ();

				{
					std::vector<AnimationFrameEntry>& frames = kv.second.GetFrames ();

					for (AnimationFrameEntry& frame : frames)
					{
						json frameObj = json::object ({});
						frameObj["atlas"] = frame.Atlas;
						frameObj["region"] = frame.AtlasRegion;

						std::vector<Point> bounds;
						bounds.push_back (frame.Bounds.Pos);
						bounds.push_back (frame.Bounds.Size);

						frameObj["bounds"] = VectorPointsToString (bounds);

						animObj["frames"].push_back (frameObj);
					}
				}

				j["animations"].push_back (animObj);
			}

			const std::string animPath = GetDataPath () + "/animations/";

			if (!EndsWith (animName, ".anim"))
			{
				animName += ".anim";
			}

			std::transform (animName.begin (), animName.end (), animName.begin (), ::tolower);

			std::ofstream out (animPath + animName);
			out << std::setw (4) << j.dump (4, ' ') << "\n";
			out.close ();

			ActorFactory::RegisterAnimations ();

			UpdateSceneActorsAnimation ();
		}
	}

	//--------------------------------------------------------------------------------------------------

	void EditorAnimationWindow::UpdateSceneActorsAnimation ()
	{
		std::vector<Actor*>& actors = m_Editor->GetMainLoop ()->GetSceneManager ().GetActiveScene ()->GetActors ();

		for (Actor* actor : actors)
		{
			if (actor->GetAnimation ().GetName () == m_AnimationGroup)
			{
				actor->SetAnimation (ActorFactory::GetAnimation (m_AnimationGroup));
				actor->GetAnimation ().SetCurrentAnimation (ANIM_IDLE_NAME);
			}
		}
	}

	//--------------------------------------------------------------------------------------------------

	void EditorAnimationWindow::OnAccept ()
	{
		if (m_OnAcceptFunc)
		{
			m_OnAcceptFunc (m_Images[m_SelectedImage], m_ImagePaths[m_SelectedImagePath]);
		}
	}

	//--------------------------------------------------------------------------------------------------

	void EditorAnimationWindow::OnCancel ()
	{
		if (m_OnCancelFunc)
		{
			m_OnAcceptFunc (m_Images[m_SelectedImage], m_ImagePaths[m_SelectedImagePath]);
		}
	}

	//--------------------------------------------------------------------------------------------------

	void EditorAnimationWindow::Render ()
	{
		const Point winSize = m_Editor->GetMainLoop ()->GetScreen ()->GetRealWindowSize ();

		ImGui::SetNextWindowPos (ImVec2 (0, 0));
		ImGui::SetNextWindowSize (ImVec2 (320, winSize.Height - 220), ImGuiCond_Always);

		if (ImGui::BeginPopupModal ("Animations", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			float controlWidth = 200.f;

			ImGui::Text ("Animations");
			ImGui::SameLine ();
			ImGui::PushItemWidth (controlWidth);
			if (ImGui::Combo ("##animations", &m_SelectedAnimationGroupIndex, m_Animations))
			{
				strcpy (m_AnimationGroup, m_Animations[m_SelectedAnimationGroupIndex].c_str ());
				UpdateNames ();
			}
			ImGui::PopItemWidth ();

			ImGui::Text ("     Names");
			ImGui::SameLine ();
			ImGui::PushItemWidth (controlWidth);
			if (ImGui::Combo ("##names", &m_SelectedName, m_Names))
			{
				strcpy (m_AnimationName, m_Names[m_SelectedName].c_str ());

				AnimationData& animData = m_Animation.GetAnimationData (m_AnimationName);

				m_AnimSpeed = animData.GetPlaySpeed ();
				m_Frames = animData.GetFrames ();
			}
			ImGui::PopItemWidth ();

			ImGui::Separator ();

			ImGui::Text (" Animation");
			ImGui::SameLine ();
			ImGui::PushItemWidth (controlWidth);

			if (ImGui::InputText ("##anim", m_AnimationGroup, ARRAY_SIZE (m_AnimationGroup)))
			{
				m_Animation.SetName (m_AnimationGroup);
			}

			ImGui::PopItemWidth ();

			ImGui::Text ("      Name");
			ImGui::SameLine ();
			ImGui::PushItemWidth (controlWidth);
			ImGui::InputText ("##name", m_AnimationName, ARRAY_SIZE (m_AnimationName));
			ImGui::PopItemWidth ();

			ImGui::Text ("      Path");
			ImGui::SameLine ();
			ImGui::PushItemWidth (controlWidth);
			if (ImGui::Combo ("##animPath", &m_SelectedImagePath, m_ImagePaths))
			{
				UpdateImageCombos ();
				m_SelectedImage = 0;
			}
			ImGui::PopItemWidth ();

			ImGui::Text ("     Image");
			ImGui::SameLine ();
			ImGui::PushItemWidth (controlWidth);
			if (ImGui::Combo ("##animImage", &m_SelectedImage, m_Images))
			{
				UpdateImageCombos ();
			}

			ImGui::PopItemWidth ();

			ImGui::Text ("     Speed");
			ImGui::SameLine ();
			ImGui::PushItemWidth (controlWidth);

			if (ImGui::InputInt ("##animSpeed", &m_AnimSpeed))
			{
				m_Animation.GetAnimationData (m_AnimationName).SetPlaySpeed (m_AnimSpeed);
			}

			ImGui::PopItemWidth ();

			ImGui::Text ("CellX");
			ImGui::SameLine ();
			ImGui::PushItemWidth (controlWidth / 2 - 10);
			ImGui::InputInt ("##cellX", &m_CellX);
			ImGui::PopItemWidth ();
			ImGui::SameLine ();
			ImGui::Text ("CellY");
			ImGui::SameLine ();
			ImGui::PushItemWidth (controlWidth / 2 - 10);
			ImGui::InputInt ("##cellY", &m_CellY);
			ImGui::PopItemWidth ();

			int headerStyle = ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_DefaultOpen;
			if (ImGui::CollapsingHeader ("Frames", headerStyle))
			{
				ImGui::Columns (3, "mycolumns2");

				for (size_t i = 0; i < m_Frames.size (); ++i)
				{
					AnimationFrameEntry frame = m_Frames[i];

					ImGui::SetColumnWidth (-1, 100.f);
					ImGui::Text (frame.Atlas.c_str ());
					ImGui::NextColumn ();
					ImGui::SetColumnWidth (-1, 120.f);
					ImGui::Text (frame.AtlasRegion.c_str ());
					ImGui::NextColumn ();

					if (ImGui::Button ((std::string ("X##animButton") + std::to_string (i)).c_str ()))
					{
						m_Frames.erase (m_Frames.begin () + i);
						break;
					}

					ImGui::SameLine ();

					if (ImGui::Button ((std::string ("^##animButton") + std::to_string (i)).c_str ()))
					{
						if (i == 0)
						{
							AnimationFrameEntry tmp = m_Frames[0];
							m_Frames[0] = m_Frames[m_Frames.size () - 1];
							m_Frames[m_Frames.size () - 1] = tmp;
						}
						else
						{
							AnimationFrameEntry tmp = m_Frames[i];
							m_Frames[i] = m_Frames[i - 1];
							m_Frames[i - 1] = tmp;
						}

						break;
					}

					ImGui::SameLine ();

					if (ImGui::Button ((std::string ("v##animButton") + std::to_string (i)).c_str ()))
					{
						if (i == m_Frames.size () - 1)
						{
							AnimationFrameEntry tmp = m_Frames[0];
							m_Frames[0] = m_Frames[m_Frames.size () - 1];
							m_Frames[m_Frames.size () - 1] = tmp;
						}
						else
						{
							AnimationFrameEntry tmp = m_Frames[i];
							m_Frames[i] = m_Frames[i + 1];
							m_Frames[i + 1] = tmp;
						}

						break;
					}

					ImGui::NextColumn ();
				}

				ImGui::Columns (1);
			}

			ImGui::Separator ();

			ImGui::BeginGroup ();

			if (ImGui::Button ("SAVE", ImVec2 (controlWidth / 2 + 40, 18)))
			{
				OnSave ();
				UpdateAnimations ();
			}

			ImGui::SameLine ();
			if (ImGui::Button ("DELETE", ImVec2 (controlWidth / 2 + 40, 18)))
			{
				OnDelete ();
				UpdateAnimations ();
			}

			ImGui::Separator ();
			ImGui::Separator ();

			if (ImGui::Button ("ACCEPT", ImVec2 (controlWidth / 2 + 40, 18)))
			{
				ImGui::CloseCurrentPopup ();
				m_IsVisible = false;

				OnAccept ();
			}

			ImGui::SameLine ();

			if (ImGui::Button ("CANCEL", ImVec2 (controlWidth / 2 + 40, 18)) || m_Editor->IsCloseCurrentPopup ())
			{
				OnCancel ();

				ImGui::CloseCurrentPopup ();
				m_IsVisible = false;

				m_Editor->SetCloseCurrentPopup (false);
			}
			ImGui::EndGroup ();

			ImGui::EndPopup ();
		}
	}

	//--------------------------------------------------------------------------------------------------

	void EditorAnimationWindow::UpdateImageCombos ()
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
	}

	//--------------------------------------------------------------------------------------------------

	void EditorAnimationWindow::UpdateAnimations ()
	{
		m_Animations.clear ();
		m_Animations.push_back ("");

		std::map<std::string, Animation>& animations = ActorFactory::GetAnimations ();

		for (auto& kv : animations)
		{
			m_Animations.push_back (kv.first);
		}
	}

	//--------------------------------------------------------------------------------------------------

	void EditorAnimationWindow::UpdateNames ()
	{
		m_Names.clear ();
		m_Names.push_back ("");

		m_Animation = ActorFactory::GetAnimation (m_Animations[m_SelectedAnimationGroupIndex]);
		std::map<std::string, AnimationData>& animations = m_Animation.GetAnimations ();

		for (auto& kv : animations)
		{
			m_Names.push_back (kv.first);
		}

		//  Also fill other data
		m_SelectedName = 0;
		memset (m_AnimationName, 0, ARRAY_SIZE (m_AnimationName));

		m_AnimSpeed = 0;
		m_Frames.clear ();
	}

	//--------------------------------------------------------------------------------------------------

	void EditorAnimationWindow::RenderSpritesheet ()
	{
		const Point winSize = m_Editor->GetMainLoop ()->GetScreen ()->GetRealWindowSize ();
		const Point beginPoint = {335, 5};

		const int margin = 10;

		al_draw_filled_rectangle (
			beginPoint.X, beginPoint.Y, winSize.Width - margin, winSize.Height - margin, COLOR_BLACK);
		al_draw_rectangle (beginPoint.X, beginPoint.Y, winSize.Width - margin, winSize.Height - margin, COLOR_GREEN, 2);

		if (m_SelectedAtlas)
		{
			ALLEGRO_BITMAP* image = m_SelectedAtlas->GetImage ();

			if (image)
			{
				int imageWidth = al_get_bitmap_width (image);
				int imageHeight = al_get_bitmap_height (image);
				int canvasWidth = static_cast<int> (winSize.Width - margin - beginPoint.X);
				int canvasHeight = static_cast<int> (winSize.Height - margin - beginPoint.Y);

				int destWidth = imageWidth;
				int destHeight = imageHeight;
				float ratio = 1.0f;

				if (imageWidth > canvasWidth || imageHeight > canvasHeight)
				{
					ratio = std::max (
						static_cast<float> (imageWidth) / canvasWidth, static_cast<float> (imageHeight) / canvasHeight);
					destWidth /= ratio;
					destHeight /= ratio;
				}

				al_draw_scaled_bitmap (
					image, 0, 0, imageWidth, imageHeight, beginPoint.X, beginPoint.Y, destWidth, destHeight, 0);

				//  Mouse selection
				std::vector<AtlasRegion>& regions = m_SelectedAtlas->GetRegions ();

				ALLEGRO_MOUSE_STATE state;
				al_get_mouse_state (&state);

				m_HoveredRegion = nullptr;
				m_HoveredArea = Rect::ZERO_RECT;

				if (m_CellX > 0 && m_CellY > 0)
				{
					float drawingWidth = winSize.Width - margin;
					float drawingHeight = winSize.Height - margin;
					Rect drawingRect = {beginPoint.X + 1, beginPoint.Y + 1, drawingWidth, drawingHeight};

					if (InsideRect (state.x, state.y, drawingRect))
					{
						Rect bounds;
						bounds.Pos
							= Point (static_cast<float> (std::floor ((state.x - beginPoint.X - 1) / m_CellX)) * m_CellX,
								static_cast<float> (std::floor ((state.y - beginPoint.Y - 1) / m_CellY)) * m_CellY);
						bounds.Size = {m_CellX, m_CellY};

						Rect drawingBounds = bounds;
						drawingBounds.Offset (beginPoint.X + 1, beginPoint.Y + 1);

						al_draw_rectangle (drawingBounds.GetTopLeft ().X, drawingBounds.GetTopLeft ().Y,
							drawingBounds.GetBottomRight ().X, drawingBounds.GetBottomRight ().Y, COLOR_YELLOW, 1);

						m_HoveredArea = bounds;
					}
				}
				else
				{
					for (AtlasRegion& region : regions)
					{
						Rect bounds = region.Bounds;
						bounds.Pos *= 1.f / ratio;
						bounds.Size *= 1.f / ratio;
						bounds.Offset (beginPoint);

						if (InsideRect (state.x, state.y, bounds))
						{
							m_HoveredRegion = &region;

							ALLEGRO_COLOR selectColor = COLOR_YELLOW;
							selectColor.a = 0.5f;

							int blendOp, blendSrc, blendDst;
							al_get_blender (&blendOp, &blendSrc, &blendDst);
							al_set_blender (ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA);

							al_draw_rectangle (bounds.GetTopLeft ().X, bounds.GetTopLeft ().Y,
								bounds.GetBottomRight ().X, bounds.GetBottomRight ().Y, COLOR_YELLOW, 1);
							al_draw_filled_rectangle (bounds.GetTopLeft ().X, bounds.GetTopLeft ().Y,
								bounds.GetBottomRight ().X, bounds.GetBottomRight ().Y, selectColor);

							al_set_blender (blendOp, blendSrc, blendDst);
						}
					}
				}
			}
		}
	}

	//--------------------------------------------------------------------------------------------------

	void EditorAnimationWindow::RenderAnimationFrames (float deltaTime)
	{
		const Point winSize = m_Editor->GetMainLoop ()->GetScreen ()->GetRealWindowSize ();
		const float margin = 10;
		const float animBoxSize = 200;
		const Point beginPoint = {margin, winSize.Height - animBoxSize - margin};

		al_draw_filled_rectangle (
			beginPoint.X, beginPoint.Y, beginPoint.X + animBoxSize, beginPoint.Y + animBoxSize, COLOR_BLACK);
		al_draw_rectangle (
			beginPoint.X, beginPoint.Y, beginPoint.X + animBoxSize, beginPoint.Y + animBoxSize, COLOR_GREEN, 2);

		m_FrameTimeLeft -= deltaTime;

		if (m_FrameTimeLeft <= 0.f)
		{
			m_FrameTimeLeft = m_AnimSpeed / 1000.f;

			if (!m_Frames.empty ())
			{
				m_CurrentFrameIndex++;

				if (m_CurrentFrameIndex >= m_Frames.size ())
				{
					m_CurrentFrameIndex = 0;
				}
			}
		}

		if (!m_Frames.empty () && m_CurrentFrameIndex < m_Frames.size ())
		{
			AnimationFrameEntry frame = m_Frames[m_CurrentFrameIndex];
			Atlas* atlas = m_Editor->GetMainLoop ()->GetAtlasManager ().GetAtlas (frame.Atlas);

			if (frame.Bounds.GetArea () > 0.f)
			{
				float xStart = beginPoint.X + 1 + animBoxSize * 0.5f - frame.Bounds.GetHalfSize ().Width;
				float yStart = beginPoint.Y + 1 + animBoxSize * 0.5f - frame.Bounds.GetHalfSize ().Height;
				atlas->DrawRegion (frame.Bounds.Pos.X, frame.Bounds.Pos.Y, frame.Bounds.Size.Width,
					frame.Bounds.Size.Height, xStart, yStart, 1.0f, 1.0f, 0.f, true);
			}
			else
			{
				AtlasRegion region = atlas->GetRegion (frame.AtlasRegion);
				Point regionHalfSize = region.Bounds.GetHalfSize ();
				float xStart = beginPoint.X + 1 + animBoxSize * 0.5f - regionHalfSize.Width;
				float yStart = beginPoint.Y + 1 + animBoxSize * 0.5f - regionHalfSize.Height;

				atlas->DrawRegion (frame.AtlasRegion, xStart, yStart, 1.0f, 1.0f, 0.f);
			}
		}
	}

	//--------------------------------------------------------------------------------------------------

	void EditorAnimationWindow::SelectAnimationFrame ()
	{
		if (m_HoveredRegion)
		{
			AnimationFrameEntry frame;
			frame.Atlas = m_SelectedAtlas->GetName ();
			frame.AtlasRegion = m_HoveredRegion->Name;
			frame.Bounds = m_HoveredRegion->Bounds;

			m_Frames.push_back (frame);
		}

		if (m_HoveredArea.GetArea () > 0.f)
		{
			AnimationFrameEntry frame;
			frame.Atlas = m_SelectedAtlas->GetName ();
			frame.Bounds = m_HoveredArea;
			frame.Bounds = m_HoveredArea;

			m_Frames.push_back (frame);
		}
	}

	//--------------------------------------------------------------------------------------------------

	bool EditorAnimationWindow::IsVisible () { return m_IsVisible; }

	//--------------------------------------------------------------------------------------------------

	void EditorAnimationWindow::ClearInputs ()
	{
		memset (m_AnimationGroup, 0, ARRAY_SIZE (m_AnimationGroup));
		memset (m_AnimationName, 0, ARRAY_SIZE (m_AnimationName));
		m_Frames.clear ();
		m_SelectedAnimationGroupIndex = 0;
		m_SelectedName = 0;
		m_AnimSpeed = 0;
	}

	//--------------------------------------------------------------------------------------------------
}
