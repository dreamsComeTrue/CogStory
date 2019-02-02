// Copyright 2017-2019 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "Editor.h"
#include "EditorActorWindow.h"
#include "EditorAnimationWindow.h"
#include "EditorComponentWindow.h"
#include "EditorOpenSceneWindow.h"
#include "EditorSaveSceneWindow.h"
#include "EditorSceneWindow.h"
#include "EditorScriptWindow.h"
#include "EditorSpeechWindow.h"
#include "EditorTilesWindow.h"
#include "MainLoop.h"
#include "Player.h"
#include "Resources.h"
#include "SceneLoader.h"
#include "Screen.h"
#include "SpeechFrameManager.h"
#include "states/GamePlayState.h"

#include "imgui.h"
#include "imgui_impl_allegro5.h"

using json = nlohmann::json;

namespace aga
{
	//--------------------------------------------------------------------------------------------------

	std::string configFileName = "editor_config.json";

	const float DOUBLE_CLICK_SPEED = 300;

	//--------------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------------

	Editor::Editor (MainLoop* mainLoop)
		: m_MainLoop (mainLoop)
		, m_EditorPhysMode (this)
		, m_EditorFlagPointMode (this)
		, m_EditorTriggerAreaMode (this)
		, m_EditorSpeechMode (this)
		, m_EditorActorMode (this)
		, m_EditorUndoRedo (this)
		, m_CursorMode (CursorMode::ActorSelectMode)
		, m_IsSnapToGrid (true)
		, m_BaseGridSize (16.0f)
		, m_GridSize (16.0f)
		, m_IsMousePan (false)
		, m_LastTimeClicked (0.0f)
		, m_IsRectSelection (false)
		, m_CloseCurrentPopup (false)
		, m_OpenPopupOpenScene (false)
		, m_OpenPopupSaveScene (false)
		, m_OpenPopupActorEditor (false)
		, m_OpenPopupFlagPointEditor (false)
		, m_OpenPopupTriggerAreaEditor (false)
		, m_OpenPopupSpeechEditor (false)
		, m_OpenPopupAnimationEditor (false)
		, m_OpenPopupTilesEditor (false)
		, m_DrawActors (true)
		, m_DrawFlagPoints (true)
		, m_DrawTriggerAreas (true)
		, m_DrawCameraBounds (false)
		, m_IsMouseDrag (false)
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

	bool Editor::Initialize ()
	{
		Lifecycle::Initialize ();

		//  Diaglos & windows
		{
			m_OpenSceneWindow = new EditorOpenSceneWindow (this);
			m_SaveSceneWindow = new EditorSaveSceneWindow (this);
			m_EditorSceneWindow = new EditorSceneWindow (this);
			m_SpeechWindow = new EditorSpeechWindow (this);
			m_ActorWindow = new EditorActorWindow (this);
			m_ScriptWindow = new EditorScriptWindow (this);
			m_ComponentWindow = new EditorComponentWindow (this);
			m_AnimationWindow = new EditorAnimationWindow (this);
			m_TilesWindow = new EditorTilesWindow (this);
		}

		std::map<std::string, Atlas*>& atlases = m_MainLoop->GetAtlasManager ().GetAtlases ();
		m_EditorActorMode.ChangeAtlas ((*atlases.begin ()).first);

		LoadConfig ();
		ScreenResize ();

		al_identity_transform (&m_NewTransform);

		// Setup Dear ImGui binding
		IMGUI_CHECKVERSION ();
		ImGui::CreateContext ();
		ImGui::GetIO ().IniFilename = nullptr;
		//      ImGuiIO& io = ImGui::GetIO ();
		//        (void)io;
		// io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
		ImGui_ImplAllegro5_Init (m_MainLoop->GetScreen ()->GetDisplay ());

		// Setup style
		ImGui::StyleColorsClassic ();
		ImGui::GetStyle ().FrameRounding = 12.0f;
		ImGui::GetStyle ().ItemSpacing.y = 3.f;
		ImGui::GetStyle ().WindowTitleAlign.x = 0.5f;
		ImGui::GetStyle ().WindowBorderSize = 0.f;

		ImGui::GetIO ().FontDefault
			= ImGui::GetIO ().Fonts->AddFontFromFileTTF (GetResourcePath (FONT_MEDIUM).c_str (), 15.0f);

		return true;
	}

	//--------------------------------------------------------------------------------------------------

	bool Editor::Destroy ()
	{
		SaveConfig ();

		SAFE_DELETE (m_OpenSceneWindow);
		SAFE_DELETE (m_SaveSceneWindow);
		SAFE_DELETE (m_SpeechWindow);
		SAFE_DELETE (m_ActorWindow);
		SAFE_DELETE (m_EditorSceneWindow);
		SAFE_DELETE (m_ScriptWindow);
		SAFE_DELETE (m_ComponentWindow);
		SAFE_DELETE (m_AnimationWindow);
		SAFE_DELETE (m_TilesWindow);

		// Cleanup
		ImGui_ImplAllegro5_Shutdown ();
		ImGui::DestroyContext ();

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
			m_DrawActors = j["show_actors"];
			m_DrawFlagPoints = j["show_flag_points"];
			m_DrawTriggerAreas = j["show_trigger_areas"];
			m_DrawCameraBounds = j["show_camera_bounds"];

			auto& recentFiles = j["recent_files"];

			for (auto& file : recentFiles)
			{
				m_OpenSceneWindow->AddRecentFileName (file);
			}
		}
		catch (const std::exception&)
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
			j["show_actors"] = m_DrawActors;
			j["show_flag_points"] = m_DrawFlagPoints;
			j["show_trigger_areas"] = m_DrawTriggerAreas;
			j["show_camera_bounds"] = m_DrawCameraBounds;

			j["recent_files"] = json::array ({});

			std::vector<std::string>& files = m_OpenSceneWindow->GetRecentFileNames ();
			for (std::string recentFile : files)
			{
				j["recent_files"].push_back (recentFile);
			}

			// write prettified JSON to another file
			std::ofstream out ((GetDataPath () + configFileName).c_str ());
			out << std::setw (4) << j.dump (4, ' ') << "\n";
		}
		catch (const std::exception&)
		{
		}
	}

	//--------------------------------------------------------------------------------------------------

	void Editor::BeforeEnter ()
	{
		OnResetScale ();
		OnResetTranslate ();
		SetDrawUITiles (true);

		if (m_LastScenePath == "")
		{
			m_LastScenePath = m_MainLoop->GetSceneManager ().GetActiveScene ()->GetPath ();
		}
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
		return (!m_EditorSceneWindow->IsVisible () && !m_SpeechWindow->IsVisible () && !m_ActorWindow->IsVisible ()
			&& !m_OpenSceneWindow->IsVisible () && !m_EditorFlagPointMode.IsVisible ()
			&& !m_EditorTriggerAreaMode.IsVisible () && !m_AnimationWindow->IsVisible ()
			&& !m_TilesWindow->IsVisible ());
	}

	//--------------------------------------------------------------------------------------------------

	void Editor::HandleCameraPan (float deltaTime)
	{
		ALLEGRO_KEYBOARD_STATE state;
		al_get_keyboard_state (&state);

		int delta = static_cast<int> (300 * deltaTime);

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
		ImGui::CloseCurrentPopup ();

		OnCloseSpriteSheetEdit ();
	}

	//--------------------------------------------------------------------------------------------------

	void Editor::ProcessEvent (ALLEGRO_EVENT* event, float)
	{
		if (event->type == ALLEGRO_EVENT_KEY_CHAR)
		{
			if (event->keyboard.keycode == ALLEGRO_KEY_ESCAPE)
			{
				m_CloseCurrentPopup = true;
				TryToCloseWindows ();
			}
		}

		if (event->type == ALLEGRO_EVENT_MOUSE_BUTTON_UP)
		{
			if (m_AnimationWindow->IsVisible () && event->mouse.button == 1)
			{
				m_AnimationWindow->SelectAnimationFrame ();
			}
		}

		ImGui_ImplAllegro5_ProcessEvent (event);

		if (m_OpenSceneWindow->IsVisible () && event->type == ALLEGRO_EVENT_KEY_CHAR)
		{
			m_OpenSceneWindow->ProcessEvent (event);
		}

		if (event->type == ALLEGRO_EVENT_DISPLAY_RESIZE)
		{
			ImGui_ImplAllegro5_InvalidateDeviceObjects ();
			al_acknowledge_resize (m_MainLoop->GetScreen ()->GetDisplay ());
			ImGui_ImplAllegro5_CreateDeviceObjects ();
		}

		ImGuiIO& io = ImGui::GetIO ();

		if (io.WantCaptureKeyboard || io.WantCaptureMouse)
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
				if (event->keyboard.modifiers == ALLEGRO_KEYMOD_CTRL)
				{
					m_EditorUndoRedo.PopCommand ();
				}
				else
				{
					m_EditorActorMode.ChangeZOrder (event->keyboard.modifiers == ALLEGRO_KEYMOD_SHIFT);
				}

				break;
			}

			case ALLEGRO_KEY_X:
			case ALLEGRO_KEY_DELETE:
			{
				m_EditorActorMode.RemoveSelectedActors ();
				break;
			}

			case ALLEGRO_KEY_C:
			{
				bool changeSelection = event->keyboard.modifiers != ALLEGRO_KEYMOD_SHIFT;
				bool linkWithParent = event->keyboard.modifiers == ALLEGRO_KEYMOD_CTRL;

				m_EditorActorMode.CopySelectedActors (changeSelection, linkWithParent);

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

			case ALLEGRO_KEY_Q:
			{
				if (event->keyboard.modifiers == ALLEGRO_KEYMOD_CTRL)
				{
					m_MainLoop->Exit ();
				}
				else
				{
					OnOpenScene ();
				}
				break;
			}

			case ALLEGRO_KEY_W:
			{
				OnSaveScene ();
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

			case ALLEGRO_KEY_F4:
			{
				OnAnimation ();
				break;
			}

			case ALLEGRO_KEY_F12:
			{
				OnTilesEditor ();
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

		if (event->type == ALLEGRO_EVENT_MOUSE_BUTTON_UP)
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

		if (event->type == ALLEGRO_EVENT_MOUSE_AXES)
		{
			ProcessMouseAxes (event->mouse);
		}

		if (event->type == ALLEGRO_EVENT_DISPLAY_RESIZE)
		{
			ScreenResize ();
		}
	}

	//--------------------------------------------------------------------------------------------------

	void Editor::HandleCameraMovement (const ALLEGRO_MOUSE_EVENT& event)
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
		// Start the Dear ImGui frame
		ImGui_ImplAllegro5_NewFrame ();
		ImGui::NewFrame ();

		if (m_CursorMode == CursorMode::EditPhysBodyMode)
		{
			if (m_IsSnapToGrid)
			{
				DrawGrid ();
			}

			RenderPhysBodyMode (deltaTime);
		}
		else if (m_CursorMode == CursorMode::EditSpriteSheetMode)
		{
			m_MainLoop->GetSceneManager ().GetCamera ().Update (deltaTime);
			m_EditorActorMode.RenderSpriteSheet ();

			DrawSelectionRect (m_SelectionRect);
		}
		else
		{
			RenderActorMode (deltaTime);
		}

		//        bool showMe;
		//   ImGui::ShowDemoWindow (&showMe);

		//  Draw GUI
		ImGui::Render ();
		ImGui_ImplAllegro5_RenderDrawData (ImGui::GetDrawData ());

		if (m_ActorWindow->IsVisible ())
		{
			m_ActorWindow->RenderActorImage ();
		}

		if (m_AnimationWindow->IsVisible ())
		{
			m_AnimationWindow->RenderSpritesheet ();
			m_AnimationWindow->RenderAnimationFrames (deltaTime);
		}

		if (m_TilesWindow->IsVisible ())
		{
			m_TilesWindow->RenderSpritesheet ();
		}

		if (m_SpeechWindow->IsVisible ())
		{
			SpeechFrameManager* frameManager = m_MainLoop->GetSceneManager ().GetSpeechFrameManager ();
			frameManager->Update (deltaTime);
			frameManager->Render (deltaTime);
		}
	}

	//--------------------------------------------------------------------------------------------------

	void Editor::DrawSelectionRect (Rect rect)
	{
		//  Prevent drawing selection rect with 1-pixel size (click-down-up)
		if (rect.GetSize ().Width > 2 || rect.GetSize ().Height > 2)
		{
			al_draw_rectangle (rect.GetTopLeft ().X, rect.GetTopLeft ().Y, rect.GetBottomRight ().X,
				rect.GetBottomRight ().Y, COLOR_WHITE, 2);

			ALLEGRO_COLOR colorBlue = COLOR_BLUE;
			colorBlue.a = 0.001f;

			al_draw_filled_rectangle (rect.GetTopLeft ().X, rect.GetTopLeft ().Y, rect.GetBottomRight ().X,
				rect.GetBottomRight ().Y, colorBlue);
		}
	}

	//--------------------------------------------------------------------------------------------------

	void Editor::RenderActorMode (float deltaTime)
	{
		if (m_IsSnapToGrid)
		{
			DrawGrid ();
		}

		m_MainLoop->GetSceneManager ().Render (deltaTime);

		if (m_DrawTriggerAreas)
		{
			m_EditorTriggerAreaMode.Render ();
		}

		m_MainLoop->GetSceneManager ().GetCamera ().UseIdentityTransform ();

		if (IsEditorCanvasNotCovered ())
		{
			ALLEGRO_MOUSE_STATE state;
			al_get_mouse_state (&state);

			Rect r;
			Actor* actorUnderCursor = m_EditorActorMode.GetActorUnderCursor (state.x, state.y, false, std::move (r));

			if (actorUnderCursor)
			{
				al_draw_rectangle (r.GetTopLeft ().X, r.GetTopLeft ().Y, r.GetBottomRight ().X, r.GetBottomRight ().Y,
					COLOR_YELLOW, 2);
			}

			for (Actor* actor : m_EditorActorMode.GetSelectedActors ())
			{
				Rect r = m_MainLoop->GetSceneManager ().GetActiveScene ()->GetRenderBounds (actor, true);

				al_draw_rectangle (
					r.GetTopLeft ().X, r.GetTopLeft ().Y, r.GetBottomRight ().X, r.GetBottomRight ().Y, COLOR_RED, 2);

				if (actor->BlueprintID > -1)
				{
					Actor* parent = m_MainLoop->GetSceneManager ().GetActiveScene ()->GetActor (actor->BlueprintID);

					if (parent)
					{
						Rect r = m_MainLoop->GetSceneManager ().GetActiveScene ()->GetRenderBounds (parent, true);

						al_draw_rectangle (r.GetTopLeft ().X, r.GetTopLeft ().Y, r.GetBottomRight ().X,
							r.GetBottomRight ().Y, COLOR_BLUE, 2);
					}
				}
			}

			if (m_CursorMode == CursorMode::ActorSelectMode)
			{
				m_EditorActorMode.SetActorUnderCursor (actorUnderCursor);
			}
		}

		if (m_DrawFlagPoints)
		{
			m_EditorFlagPointMode.Render ();
		}

		m_EditorTriggerAreaMode.MarkSelectedTriggerAreas ();

		if (m_EditorActorMode.IsDrawTiles ())
		{
			m_EditorActorMode.DrawTiles ();
			RenderUI ();
		}

		if (m_DrawCameraBounds)
		{
			Point cameraScale = m_MainLoop->GetSceneManager ().GetCamera ().GetScale ();
			Point windowSize = m_MainLoop->GetScreen ()->GetRealWindowSize ();
			Point gameWindowSize = m_MainLoop->GetScreen ()->GetGameWindowSize ();

			float finalWidth = gameWindowSize.Width * cameraScale.Width / GAME_WINDOW_SCALE;
			float finalHeight = gameWindowSize.Height * cameraScale.Height / GAME_WINDOW_SCALE;
			Rect r = Rect (windowSize.Width * 0.5f - finalWidth * 0.5f, windowSize.Height * 0.5f - finalHeight * 0.5f,
				finalWidth, finalHeight);

			ALLEGRO_MOUSE_STATE state;
			al_get_mouse_state (&state);

			ALLEGRO_COLOR color = COLOR_ORANGE;

			float margin = 2;
			if (InsideRect (state.x, state.y,
					Rect (r.GetCenter ().X - margin, r.GetCenter ().Y - margin, margin * 2, margin * 2)))
			{
				color = COLOR_GREEN;
			}

			al_draw_rectangle (
				r.GetTopLeft ().X, r.GetTopLeft ().Y, r.GetBottomRight ().X, r.GetBottomRight ().Y, color, 2);
			al_draw_line (r.GetTopLeft ().X + r.GetHalfSize ().Width, r.GetTopLeft ().Y,
				r.GetTopLeft ().X + r.GetHalfSize ().Width, r.GetBottomRight ().Y, color, 1);
			al_draw_line (r.GetTopLeft ().X, r.GetTopLeft ().Y + r.GetHalfSize ().Height, r.GetBottomRight ().X,
				r.GetTopLeft ().Y + r.GetHalfSize ().Height, color, 1);
		}

		DrawSelectionRect (m_MainLoop->GetSceneManager ().GetActiveScene ()->GetRenderBounds (m_SelectionRect));
	}

	//--------------------------------------------------------------------------------------------------

	void Editor::RenderPhysBodyMode (float deltaTime)
	{
		Camera& camera = m_MainLoop->GetSceneManager ().GetCamera ();
		camera.Update (deltaTime);

		if (!m_EditorActorMode.GetSelectedActors ().empty ())
		{
			Actor* actor = m_EditorActorMode.GetSelectedActors ()[0];

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

		const Point screenSize = m_MainLoop->GetScreen ()->GetRealWindowSize ();
		Camera& camera = m_MainLoop->GetSceneManager ().GetCamera ();
		Point trans = camera.GetTranslate ();
		Point scale = camera.GetScale ();
		Point cameraCenter = camera.GetCenter ();

		cameraCenter.X = cameraCenter.X / m_GridSize * scale.X;
		cameraCenter.Y = cameraCenter.Y / m_GridSize * scale.Y;

		int halfSegmentsX = static_cast<int> (screenSize.Width * 0.5f / m_GridSize);
		int halfSegmentsY = static_cast<int> (screenSize.Height * 0.5f / m_GridSize);

		int spareSegments = 2;
		int horBeginX = static_cast<int> (cameraCenter.X - halfSegmentsX - spareSegments);
		int horEndX = static_cast<int> (cameraCenter.X + halfSegmentsX + spareSegments);

		for (int i = horBeginX; i < horEndX; ++i)
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

		int horBeginY = static_cast<int> (cameraCenter.Y - halfSegmentsY - spareSegments);
		int horEndY = static_cast<int> (cameraCenter.Y + halfSegmentsY + spareSegments);

		for (int i = horBeginY; i < horEndY; ++i)
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
		Rect r = Rect{{point.X - outsets, point.Y - outsets}, {point.X + outsets, point.Y + outsets}};

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

	void Editor::LoadScene (const std::string& openFileName)
	{
		std::string path = GetDataPath () + "scenes/" + openFileName;

		if (IsFileExists (path))
		{
			m_MainLoop->GetSceneManager ().RemoveScene (m_MainLoop->GetSceneManager ().GetActiveScene ());
			m_MainLoop->GetSceneManager ().SetActiveScene (openFileName, false);

			ResetSettings ();

			m_LastScenePath = openFileName;

			bool found = false;
			std::vector<std::string>& files = m_OpenSceneWindow->GetRecentFileNames ();
			for (std::string fileName : files)
			{
				if (fileName == openFileName)
				{
					found = true;
					break;
				}
			}

			if (!found)
			{
				m_OpenSceneWindow->AddRecentFileName (openFileName);
			}

			Log ("Scene loaded: %s\n", openFileName.c_str ());
		}
		else
		{
			Log ("Scene %s doesn't exist!", openFileName.c_str ());
		}
	}

	//--------------------------------------------------------------------------------------------------

	void Editor::SaveScene (const std::string& filePath)
	{
		std::string path = GetDataPath () + "scenes/" + filePath;
		SceneLoader::SaveScene (m_MainLoop->GetSceneManager ().GetActiveScene (), path);

		m_LastScenePath = filePath;

		Log ("Scene saved: %s\n", filePath.c_str ());
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

	void Editor::OnPlay () { m_MainLoop->GetStateManager ().SetActiveState ("GAMEPLAY_STATE"); }

	//--------------------------------------------------------------------------------------------------

	void Editor::OnScrollPrevTiles () { m_EditorActorMode.ScrollPrevTile (1); }

	//--------------------------------------------------------------------------------------------------

	void Editor::OnScrollNextTiles () { m_EditorActorMode.ScrollNextTile (1); }

	//--------------------------------------------------------------------------------------------------

	void Editor::OnBigScrollPrevTiles () { m_EditorActorMode.ScrollPrevTile (TILES_COUNT); }

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

	void Editor::OnBigScrollNextTiles () { m_EditorActorMode.ScrollNextTile (TILES_COUNT); }

	//--------------------------------------------------------------------------------------------------

	void Editor::OnResetTranslate ()
	{
		const Point& windowSize = m_MainLoop->GetScreen ()->GetRealWindowSize ();
		m_MainLoop->GetSceneManager ().GetCamera ().SetTranslate (windowSize.Width * 0.5f, windowSize.Height * 0.5f);
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

	void Editor::OnShowGrid () { m_IsSnapToGrid = !m_IsSnapToGrid; }

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

	void Editor::SwitchCursorMode ()
	{
		if (m_CursorMode != CursorMode::EditPhysBodyMode)
		{
			if (!m_EditorActorMode.GetSelectedActors ().empty ())
			{
				Camera& camera = m_MainLoop->GetSceneManager ().GetCamera ();

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
	}

	//--------------------------------------------------------------------------------------------------

	void Editor::OnRemoveBody ()
	{
		if (!m_EditorActorMode.GetSelectedActors ().empty ())
		{
			Actor* actor = m_EditorActorMode.GetSelectedActors ()[0];
			actor->PhysPoints.clear ();
			actor->ClearPhysPolygons ();

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
			Actor* actor = m_EditorActorMode.GetSelectedActors ()[0];
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
			Actor* actor = m_EditorActorMode.GetSelectedActors ()[0];

			actor->PhysPoints.push_back ({});
			m_EditorPhysMode.SetPhysPoly (&actor->PhysPoints[actor->PhysPoints.size () - 1]);
		}
	}

	//--------------------------------------------------------------------------------------------------

	void Editor::OnOpenScene ()
	{
		m_OpenPopupOpenScene = true;

		m_OpenSceneWindow->Show (m_LastScenePath, [&](std::string name) { m_LastScenePath = name; }, nullptr);
	}

	//--------------------------------------------------------------------------------------------------

	void Editor::OnSaveScene ()
	{
		m_OpenPopupSaveScene = true;

		m_SaveSceneWindow->Show (m_LastScenePath, [&](std::string name) { m_LastScenePath = name; }, nullptr);
	}

	//--------------------------------------------------------------------------------------------------

	void Editor::OnSpeech ()
	{
		m_OpenPopupSpeechEditor = true;

		m_SpeechWindow->Show ();
	}

	//--------------------------------------------------------------------------------------------------

	void Editor::OnActorSelected ()
	{
		if (!m_EditorActorMode.GetSelectedActors ().empty ())
		{
			m_OpenPopupActorEditor = true;

			m_ActorWindow->SelectActor (m_EditorActorMode.GetSelectedActors ()[0]);
			m_ActorWindow->Show ();
		}
	}

	//--------------------------------------------------------------------------------------------------

	void Editor::OnAnimation ()
	{
		m_OpenPopupAnimationEditor = true;

		m_AnimationWindow->Show ([&](std::string, std::string) {}, nullptr);
	}

	//--------------------------------------------------------------------------------------------------

	void Editor::OnTilesEditor ()
	{
		m_OpenPopupTilesEditor = true;

		m_TilesWindow->Show ([&](std::string, std::string) {}, nullptr);
	}

	//--------------------------------------------------------------------------------------------------

	void Editor::MarkPlayerPosition ()
	{
		ALLEGRO_MOUSE_STATE state;
		al_get_mouse_state (&state);

		Point point = CalculateWorldPoint (state.x, state.y);

		m_MainLoop->GetSceneManager ().GetActiveScene ()->SetPlayerStartLocation (point);

		Player* player = m_MainLoop->GetSceneManager ().GetPlayer ();

		point -= player->Bounds.GetHalfSize ();

		player->SetPosition (point);
		player->TemplateBounds.Pos = player->GetPosition ();
	}

	//--------------------------------------------------------------------------------------------------

	void Editor::RenderUI ()
	{
		ImGui::SetNextWindowPos (ImVec2 (5, 5), ImGuiCond_Always);
		ImGui::SetNextWindowSize (ImVec2 (300, 15));
		ImGui::PushStyleColor (ImGuiCol_WindowBg, ImVec4 (0.f, 0.f, 0.f, 0.0f));

		if (ImGui::Begin ("SceneLabel", nullptr,
				ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove
					| ImGuiWindowFlags_NoCollapse))
		{
			ImGui::Text (
				(ToString ("SCENE: ") + m_MainLoop->GetSceneManager ().GetActiveScene ()->GetName ()).c_str ());
			ImGui::End ();
		}
		ImGui::PopStyleColor ();

		ImGui::SetNextWindowPos (ImVec2 (5, 15), ImGuiCond_Always);
		ImGui::SetNextWindowSize (ImVec2 (150, 600));
		ImGui::PushStyleColor (ImGuiCol_WindowBg, ImVec4 (0.f, 0.f, 0.f, 0.0f));
		if (ImGui::Begin ("Tools", nullptr,
				ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove
					| ImGuiWindowFlags_NoCollapse))
		{
			ImGui::NewLine ();

			ImVec2 buttonSize = ImVec2 (135.f, 18.f);

			if (ImGui::Button ("NEW SCENE", buttonSize))
			{
				ImGui::OpenPopup ("New Scene");
			}

			RenderUINewScene ();

			if (ImGui::Button ("OPEN SCENE", buttonSize) || m_OpenPopupOpenScene)
			{
				if (!m_OpenPopupOpenScene)
				{
					OnOpenScene ();
				}

				ImGui::OpenPopup ("Open Scene");

				m_OpenPopupOpenScene = false;
			}

			m_OpenSceneWindow->Render ();

			if (ImGui::Button ("SAVE SCENE", buttonSize) || m_OpenPopupSaveScene)
			{
				if (!m_OpenPopupSaveScene)
				{
					OnSaveScene ();
				}

				ImGui::OpenPopup ("Save Scene");

				m_OpenPopupSaveScene = false;
			}

			m_SaveSceneWindow->Render ();

			if (ImGui::Button ("SCENE EDIT", buttonSize))
			{
				ImGui::OpenPopup ("Scene Settings");
				m_EditorSceneWindow->Show ();
			}

			m_EditorSceneWindow->RenderUI ();

			ImGui::NewLine ();

			if (ImGui::Button ("RESET MOVE", buttonSize))
			{
				OnResetTranslate ();
			}

			if (ImGui::Button ("RESET SCALE", buttonSize))
			{
				OnResetScale ();
			}

			if (ImGui::Button (m_IsSnapToGrid ? "HIDE GRID" : "SHOW GRID", buttonSize))
			{
				OnShowGrid ();
			}

			if (ImGui::Button ("+++", buttonSize))
			{
				OnGridIncrease ();
			}

			if (ImGui::Button ("---", buttonSize))
			{
				OnGridDecrease ();
			}

			if (m_CursorMode != CursorMode::EditPhysBodyMode)
			{
				ImGui::NewLine ();

				if (ImGui::Button ("FLAG POINT", buttonSize) || m_OpenPopupFlagPointEditor)
				{
					ImGui::OpenPopup ("Flag Point");

					m_OpenPopupFlagPointEditor = false;
					m_EditorFlagPointMode.ShowUI ();
				}

				m_EditorFlagPointMode.RenderUI ();

				if (ImGui::Button ("TRIGGER AREA", buttonSize) || m_OpenPopupTriggerAreaEditor)
				{
					ImGui::OpenPopup ("Trigger Area");

					m_OpenPopupTriggerAreaEditor = false;
					m_EditorTriggerAreaMode.ShowUI ();
				}

				m_EditorTriggerAreaMode.RenderUI ();
			}

			ImGui::NewLine ();

			if (ImGui::Button (m_CursorMode != CursorMode::EditPhysBodyMode ? "PHYS MODE" : "EXIT PHYS", buttonSize))
			{
				SwitchCursorMode ();
			}

			if (m_CursorMode == CursorMode::EditPhysBodyMode)
			{
				if (ImGui::Button ("OUTLINE BODY", buttonSize))
				{
					OutlineBody ();
				}

				if (ImGui::Button ("REMOVE BODY", buttonSize))
				{
					OnRemoveBody ();
				}

				if (ImGui::Button ("NEW POLY", buttonSize))
				{
					OnNewPoly ();
				}
			}
			else
			{
				ImGui::NewLine ();

				if (ImGui::Button ("ACTOR [F2]", buttonSize) || m_OpenPopupActorEditor)
				{
					if (!m_OpenPopupActorEditor)
					{
						OnActorSelected ();
					}

					ImGui::OpenPopup ("Actor Editor");

					m_OpenPopupActorEditor = false;
				}

				m_ActorWindow->RenderUI ();

				if (ImGui::Button ("SPEECH [F3]", buttonSize) || m_OpenPopupSpeechEditor)
				{
					if (!m_OpenPopupSpeechEditor)
					{
						OnSpeech ();
					}

					ImGui::OpenPopup ("Speech Editor");

					m_OpenPopupSpeechEditor = false;
				}

				m_SpeechWindow->RenderUI ();

				if ((ImGui::Button ("ANIM [F4]", buttonSize) || m_OpenPopupAnimationEditor))
				{
					if (!m_OpenPopupAnimationEditor)
					{
						OnAnimation ();
					}

					ImGui::OpenPopup ("Animations");

					m_OpenPopupAnimationEditor = false;
				}

				m_AnimationWindow->Render ();

				if ((ImGui::Button ("TILES [F12]", buttonSize) || m_OpenPopupTilesEditor))
				{
					if (!m_OpenPopupTilesEditor)
					{
						OnTilesEditor ();
					}

					ImGui::OpenPopup ("Tile Editor");

					m_OpenPopupTilesEditor = false;
				}

				m_TilesWindow->Render ();

				if (ImGui::Button ("PLAY [F1]", buttonSize))
				{
					OnPlay ();
				}
			}

			ImGui::End ();
		}
		ImGui::PopStyleColor ();

		const Point windowSize = m_MainLoop->GetScreen ()->GetRealWindowSize ();
		float beginning = windowSize.Width * 0.5f - (TILES_COUNT - 1) * 0.5f * TILE_SIZE - TILE_SIZE * 0.5f;
		float end = beginning + TILES_COUNT * TILE_SIZE;

		ImGui::SetNextWindowPos (ImVec2 (beginning - 130, windowSize.Height - 58), ImGuiCond_Always);
		ImGui::PushStyleColor (ImGuiCol_WindowBg, ImVec4 (0, 0, 0, 0.0f));
		ImGui::PushStyleColor (ImGuiCol_Border, ImVec4 (0, 0, 0, 0.0f));

		if (ImGui::Begin ("Sprites", nullptr,
				ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove
					| ImGuiWindowFlags_NoCollapse))
		{
			std::map<std::string, Atlas*>& atlases = m_MainLoop->GetAtlasManager ().GetAtlases ();
			static const char* selectedTileset = (*atlases.begin ()).first.c_str ();

			ImGui::PushItemWidth (110);
			if (ImGui::BeginCombo ("", selectedTileset, 0))
			{
				for (const auto& atlas : atlases)
				{
					bool is_selected = (selectedTileset == atlas.first.c_str ());

					if (ImGui::Selectable (atlas.first.c_str (), is_selected))
					{
						selectedTileset = atlas.first.c_str ();
						OnTilesetSelected (selectedTileset);
					}
					if (is_selected)
					{
						ImGui::SetItemDefaultFocus ();
					}
				}
				ImGui::EndCombo ();
			}
			ImGui::PopItemWidth ();

			if (ImGui::Button ("# [`]", ImVec2 (110.f, 18.f)))
			{
				OnSpriteSheetEdit ();
			}

			ImGui::End ();
		}
		ImGui::PopStyleColor (2);

		ImGui::SetNextWindowPos (ImVec2 (end + 5, windowSize.Height - 58), ImGuiCond_Always);
		ImGui::PushStyleColor (ImGuiCol_WindowBg, ImVec4 (0, 0, 0, 0.0f));
		ImGui::PushStyleColor (ImGuiCol_Border, ImVec4 (0, 0, 0, 0.0f));

		if (ImGui::Begin ("SpritesControls", nullptr,
				ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove
					| ImGuiWindowFlags_NoCollapse))
		{
			ImVec2 buttonSize = ImVec2 (40.f, 18.f);

			if (ImGui::Button ("<", buttonSize))
			{
				OnScrollPrevTiles ();
			}

			ImGui::SameLine ();

			if (ImGui::Button (">", buttonSize))
			{
				OnScrollNextTiles ();
			}

			if (ImGui::Button ("<<", buttonSize))
			{
				OnBigScrollPrevTiles ();
			}

			ImGui::SameLine ();

			if (ImGui::Button (">>", buttonSize))
			{
				OnBigScrollNextTiles ();
			}

			ImGui::End ();
		}
		ImGui::PopStyleColor (2);

		Point translate = m_MainLoop->GetSceneManager ().GetCamera ().GetTranslate ();
		Point scale = m_MainLoop->GetSceneManager ().GetCamera ().GetScale ();

		ALLEGRO_MOUSE_STATE state;
		al_get_mouse_state (&state);

		ImGui::SetNextWindowPos (
			ImVec2 (m_MainLoop->GetScreen ()->GetRealWindowSize ().Width - 155, 5), ImGuiCond_Always);
		ImGui::SetNextWindowSize (ImVec2 (150, 302));

		ImGui::PushStyleColor (ImGuiCol_WindowBg, ImVec4 (0, 0, 0, 0.3f));
		if (ImGui::Begin ("Info", nullptr,
				ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove
					| ImGuiWindowFlags_NoCollapse))
		{
			char buf[50];
			sprintf (buf, "AVG: %.2f ms", 1000.0f / m_MainLoop->GetScreen ()->GetFPS ());
			ImGui::Text (buf);

			sprintf (buf, "FPS: %.1f", m_MainLoop->GetScreen ()->GetFPS ());
			ImGui::Text (buf);

			Entity* selectedEntity = nullptr;

			if (!m_EditorActorMode.GetSelectedActors ().empty ())
			{
				selectedEntity = m_EditorActorMode.GetSelectedActors ()[0];
			}

			ImGui::Text (std::string ("    ID: " + (selectedEntity ? ToString (selectedEntity->ID) : "-")).c_str ());

			ImGui::Text (
				std::string ("PARENT: " + (selectedEntity ? ToString (selectedEntity->BlueprintID) : "-")).c_str ());

			std::stringstream streamX;
			streamX << std::fixed << std::setprecision (0) << (translate.X + state.x) * (1 / scale.X);
			std::stringstream streamY;
			streamY << std::fixed << std::setprecision (0) << (translate.Y + state.y) * (1 / scale.Y);

			ImGui::Text ((std::string ("     X: ") + streamX.str ()).c_str ());
			ImGui::Text ((std::string ("     Y: ") + streamY.str ()).c_str ());

			AtlasRegion* atlasRegion = m_EditorActorMode.GetSelectedAtlasRegion ();
			Point size = Point::ZERO_POINT;

			if (atlasRegion)
			{
				size = atlasRegion->Bounds.GetSize ();
			}

			ImGui::Text (std::string ("     W: " + ToString (size.Width)).c_str ());
			ImGui::Text (std::string ("     H: " + ToString (size.Height)).c_str ());
			ImGui::Text (
				std::string ("     A: " + (selectedEntity ? ToString (selectedEntity->Rotation) : "-")).c_str ());
			ImGui::Text (
				std::string ("  ZORD: " + (selectedEntity ? ToString (selectedEntity->ZOrder) : "-")).c_str ());

			std::stringstream streamScale;
			streamScale << std::fixed << std::setprecision (2) << scale.X;

			ImGui::Text ((std::string (" SCALE: ") + streamScale.str ()).c_str ());
			ImGui::Text (std::string ("  SNAP: " + ToString (m_IsSnapToGrid ? "YES" : "NO")).c_str ());
			ImGui::Text (std::string ("  GRID: " + ToString (m_BaseGridSize)).c_str ());

			Scene* activeScene = m_MainLoop->GetSceneManager ().GetActiveScene ();

			ImGui::Text (std::string (" TILES: " + ToString (activeScene->GetTiles ().size ())).c_str ());
			ImGui::Text (std::string ("ACTORS: " + ToString (activeScene->GetActors ().size ())).c_str ());

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

			ImGui::Text (std::string ("CURSOR: " + ToString (cursorMode)).c_str ());

			ImGui::End ();
		}
		ImGui::PopStyleColor ();

		ImGui::SetNextWindowPos (
			ImVec2 (m_MainLoop->GetScreen ()->GetRealWindowSize ().Width - 155, 300), ImGuiCond_Always);
		ImGui::SetNextWindowSize (ImVec2 (150, 270));
		ImGui::PushStyleColor (ImGuiCol_WindowBg, ImVec4 (0.f, 0.f, 0.f, 0.0f));
		if (ImGui::Begin ("Visiblies", nullptr,
				ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove
					| ImGuiWindowFlags_NoCollapse))
		{
			ImGui::Checkbox ("Actors", &m_DrawActors);
			ImGui::Checkbox ("Flag Points", &m_DrawFlagPoints);
			ImGui::Checkbox ("Trigger Areas", &m_DrawTriggerAreas);
			ImGui::Checkbox ("Camera Bounds", &m_DrawCameraBounds);

			ImGui::End ();
		}
		ImGui::PopStyleColor ();
	}

	//--------------------------------------------------------------------------------------------------

	void Editor::RenderUINewScene ()
	{
		if (ImGui::BeginPopupModal ("New Scene", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Text ("Are you sure clearing current scene?");

			static char sceneName[100] = {};
			ImGui::InputText ("", sceneName, ARRAY_SIZE (sceneName));
			ImGui::SetItemDefaultFocus ();

			ImGui::Separator ();
			ImGui::BeginGroup ();

			if (ImGui::Button ("YES", ImVec2 (50.f, 18.f)))
			{
				ImGui::CloseCurrentPopup ();

				ResetSettings ();
				m_MainLoop->GetSceneManager ().GetActiveScene ()->Reset ();
				m_MainLoop->GetSceneManager ().GetActiveScene ()->SetName (sceneName);
			}

			ImGui::SameLine ();

			if (ImGui::Button ("NO", ImVec2 (50.f, 18.f)) || m_CloseCurrentPopup)
			{
				ImGui::CloseCurrentPopup ();
				m_CloseCurrentPopup = false;
			}
			ImGui::EndGroup ();

			ImGui::EndPopup ();
		}
	}

	//--------------------------------------------------------------------------------------------------

	void Editor::SetDrawUITiles (bool draw) { m_EditorActorMode.SetDrawTiles (draw); }

	//--------------------------------------------------------------------------------------------------

	void Editor::ScreenResize () {}

	//--------------------------------------------------------------------------------------------------

	void Editor::ProcessMouseButtonDoubleClick (ALLEGRO_MOUSE_EVENT& event)
	{
		if (event.button == 1)
		{
			m_IsRectSelection = false;

			if (m_EditorFlagPointMode.GetFlagPointUnderCursor (event.x, event.y) != "")
			{
				m_OpenPopupFlagPointEditor = true;
			}
			else if (m_EditorTriggerAreaMode.GetTriggerAreaUnderCursor (event.x, event.y))
			{
				m_OpenPopupTriggerAreaEditor = true;
			}
			else
			{
				OnActorSelected ();
			}
		}
	}

	//--------------------------------------------------------------------------------------------------

	void Editor::ProcessMouseButtonDown (ALLEGRO_MOUSE_EVENT& event)
	{
		m_IsMousePan = event.button == 3;

		if (event.button == 1)
		{
			m_IsMouseDrag = true;

			ALLEGRO_KEYBOARD_STATE state;
			al_get_keyboard_state (&state);

			if (al_key_down (&state, ALLEGRO_KEY_LSHIFT))
			{
				Rect r;
				Actor* actorUnderCursor = m_EditorActorMode.GetActorUnderCursor (event.x, event.y, true, std::move (r));

				if (actorUnderCursor)
				{
					if (m_EditorActorMode.IsActorSelected (actorUnderCursor))
					{
						m_EditorActorMode.RemoveActorFromSelection (actorUnderCursor);
					}
					else
					{
						m_EditorActorMode.AddActorToSelection (actorUnderCursor);
					}
				}
				else
				{
					m_EditorActorMode.ClearSelectedActors ();
				}
			}
			else if (m_CursorMode == CursorMode::EditSpriteSheetMode)
			{
				m_IsRectSelection = true;
				m_SelectionRect.Pos = CalculateWorldPoint (event.x, event.y);
			}
			else if (m_CursorMode == CursorMode::ActorSelectMode)
			{
				bool newActorCreated = false;

				if (m_EditorActorMode.IsDrawTiles ())
				{
					if (m_EditorActorMode.ChooseTile (event.x, event.y))
					{
						OnCloseSpriteSheetEdit ();
						m_EditorActorMode.SetRotation (0);
						m_EditorActorMode.AddActor (event.x, event.y);
						newActorCreated = true;
					}
				}

				if (m_EditorActorMode.IsSpriteSheetChoosen ())
				{
					m_EditorActorMode.SetSpriteSheetChoosen (false);
				}

				if (!newActorCreated)
				{
					Rect r;
					Actor* actorUnderCursor
						= m_EditorActorMode.GetActorUnderCursor (event.x, event.y, true, std::move (r));

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

							if (!m_EditorActorMode.GetSelectedActors ().empty ())
							{
								Actor* actor = m_EditorActorMode.GetSelectedActors ()[0];
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

						std::vector<Actor*> selectedActors = m_EditorActorMode.GetSelectedActors ();
						MoveActorsCommand* command = new MoveActorsCommand (&m_EditorUndoRedo, selectedActors);
						m_EditorUndoRedo.PushCommand (command);
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
			if (m_CursorMode == CursorMode::EditPhysBodyMode)
			{
				m_EditorPhysMode.RemovePhysPointUnderCursor (event.x, event.y);
			}
			else
			{
				bool flagPointRemoved = m_EditorFlagPointMode.RemoveFlagPointUnderCursor (event.x, event.y);
				bool triggerPointRemoved = m_EditorTriggerAreaMode.RemoveTriggerPointUnderCursor (event.x, event.y);

				m_EditorFlagPointMode.SetFlagPoint ("");

				if (!flagPointRemoved && !triggerPointRemoved)
				{
					SetCursorMode (CursorMode::ActorSelectMode);
				}
			}
		}
	}

	//--------------------------------------------------------------------------------------------------

	void Editor::ProcessMouseButtonUp (ALLEGRO_MOUSE_EVENT& event)
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
		m_IsMouseDrag = false;
		m_IsRectSelection = false;
		m_SelectionRect = {};

		if (m_CursorMode == CursorMode::EditPhysBodyMode && event.button == 1)
		{
			SelectPhysPoint ();
		}

		if (m_CursorMode == CursorMode::EditTriggerAreaMode && event.button == 1)
		{
			ALLEGRO_KEYBOARD_STATE state;
			al_get_keyboard_state (&state);

			if (al_key_down (&state, ALLEGRO_KEY_LSHIFT))
			{
				SelectTriggerAreaPoint ();
			}
			else
			{
				m_EditorTriggerAreaMode.ClearSelection ();
			}
		}
	}

	//--------------------------------------------------------------------------------------------------

	void Editor::ProcessMouseAxes (ALLEGRO_MOUSE_EVENT& event)
	{
		if (!m_EditorFlagPointMode.MoveSelectedFlagPoint () && !m_EditorTriggerAreaMode.MoveSelectedTriggerPoint ())
		{
			if (m_CursorMode == CursorMode::EditPhysBodyMode)
			{
				m_EditorPhysMode.MoveSelectedPhysPoint ();
			}
			else if (m_CursorMode == CursorMode::ActorSelectMode || m_CursorMode == CursorMode::EditSpriteSheetMode)
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

	void Editor::AddActorsFromSpritesheet (int x, int y)
	{
		if (m_EditorActorMode.ChooseTilesFromSpriteSheet ())
		{
			m_EditorActorMode.ClearSelectedActors ();

			OnCloseSpriteSheetEdit ();
			m_EditorActorMode.AddActor (x, y);
		}
	}

	//--------------------------------------------------------------------------------------------------

	void Editor::SelectActorsWithinSelectionRect ()
	{
		Rect selectionWorld = m_MainLoop->GetSceneManager ().GetActiveScene ()->GetRenderBounds (m_SelectionRect);

		if (selectionWorld.GetSize ().Width > 2 || selectionWorld.GetSize ().Height > 2)
		{
			std::vector<Actor*>& actors = m_MainLoop->GetSceneManager ().GetActiveScene ()->GetActors ();

			for (Actor* actorIt : actors)
			{
				Rect rect = m_MainLoop->GetSceneManager ().GetActiveScene ()->GetRenderBounds (actorIt);

				if (Intersect (rect, selectionWorld))
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

	void Editor::OnTilesetSelected (const std::string& path) { m_EditorActorMode.ChangeAtlas (path); }

	//--------------------------------------------------------------------------------------------------

	CursorMode Editor::GetCursorMode () const { return m_CursorMode; }

	//--------------------------------------------------------------------------------------------------

	void Editor::SetCursorMode (CursorMode mode) { m_CursorMode = mode; }

	//--------------------------------------------------------------------------------------------------

	MainLoop* Editor::GetMainLoop () { return m_MainLoop; }

	//--------------------------------------------------------------------------------------------------

	EditorPhysMode& Editor::GetEditorPhysMode () { return m_EditorPhysMode; }

	//--------------------------------------------------------------------------------------------------

	EditorActorMode& Editor::GetEditorActorMode () { return m_EditorActorMode; }

	//--------------------------------------------------------------------------------------------------

	EditorFlagPointMode& Editor::GetEditorFlagPointMode () { return m_EditorFlagPointMode; }

	//--------------------------------------------------------------------------------------------------

	EditorSpeechMode& Editor::GetEditorSpeechMode () { return m_EditorSpeechMode; }

	//--------------------------------------------------------------------------------------------------

	EditorTriggerAreaMode& Editor::GetEditorTriggerAreaMode () { return m_EditorTriggerAreaMode; }

	//--------------------------------------------------------------------------------------------------

	EditorScriptWindow* Editor::GetScriptWindow () { return m_ScriptWindow; }

	//--------------------------------------------------------------------------------------------------

	EditorComponentWindow* Editor::GetComponentWindow () { return m_ComponentWindow; }

	//--------------------------------------------------------------------------------------------------

	EditorAnimationWindow* Editor::GetAnimationWindow () { return m_AnimationWindow; }

	//--------------------------------------------------------------------------------------------------

	EditorSpeechWindow* Editor::GetSpeechWindow () { return m_SpeechWindow; }

	//--------------------------------------------------------------------------------------------------

	EditorActorWindow* Editor::GetActorWindow () { return m_ActorWindow; }

	//--------------------------------------------------------------------------------------------------

	ALLEGRO_TRANSFORM& Editor::GetWorldTransform () { return m_WorldTransform; }

	//--------------------------------------------------------------------------------------------------

	Point Editor::GetLastMousePos () { return m_LastMousePos; }

	//--------------------------------------------------------------------------------------------------

	bool Editor::IsSnapToGrid () { return m_IsSnapToGrid; }

	//--------------------------------------------------------------------------------------------------

	float Editor::GetGridSize () { return m_GridSize; }

	//--------------------------------------------------------------------------------------------------

	Rect Editor::GetSelectionRect () { return m_SelectionRect; }

	//--------------------------------------------------------------------------------------------------

	bool Editor::IsCloseCurrentPopup () { return m_CloseCurrentPopup; }

	//--------------------------------------------------------------------------------------------------

	void Editor::SetCloseCurrentPopup (bool close) { m_CloseCurrentPopup = close; }

	//--------------------------------------------------------------------------------------------------

	EditorUndoRedo* Editor::GetUndoRedo () { return &m_EditorUndoRedo; }

	//--------------------------------------------------------------------------------------------------

} // namespace aga
