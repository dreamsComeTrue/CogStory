// Copyright 2017-2019 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "SceneManager.h"
#include "ActorFactory.h"
#include "AudioStream.h"
#include "MainLoop.h"
#include "Player.h"
#include "Scene.h"
#include "SceneLoader.h"
#include "Screen.h"
#include "Script.h"
#include "SpeechFrameManager.h"
#include "states/GamePlayState.h"

namespace aga
{
	//--------------------------------------------------------------------------------------------------

	SceneManager::SceneManager (MainLoop* mainLoop)
		: m_MainLoop (mainLoop)
		, m_Player (nullptr)
		, m_Camera (this)
		, m_ActiveScene (nullptr)
		, m_NextScene (nullptr)
		, m_Transitioning (true)
		, m_FadeColor (COLOR_BLACK)
		, m_DrawPhysData (true)
		, m_DrawBoundingBox (true)
		, m_DrawActorsNames (true)
		, m_OverlayText ("")
		, m_OverlayPosition (Center)
		, m_OverlayDuration (2000.f)
		, m_OverlayTextMode (Plain)
		, m_RandomOverlayProcessing (false)
		, m_OverlayTextColor (al_map_rgb (240, 240, 240))
		, m_OverlayCharMaxDuration (150.f / 1000.f)
		, m_SceneLoadedCallback (nullptr)
		, m_LastTweenID (-1)
	{
		m_SpeechFrameManager = new SpeechFrameManager (this);
	}

	//--------------------------------------------------------------------------------------------------

	SceneManager::~SceneManager ()
	{
		if (!IsDestroyed ())
		{
			Destroy ();
		}
	}

	//--------------------------------------------------------------------------------------------------

	bool SceneManager::Initialize ()
	{
		Lifecycle::Initialize ();

		ActorFactory::RegisterActorTypes ();
		ActorFactory::RegisterActorComponents ();
		ActorFactory::RegisterAnimations ();

		m_Player = new Player (this);
		m_Player->Initialize ();
		m_Player->SetCheckOverlap (true);

		m_Camera.SetFollowActor (m_Player, {0, 0});

		m_SpeechFrameManager->Initialize ();

		return true;
	}

	//--------------------------------------------------------------------------------------------------

	bool SceneManager::Destroy ()
	{
		SAFE_DELETE (m_SpeechFrameManager);
		SAFE_DELETE (m_Player);

		for (std::map<std::string, Scene*>::iterator it = m_Scenes.begin (); it != m_Scenes.end ();)
		{
			SAFE_DELETE (it->second);
			m_Scenes.erase (it++);
		}

		return Lifecycle::Destroy ();
	}

	//--------------------------------------------------------------------------------------------------

	void SceneManager::BeforeEnter ()
	{
		m_SpeechFrameManager->Clear ();
		m_Camera.ClearTransformations ();
		m_ActiveScene->SetBackgroundColor (m_ActiveScene->GetBackgroundColor ());

		AudioStream* currentAudioStream = m_ActiveScene->GetSceneAudioStream ();

		if (currentAudioStream)
		{
			float fadeTime = 1000.f;
			currentAudioStream->SetFadeIn (fadeTime);
		}
	}

	//--------------------------------------------------------------------------------------------------

	void SceneManager::AfterLeave ()
	{
		AudioStream* currentAudioStream = m_ActiveScene->GetSceneAudioStream ();

		if (currentAudioStream)
		{
			float fadeTime = 1000.f;
			currentAudioStream->SetFadeOut (fadeTime, true);
		}
	}

	//--------------------------------------------------------------------------------------------------

	Scene* SceneManager::CreateNewScene (const std::string& name)
	{
		Scene* scene = new Scene (this, Rect (0, 0, 128, 128));
		scene->SetName (name);

		m_Scenes.insert (std::make_pair (name, scene));

		return scene;
	}

	//--------------------------------------------------------------------------------------------------

	void SceneManager::AddScene (Scene* scene)
	{
		if (scene)
		{
			Scene* foundScene = GetScene (scene->GetPath ());

			if (!foundScene)
			{
				//  Save current scene as active, only for script's "Start" method purposes
				Scene* savedScene = m_ActiveScene;
				m_ActiveScene = scene;

				std::vector<ScriptMetaData>& scripts = scene->GetScripts ();

				for (ScriptMetaData& obj : scripts)
				{
					obj.ScriptObj->Run ("void Start ()");
				}

				m_ActiveScene = savedScene;

				m_Scenes.insert (std::make_pair (scene->GetPath (), scene));
			}
		}
	}

	//--------------------------------------------------------------------------------------------------

	void SceneManager::RemoveScene (Scene* scene)
	{
		for (std::map<std::string, Scene*>::iterator it = m_Scenes.begin (); it != m_Scenes.end (); ++it)
		{
			if (it->second == scene)
			{
				it->second->AfterLeave ();

				bool clearActiveScene = false;
				if (it->second == m_ActiveScene)
				{
					clearActiveScene = true;
				}

				SAFE_DELETE (it->second);
				m_Scenes.erase (it);

				if (clearActiveScene)
				{
					m_ActiveScene = nullptr;
				}

				break;
			}
		}
	}

	//--------------------------------------------------------------------------------------------------

	void SceneManager::SetActiveScene (Scene* scene)
	{
		if (scene)
		{
			std::string prevSceneName = "";
			Scene* prevScene = nullptr;

			if (m_ActiveScene != nullptr)
			{
				m_ActiveScene->AfterLeave ();
				prevSceneName = m_ActiveScene->GetName ();
				prevScene = m_ActiveScene;
			}

			m_OverlayCharsIndices.clear ();
			m_OverlayRandomTexts.clear ();
			m_RandomOverlayProcessing = true;

			m_ActiveScene = scene;
			m_ActiveScene->BeforeEnter ();
			m_ActiveScene->RunAllScripts ("void SceneChanged (const string &in str)", &prevSceneName);

			AudioStream* prevAudioStream = nullptr;

			if (prevScene)
			{
				prevAudioStream = prevScene->GetSceneAudioStream ();
			}

			AudioStream* currentAudioStream = nullptr;

			if (m_ActiveScene)
			{
				currentAudioStream = m_ActiveScene->GetSceneAudioStream ();
			}

			if (prevScene && prevAudioStream != currentAudioStream)
			{
				float fadeTime = 1000.f;

				if (prevAudioStream)
				{
					prevAudioStream->SetFadeOut (fadeTime, true);
				}

				if (currentAudioStream)
				{
					currentAudioStream->SetFadeIn (fadeTime);
				}
			}

			if (!m_ActiveScene->IsSuppressSceneInfo ())
			{
				SceneIntro (2500.0f);
			}

			TeleportToMarkerPosition (prevSceneName);

			if (m_SceneLoadedCallback)
			{
				m_MainLoop->GetScriptManager ().RunScriptFunction (m_SceneLoadedCallback);
				m_SceneLoadedCallback = nullptr;
			}
		}
	}

	//--------------------------------------------------------------------------------------------------

	void SceneManager::TeleportToMarkerPosition (std::string& lastSceneName)
	{
		//  Find matching marker position
		std::replace (lastSceneName.begin (), lastSceneName.end (), ' ', '_');
		ToUpper (lastSceneName);

		std::string flagPointName = "MARKER_" + lastSceneName;
		FlagPoint* flagPoint = m_ActiveScene->GetFlagPoint (flagPointName);

		if (flagPoint)
		{
			m_Player->SetCenterPosition (flagPoint->Pos);
		}
	}

	//--------------------------------------------------------------------------------------------------

	void SceneManager::SetActiveScene (
		const std::string& scenePath, bool fadeAnimation, asIScriptFunction* sceneLoadedCallback)
	{
		Scene* sceneToSet = GetScene (scenePath);

		if (!sceneToSet)
		{
			bool loadBounds = false; // m_MainLoop->GetStateManager ().GetActiveStateName () == GAMEPLAY_STATE_NAME;

			sceneToSet = SceneLoader::LoadScene (this, scenePath, loadBounds);
			AddScene (sceneToSet);
		}

		if (sceneLoadedCallback != nullptr)
		{
			m_SceneLoadedCallback = sceneLoadedCallback;
		}

		if (fadeAnimation)
		{
			m_NextScene = sceneToSet;

			SceneFadeInOut ();
		}
		else
		{
			SetActiveScene (sceneToSet);
		}
	}

	//--------------------------------------------------------------------------------------------------

	Scene* SceneManager::GetActiveScene () { return m_ActiveScene; }

	//--------------------------------------------------------------------------------------------------

	Scene* SceneManager::GetScene (const std::string& path)
	{
		for (std::map<std::string, Scene*>::iterator it = m_Scenes.begin (); it != m_Scenes.end (); ++it)
		{
			if ((*it->second).GetPath () == path)
			{
				return it->second;
			}
		}

		return nullptr;
	}

	//--------------------------------------------------------------------------------------------------

	bool SceneManager::ProcessEvent (ALLEGRO_EVENT* event, float deltaTime)
	{
		return m_SpeechFrameManager->ProcessEvent (event, deltaTime);
	}

	//--------------------------------------------------------------------------------------------------

	bool SceneManager::Update (float deltaTime)
	{
		if (m_ActiveScene)
		{
			m_ActiveScene->Update (deltaTime);

			if (m_MainLoop->GetStateManager ().GetActiveStateName () == GAMEPLAY_STATE_NAME)
			{
				m_SpeechFrameManager->Update (deltaTime);
			}
		}

		return true;
	}

	//--------------------------------------------------------------------------------------------------

	void SceneManager::Render (float deltaTime)
	{
		if (m_ActiveScene)
		{
			m_ActiveScene->Render (deltaTime);

			m_Camera.UseIdentityTransform ();

			if (m_MainLoop->GetStateManager ().GetActiveStateName () == GAMEPLAY_STATE_NAME)
			{
				m_SpeechFrameManager->Render (deltaTime);
			}
		}

		if (m_Transitioning)
		{
			const Point size = m_MainLoop->GetScreen ()->GetBackBufferSize ();
			al_draw_filled_rectangle (0, 0, size.Width, size.Height, m_FadeColor);
		}

		if (m_ActiveScene && m_SceneIntro)
		{
			m_OverlayPosition = BottomRight;
			m_OverlayText = m_ActiveScene->GetName ();
		}

		switch (m_OverlayTextMode)
		{
		case OverlayTextMode::Plain:
			RenderPlainOverlay (deltaTime);
			break;

		case OverlayTextMode::Sequence:
			RenderSequenceOverlay (deltaTime);
			break;

		case OverlayTextMode::Random:
			RenderRandomOverlay (deltaTime);
			break;
		}
	}

	//--------------------------------------------------------------------------------------------------

	void SceneManager::RenderPlainOverlay (float deltaTime)
	{
		int blendOp, blendSrc, blendDst;
		al_get_blender (&blendOp, &blendSrc, &blendDst);
		al_set_blender (ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA);

		if (!m_ActiveScene->IsSuppressSceneInfo ())
		{
			PrintOverlayText (m_OverlayText, m_OverlayTextColor, m_OverlayPosition);
		}

		al_set_blender (blendOp, blendSrc, blendDst);
	}

	//--------------------------------------------------------------------------------------------------

	void SceneManager::RenderSequenceOverlay (float deltaTime)
	{
		int blendOp, blendSrc, blendDst;
		al_get_blender (&blendOp, &blendSrc, &blendDst);
		al_set_blender (ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA);

		PrintOverlayText (m_OverlayText.substr (0, m_OverlayCharPos), m_OverlayTextColor, m_OverlayPosition);

		al_set_blender (blendOp, blendSrc, blendDst);

		m_OverlayCharDuration += deltaTime;

		if (m_OverlayCharDuration > m_OverlayCharMaxDuration)
		{
			++m_OverlayCharPos;
			m_OverlayCharPos = std::min (m_OverlayCharPos, static_cast<int> (m_OverlayText.length ()));

			m_OverlayCharDuration = 0.f;
		}

		if (m_OverlayCharPos >= m_OverlayText.length ())
		{
			m_SceneIntro = false;
		}
	}

	//--------------------------------------------------------------------------------------------------

	void SceneManager::RenderRandomOverlay (float deltaTime)
	{
		std::string finalString = "";

		m_OverlayCharDuration += deltaTime;

		if (m_RandomOverlayProcessing && m_OverlayCharDuration > m_OverlayCharMaxDuration)
		{
			auto changeColorFunc = [&](float v) {
				m_OverlayTextColor.a = std::min (1.0f, v);

				return false;
			};

			tweeny::tween<float> tween;
			float timeMs = m_OverlayCharMaxDuration * 1000.f;

			if (m_OverlayCharsIndices.size () < m_OverlayText.length ())
			{
				int overlayCharIndex = ComputeNextOverlayCharIndex (m_OverlayText);
				m_OverlayCharDuration = 0.f;
				m_OverlayTextColor.a = 0.f;

				for (int i = 0; i < m_OverlayText.length (); ++i)
				{
					if (i == overlayCharIndex)
					{
						finalString += m_OverlayText[i];
					}
					else
					{
						finalString += " ";
					}
				}

				m_OverlayRandomTexts.push_back (finalString);

				tween = tweeny::from (0.0f).to (1.0f).during (timeMs).onStep (changeColorFunc);
				m_MainLoop->GetTweenManager ().AddTween (-1, tween, nullptr);
			}
			else
			{
				float closeTimeMs = timeMs * 3;
				tween = tweeny::from (1.0f).to (0.0f).during (closeTimeMs).onStep (changeColorFunc);
				m_MainLoop->GetTweenManager ().AddTween (-1, tween, [&](int) { m_SceneIntro = false; });

				m_RandomOverlayProcessing = false;
			}
		}

		int blendOp, blendSrc, blendDst;
		al_get_blender (&blendOp, &blendSrc, &blendDst);
		al_set_blender (ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA);

		int textsGeneratedCount = m_OverlayRandomTexts.size ();

		for (int i = 0; i < textsGeneratedCount; ++i)
		{
			ALLEGRO_COLOR color = m_OverlayTextColor;

			if (i < textsGeneratedCount - 1 && m_RandomOverlayProcessing)
			{
				color.a = 1.0f;
			}

			PrintOverlayText (m_OverlayRandomTexts[i].c_str (), color, m_OverlayPosition);
		}

		al_set_blender (blendOp, blendSrc, blendDst);
	}

	//--------------------------------------------------------------------------------------------------

	Player* SceneManager::GetPlayer () { return m_Player; }

	//--------------------------------------------------------------------------------------------------

	Camera& SceneManager::GetCamera () { return m_Camera; }

	//--------------------------------------------------------------------------------------------------

	MainLoop* SceneManager::GetMainLoop () { return m_MainLoop; }

	//--------------------------------------------------------------------------------------------------

	void SceneManager::AddOnEnterCallback (
		const std::string& triggerName, std::function<void(std::string areaName, float dx, float dy)> func)
	{
		if (m_ActiveScene)
		{
			m_ActiveScene->AddOnEnterCallback (triggerName, func);
		}
	}

	//--------------------------------------------------------------------------------------------------

	void SceneManager::AddOnEnterCallback (const std::string& triggerName, asIScriptFunction* func)
	{
		if (m_ActiveScene)
		{
			m_ActiveScene->AddOnEnterCallback (triggerName, func);
		}
	}

	//--------------------------------------------------------------------------------------------------

	void SceneManager::RemoveOnEnterCallback (const std::string& triggerName)
	{
		if (m_ActiveScene)
		{
			m_ActiveScene->RemoveOnEnterCallback (triggerName);
		}
	}

	//--------------------------------------------------------------------------------------------------

	void SceneManager::AddOnLeaveCallback (
		const std::string& triggerName, std::function<void(std::string areaName, float dx, float dy)> func)
	{
		if (m_ActiveScene)
		{
			m_ActiveScene->AddOnLeaveCallback (triggerName, func);
		}
	}

	//--------------------------------------------------------------------------------------------------

	void SceneManager::AddOnLeaveCallback (const std::string& triggerName, asIScriptFunction* func)
	{
		if (m_ActiveScene)
		{
			m_ActiveScene->AddOnLeaveCallback (triggerName, func);
		}
	}

	//--------------------------------------------------------------------------------------------------

	void SceneManager::RemoveOnLeaveCallback (const std::string& triggerName)
	{
		if (m_ActiveScene)
		{
			m_ActiveScene->RemoveOnLeaveCallback (triggerName);
		}
	}

	//--------------------------------------------------------------------------------------------------
	//
	void SceneManager::RegisterChoiceFunction (const std::string& name, asIScriptFunction* func)
	{
		if (m_ActiveScene)
		{
			m_ActiveScene->RegisterChoiceFunction (name, func);
		}
	}

	//--------------------------------------------------------------------------------------------------

	Actor* SceneManager::GetActor (const std::string& name)
	{
		if (name == "PLAYER")
		{
			return m_Player;
		}

		if (m_ActiveScene)
		{
			return m_ActiveScene->GetActor (name);
		}

		return nullptr;
	}

	//--------------------------------------------------------------------------------------------------

	Actor* SceneManager::GetActor (int id)
	{
		if (m_ActiveScene)
		{
			return m_ActiveScene->GetActor (id);
		}

		return nullptr;
	}

	//--------------------------------------------------------------------------------------------------

	FlagPoint* SceneManager::GetFlagPoint (const std::string& name)
	{
		if (m_ActiveScene)
		{
			return m_ActiveScene->GetFlagPoint (name);
		}

		return nullptr;
	}

	//--------------------------------------------------------------------------------------------------

	Actor* SceneManager::GetCurrentlyProcessedActor ()
	{
		if (m_ActiveScene)
		{
			return m_ActiveScene->GetCurrentlyProcessedActor ();
		}

		return nullptr;
	}

	//--------------------------------------------------------------------------------------------------

	AudioStream* SceneManager::SetSceneAudioStream (const std::string& path)
	{
		if (m_ActiveScene)
		{
			return m_ActiveScene->SetSceneAudioStream (path);
		}

		return nullptr;
	}

	//--------------------------------------------------------------------------------------------------

	AudioStream* SceneManager::GetSceneAudioStream ()
	{
		if (m_ActiveScene)
		{
			return m_ActiveScene->GetSceneAudioStream ();
		}

		return nullptr;
	}

	//--------------------------------------------------------------------------------------------------

	Point SceneManager::GetPlayerStartLocation ()
	{
		if (m_ActiveScene)
		{
			return m_ActiveScene->GetPlayerStartLocation ();
		}

		return Point::ZERO_POINT;
	}

	//--------------------------------------------------------------------------------------------------

	void SceneManager::SetSuppressSceneInfo (bool suppress)
	{
		if (m_ActiveScene)
		{
			m_ActiveScene->SetSuppressSceneInfo (suppress);
		}
	}

	//--------------------------------------------------------------------------------------------------

	bool SceneManager::IsSuppressSceneInfo () const
	{
		return m_ActiveScene ? m_ActiveScene->IsSuppressSceneInfo () : false;
	}

	//--------------------------------------------------------------------------------------------------

	SpeechFrameManager* SceneManager::GetSpeechFrameManager () { return m_SpeechFrameManager; }

	//--------------------------------------------------------------------------------------------------

	void SceneManager::Reset ()
	{
		m_Camera.ClearTransformations ();
		m_FadeColor.a = 0.0f;
		m_Transitioning = false;

		m_OverlayTextColor.a = 0.0f;
		m_SceneIntro = false;

		m_OverlayRandomTexts.clear ();
		m_OverlayCharsIndices.clear ();
		m_RandomOverlayProcessing = true;
	}

	//--------------------------------------------------------------------------------------------------

	void SceneManager::SceneFadeInOut (float fadeInMs, float fadeOutMs, ALLEGRO_COLOR color)
	{
		m_FadeColor = color;
		m_FadeColor.a = 0.0f;
		m_Transitioning = true;

		auto fadeFunc = [&](float value) {
			m_FadeColor.a = value;

			if (m_NextScene && value >= 0.98f)
			{
				if (m_NextScene != m_ActiveScene)
				{
					SetActiveScene (m_NextScene);
				}

				m_NextScene = nullptr;
			}

			return false;
		};

		tweeny::tween<float> tween
			= tweeny::from (0.0f).to (1.0f).during (fadeInMs).to (0.0f).during (fadeOutMs).onStep (fadeFunc);

		m_MainLoop->GetTweenManager ().AddTween (-1, tween, [&](int) { m_Transitioning = false; });
	}

	//--------------------------------------------------------------------------------------------------

	bool SceneManager::IsTransitioning () const { return m_Transitioning; }

	//--------------------------------------------------------------------------------------------------

	void SceneManager::SetDrawPhysData (bool enable) { m_DrawPhysData = enable; }

	//--------------------------------------------------------------------------------------------------

	bool SceneManager::IsDrawPhysData () { return m_DrawPhysData; }

	//--------------------------------------------------------------------------------------------------

	void SceneManager::SetDrawBoundingBox (bool enable) { m_DrawBoundingBox = enable; }

	//--------------------------------------------------------------------------------------------------

	bool SceneManager::IsDrawBoundingBox () { return m_DrawBoundingBox; }

	//--------------------------------------------------------------------------------------------------

	void SceneManager::SetDrawActorsNames (bool enable) { m_DrawActorsNames = enable; }

	//--------------------------------------------------------------------------------------------------

	bool SceneManager::IsDrawActorsNames () { return m_DrawActorsNames; }

	//--------------------------------------------------------------------------------------------------

	void SceneManager::SceneIntro (float duration)
	{
		m_SceneIntro = true;

		if (m_OverlayTextMode == OverlayTextMode::Plain)
		{
			m_OverlayTextColor.a = 0.0f;

			auto introFunc = [&](float v) {
				m_OverlayTextColor.a = std::min (1.0f, v);

				return false;
			};

			float fadeShowTime = duration * 0.5f;
			tweeny::tween<float> tween = tweeny::from (0.0f)
											 .to (1.0f)
											 .during (fadeShowTime)
											 .to (1.0f)
											 .during (duration)
											 .to (0.0f)
											 .during (fadeShowTime)
											 .onStep (introFunc);

			if (m_LastTweenID >= 0)
			{
				m_MainLoop->GetTweenManager ().RemoveTween (m_LastTweenID);
			}

			TweenData& lastTween
				= m_MainLoop->GetTweenManager ().AddTween (-1, tween, [&](int) { m_SceneIntro = false; });
			m_LastTweenID = lastTween.ID;
		}
	}

	//--------------------------------------------------------------------------------------------------

	void SceneManager::PrintOverlayText (
		const std::string& text, ALLEGRO_COLOR color, ScreenRelativePosition screenPos, const std::string& fontName)
	{
		const Point winSize = m_MainLoop->GetScreen ()->GetBackBufferSize ();
		Font& font = m_MainLoop->GetScreen ()->GetFont ();
		Point textSize = font.GetTextDimensions (fontName, text);
		float offset = 30.f;

		Point pos;
		int align = 0;

		float textSizeHeight = textSize.Height * (std::count (text.begin (), text.end (), '\n') + 1);

		switch (screenPos)
		{
		case aga::Absoulte:
			break;

		case aga::TopLeft:
			pos = {offset, offset};
			align = ALLEGRO_ALIGN_LEFT;
			break;

		case aga::TopCenter:
			pos = {winSize.Width * 0.5f, offset};
			align = ALLEGRO_ALIGN_CENTER;
			break;

		case aga::TopRight:
			pos = {winSize.Width - offset, offset};
			align = ALLEGRO_ALIGN_RIGHT;
			break;

		case aga::BottomLeft:
			pos = {offset, winSize.Height - textSizeHeight - offset};
			align = ALLEGRO_ALIGN_LEFT;
			break;

		case aga::BottomCenter:
			pos = {winSize.Width * 0.5f, winSize.Height - textSizeHeight - offset};
			align = ALLEGRO_ALIGN_CENTER;
			break;

		case aga::BottomRight:
			pos = {winSize.Width - textSizeHeight, winSize.Height - textSizeHeight - offset};
			align = ALLEGRO_ALIGN_RIGHT;
			break;

		case aga::Center:
			pos = {winSize.Width * 0.5f, winSize.Height * 0.5f - textSizeHeight * 0.5f};
			align = ALLEGRO_ALIGN_CENTER;
			break;
		}

		font.DrawMultilineText (fontName, color, pos.X, pos.Y, winSize.Width, textSize.Height, align, text.c_str ());
	}

	//--------------------------------------------------------------------------------------------------

	int SceneManager::ComputeNextOverlayCharIndex (const std::string& text)
	{
		do
		{
			int nextIndex = RandInRange (0, text.size ());

			if (std::find (m_OverlayCharsIndices.begin (), m_OverlayCharsIndices.end (), nextIndex)
				== m_OverlayCharsIndices.end ())
			{
				m_OverlayCharsIndices.push_back (nextIndex);

				return nextIndex;
			}
		} while (m_OverlayCharsIndices.size () < text.size ());

		return -1;
	}

	//--------------------------------------------------------------------------------------------------

	void SceneManager::SetOverlayText (
		const std::string& text, float duration, OverlayTextMode mode, float charTimeDelay, ScreenRelativePosition pos)
	{
		m_OverlayText = text;
		m_OverlayDuration = duration;
		m_OverlayTextMode = mode;
		m_OverlayPosition = pos;
		m_OverlayCharPos = 0;
		m_OverlayCharDuration = 0.f;
		m_OverlayCharMaxDuration = charTimeDelay / 1000.f;

		m_OverlayRandomTexts.clear ();
		m_OverlayCharsIndices.clear ();

		if (m_OverlayTextMode == OverlayTextMode::Random)
		{
			m_RandomOverlayProcessing = true;
		}

		auto introFunc = [&](float v) {
			m_OverlayTextColor.a = std::min (1.0f, v);

			return false;
		};

		float halfTime = duration * 0.5f;
		tweeny::tween<float> tween = tweeny::from (0.0f)
										 .to (1.0f)
										 .during (halfTime)
										 .to (1.0f)
										 .during (duration)
										 .to (0.0f)
										 .during (halfTime)
										 .onStep (introFunc);

		m_MainLoop->GetTweenManager ().AddTween (-1, tween, [&](int) {
			m_OverlayTextMode = OverlayTextMode::None;
			m_OverlayText = "";
			m_OverlayDuration = 0.f;
		});
	}

	//--------------------------------------------------------------------------------------------------

	void SceneManager::SetOverlayTextMode (OverlayTextMode mode)
	{
		m_OverlayTextMode = mode;

		if (mode == OverlayTextMode::Random)
		{
			m_RandomOverlayProcessing = true;
		}
	}

	//--------------------------------------------------------------------------------------------------

	void SceneManager::SetOverlayTextColor (ALLEGRO_COLOR color) { m_OverlayTextColor = color; }

	//--------------------------------------------------------------------------------------------------

	void SceneManager::PushPoint (Point p) { m_SavedPoints.push (p); }

	//--------------------------------------------------------------------------------------------------

	Point SceneManager::PopPoint ()
	{
		Point point = m_SavedPoints.top ();
		m_SavedPoints.pop ();

		return point;
	}

	//--------------------------------------------------------------------------------------------------

	void SceneManager::RegisterTriggerScene (const std::string& areaName, const std::string& /**sceneFile**/)
	{
		AddOnEnterCallback (areaName, [&](std::string areaName, float, float) {
			SetActiveScene (m_ActiveScene->GetSceneTransition (areaName), true);

			return false;
		});
	}

	//--------------------------------------------------------------------------------------------------

	Script* SceneManager::AttachScript (class Script* script, const std::string& path, bool temporary)
	{
		if (m_ActiveScene)
		{
			return m_ActiveScene->AttachScript (script, path, temporary);
		}

		return nullptr;
	}

	//--------------------------------------------------------------------------------------------------

	Script* SceneManager::AttachScript (const std::string& name, const std::string& path, bool temporary)
	{
		if (m_ActiveScene)
		{
			return m_ActiveScene->AttachScript (name, path, temporary);
		}

		return nullptr;
	}

	//--------------------------------------------------------------------------------------------------

	void SceneManager::RemoveScript (const std::string& name)
	{
		if (m_ActiveScene)
		{
			m_ActiveScene->RemoveScript (name);
		}
	}

	//--------------------------------------------------------------------------------------------------

	void SceneManager::EnableSceneScripts ()
	{
		if (m_ActiveScene)
		{
			m_ActiveScene->EnableScripts ();
		}
	}

	//--------------------------------------------------------------------------------------------------

	void SceneManager::DisableSceneScripts ()
	{
		if (m_ActiveScene)
		{
			m_ActiveScene->DisableScripts ();
		}
	}

	//--------------------------------------------------------------------------------------------------

	Actor* SceneManager::GetDummyActor ()
	{
		if (m_ActiveScene)
		{
			return m_ActiveScene->GetDummyActor ();
		}

		return nullptr;
	}

	//--------------------------------------------------------------------------------------------------
}
