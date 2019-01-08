// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "EditorUndoRedo.h"
#include "Actor.h"
#include "Editor.h"
#include "MainLoop.h"
#include "SceneManager.h"

namespace aga
{
	//--------------------------------------------------------------------------------------------------

	UndoRedoCommand::UndoRedoCommand (EditorUndoRedo* undoRedo)
		: m_UndoRedo (undoRedo)
	{
	}

	//--------------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------------

	RemoveActorsCommand::RemoveActorsCommand (EditorUndoRedo* undoRedo, std::vector<Actor*> actors)
		: UndoRedoCommand (undoRedo)
		, m_Actors (actors)
	{
		m_Actors = actors;
	}

	//--------------------------------------------------------------------------------------------------

	void RemoveActorsCommand::AtPush ()
	{
		std::vector<Actor*> tmpActors;

		for (Actor* actor : m_Actors)
		{
			Actor* newActor = actor->Clone ();
			tmpActors.push_back (newActor);

			m_UndoRedo->GetEditor ()->GetMainLoop ()->GetSceneManager ().GetActiveScene ()->RemoveActor (actor);
		}

		m_Actors = tmpActors;
	}

	//--------------------------------------------------------------------------------------------------

	void RemoveActorsCommand::AtPop ()
	{
		for (Actor* actor : m_Actors)
		{
			m_UndoRedo->GetEditor ()->GetMainLoop ()->GetSceneManager ().GetActiveScene ()->AddActor (actor);
		}
	}

	//--------------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------------

	EditorUndoRedo::EditorUndoRedo (Editor* editor)
		: m_Editor (editor)
	{
	}

	//--------------------------------------------------------------------------------------------------

	EditorUndoRedo::~EditorUndoRedo () {}

	//--------------------------------------------------------------------------------------------------

	void EditorUndoRedo::PushCommand (UndoRedoCommand* command)
	{
		command->AtPush ();
		m_Commands.push (command);
	}

	//--------------------------------------------------------------------------------------------------

	void EditorUndoRedo::PopCommand ()
	{
		if (!m_Commands.empty ())
		{
			m_Commands.top ()->AtPop ();
			m_Commands.pop ();
		}
	}

	//--------------------------------------------------------------------------------------------------

	Editor* EditorUndoRedo::GetEditor () { return m_Editor; }

	//--------------------------------------------------------------------------------------------------
}
