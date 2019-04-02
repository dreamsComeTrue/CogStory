// Copyright 2017-2019 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __ENTITY_H__
#define __ENTITY_H__

#include "Common.h"
#include "Transformable.h"

namespace aga
{
	class SceneManager;

	struct OverlapCallback
	{
		asIScriptFunction* BeginFunc = nullptr;
		asIScriptFunction* OverlappingFunc = nullptr;
		asIScriptFunction* EndFunc = nullptr;
	};

	class Entity : public Transformable
	{
	public:
		Entity ();
		Entity (SceneManager* sceneManager);
		Entity (const Entity& rhs);
		virtual ~Entity () {}

		void SetCheckOverlap (bool check);
		bool IsCheckOverlap ();

		SceneManager* GetSceneManager ();
		bool IsOverlaping (Entity* entity);

		float GetAngleWith (Entity* ent);

		virtual std::string GetTypeName () = 0;

		void AddBeginOverlapCallback (asIScriptFunction* func);
		void AddOverlappingCallback (asIScriptFunction* func);
		void AddEndOverlapCallback (asIScriptFunction* func);
		void AddOverlapCallbacks (asIScriptFunction* begin, asIScriptFunction* update, asIScriptFunction* end);

		std::vector<Entity*> GetBluprintChildren ();

		static int GetNextID ();

		static bool CompareByZOrder (const Entity* a, const Entity* b);

		static int GlobalID;

	public:
		int ID = 0;
		std::string Name;
		int ZOrder = 0;
		int RenderID = 0;
		int BlueprintID = -1;
		Point OverlapSize;

	protected:
		void CheckOverlap ();
		virtual void BeginOverlap (Entity* entity);
		virtual void EndOverlap (Entity* entity);

		void CallBeginOverlapCallbacks (Entity* whom, Entity* target);
		void CallOverlappingCallbacks (Entity* whom, Entity* target);
		void CallEndOverlapCallbacks (Entity* whom, Entity* target);
		
		Rect GetOverlapRect ();

	protected:
		SceneManager* m_SceneManager;
		std::vector<Entity*> m_OverlapedEntities;
		std::vector<OverlapCallback> m_OverlapCallbacks;

		bool m_CheckOverlap;
	};
}

#endif //   __ENTITY_H__
