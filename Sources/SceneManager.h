// Copyright 2017-2019 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __SCENE_MANAGER_H__
#define __SCENE_MANAGER_H__

#include "Camera.h"
#include "Common.h"
#include "Font.h"

namespace aga
{
	class Scene;
	class Player;
	class Script;
	class MainLoop;
	class AtlasManager;
	class AudioStream;
	class SpeechFrameManager;
	struct TweenData;
	struct FlagPoint;

	enum OverlayTextMode
	{
		None,
		Plain,
		Sequence,
		Random
	};

	class SceneManager : public Lifecycle
	{
	public:
		SceneManager (MainLoop* mainLoop);
		virtual ~SceneManager ();
		bool Initialize ();
		bool Destroy ();

		void BeforeEnter ();
		void AfterLeave ();

		Scene* CreateNewScene (const std::string& name);
		void AddScene (Scene* scene);
		void RemoveScene (Scene* scene);
		void SetActiveScene (Scene* scene);
		Scene* GetActiveScene ();
		Scene* GetScene (const std::string& path);

		void SetActiveScene (
			const std::string& scenePath, bool fadeAnimation = true, asIScriptFunction* sceneLoadedCallback = nullptr);

		bool ProcessEvent (ALLEGRO_EVENT* event, float deltaTime);
		bool Update (float deltaTime);
		void Render (float deltaTime);

		Player* GetPlayer ();
		Camera& GetCamera ();
		MainLoop* GetMainLoop ();

		void AddOnEnterCallback (
			const std::string& triggerName, std::function<void(std::string areaName, float dx, float dy)> func);
		void AddOnEnterCallback (const std::string& triggerName, asIScriptFunction* func);

		void RemoveOnEnterCallback (const std::string& triggerName);

		void AddOnLeaveCallback (
			const std::string& triggerName, std::function<void(std::string areaName, float dx, float dy)> func);
		void AddOnLeaveCallback (const std::string& triggerName, asIScriptFunction* func);

		void RemoveOnLeaveCallback (const std::string& triggerName);

		void RegisterChoiceFunction (const std::string& name, asIScriptFunction* func);

		Actor* GetActor (const std::string& name);
		Actor* GetActor (int id);
		FlagPoint* GetFlagPoint (const std::string& name);

		Actor* GetCurrentlyProcessedActor ();

		SpeechFrameManager* GetSpeechFrameManager ();

		void Reset ();
		void SceneFadeInOut (float fadeInMs = 500, float fadeOutMs = 500, ALLEGRO_COLOR color = COLOR_BLACK);
		bool IsTransitioning () const;

		void SetDrawPhysData (bool enable);
		bool IsDrawPhysData ();

		void SetDrawBoundingBox (bool enable);
		bool IsDrawBoundingBox ();

		void SetDrawActorsNames (bool enable);
		bool IsDrawActorsNames ();

		void SetSuppressSceneInfo (bool suppress);
		bool IsSuppressSceneInfo () const;

		AudioStream* SetSceneAudioStream (const std::string& path);
		AudioStream* GetSceneAudioStream ();

		Point GetPlayerStartLocation ();

		void SetOverlayText (const std::string& text, float duration = 2000.f, OverlayTextMode mode = Plain,
			float charTimeDelay = 5.f, ScreenRelativePosition pos = BottomRight);
		void SetOverlayTextMode (OverlayTextMode mode);
		void SetOverlayTextColor (ALLEGRO_COLOR color);

		void PushPoint (Point p);
		Point PopPoint ();

		void RegisterTriggerScene (const std::string& areaName, const std::string& sceneFile);

		Script* AttachScript (Script* script, const std::string& path, bool temporary = false);
		Script* AttachScript (const std::string& name, const std::string& path, bool temporary = false);
		void RemoveScript (const std::string& name);
		void EnableSceneScripts ();
		void DisableSceneScripts ();

		Actor* GetDummyActor ();

	private:
		void SceneIntro (float duration = 1000.f);
		void PrintOverlayText (const std::string& text, ALLEGRO_COLOR color, ScreenRelativePosition pos = BottomRight,
			const std::string& fontName = FONT_NAME_MENU_ITEM_NORMAL);
		void RenderPlainOverlay (float deltaTime);
		void RenderSequenceOverlay (float deltaTime);
		void RenderRandomOverlay (float deltaTime);
		void TeleportToMarkerPosition (std::string& lastSceneName);

		int ComputeNextOverlayCharIndex (const std::string& text);

	private:
		MainLoop* m_MainLoop;
		SpeechFrameManager* m_SpeechFrameManager;
		Player* m_Player;
		Camera m_Camera;

		std::map<std::string, Scene*> m_Scenes;
		Scene* m_ActiveScene;
		Scene* m_NextScene;

		int m_LastTweenID;
		bool m_Transitioning;
		ALLEGRO_COLOR m_FadeColor;

		bool m_SceneIntro;
		ALLEGRO_COLOR m_OverlayTextColor;

		bool m_DrawPhysData;
		bool m_DrawBoundingBox;
		bool m_DrawActorsNames;

		OverlayTextMode m_OverlayTextMode;
		std::string m_OverlayText;
		int m_OverlayCharPos;
		float m_OverlayCharDuration;
		ScreenRelativePosition m_OverlayPosition;
		float m_OverlayDuration;
		float m_OverlayCharMaxDuration;
		bool m_RandomOverlayProcessing;
		std::vector<int> m_OverlayCharsIndices;
		std::vector<std::string> m_OverlayRandomTexts;

		std::stack<Point> m_SavedPoints;

		asIScriptFunction* m_SceneLoadedCallback;
	};
}

#endif //   __SCENE_MANAGER_H__
