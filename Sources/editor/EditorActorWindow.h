// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __EDITOR_ACTOR_WINDOW_H__
#define __EDITOR_ACTOR_WINDOW_H__

#include "Common.h"

namespace aga
{
    class Editor;

    class EditorActorWindow : public Gwk::Event::Handler
    {
    public:
        EditorActorWindow (Editor* editor, Gwk::Controls::Canvas* canvas);

        void Show ();

        void OnSave ();
        void OnRemove ();

        void OnAccept ();
        void OnCancel ();

    private:
        Editor* m_Editor;

        int m_LangIndex;

        Gwk::Controls::WindowControl* m_SceneWindow;
        Gwk::Controls::TreeControl* m_SpeechesTree;
        Gwk::Controls::TextBox* m_NameTextBox;
        Gwk::Controls::ComboBox* m_LanguageCombo;
        Gwk::Controls::TextBoxMultiline* m_TextData;
        Gwk::Controls::ScrollControl* m_OutcomesContainer;
    };
}

#endif //   __EDITOR_ACTOR_WINDOW_H__
