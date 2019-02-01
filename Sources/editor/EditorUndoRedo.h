// Copyright 2017-2019 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __EDITOR_UNDO_REDO_H__
#define __EDITOR_UNDO_REDO_H__

#include "Common.h"

namespace aga
{
	class Editor;
    class EditorUndoRedo;
    
    class Actor;

	class UndoRedoCommand
	{
	public:
		UndoRedoCommand (EditorUndoRedo* undoRedo);

		virtual void AtPush () = 0;
		virtual void AtPop () = 0;

	protected:
		EditorUndoRedo* m_UndoRedo;
	};
	
	class MoveActorsCommand : public UndoRedoCommand
	{
	public:
		MoveActorsCommand (EditorUndoRedo* undoRedo, std::vector<Actor*> actors);

		void AtPush ();
		void AtPop ();

	private:
		std::vector<Actor*> m_Actors;
		std::vector<Point> m_Positions;
	};	

	class RemoveActorsCommand : public UndoRedoCommand
	{
	public:
		RemoveActorsCommand (EditorUndoRedo* undoRedo, std::vector<Actor*> actors);

		void AtPush ();
		void AtPop ();

	private:
		std::vector<Actor*> m_Actors;
	};

	class EditorUndoRedo
	{
	public:
		EditorUndoRedo (Editor* editor);
		virtual ~EditorUndoRedo ();

		void PushCommand (UndoRedoCommand* command);
		void PopCommand ();
        
        Editor* GetEditor ();

	private:
		Editor* m_Editor;
		std::stack<UndoRedoCommand*> m_Commands;
	};
}

#endif //   __EDITOR_UNDO_REDO_H__
