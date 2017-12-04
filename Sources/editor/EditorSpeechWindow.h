// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __EDITOR_SPEECH_WINDOW_H__
#define __EDITOR_SPEECH_WINDOW_H__

#include "Common.h"

#include <Gwork/Controls.h>
#include <Gwork/Controls/WindowControl.h>

namespace aga
{
    class Editor;

    class SpeechWindow : public Gwk::Event::Handler
    {
    public:
        SpeechWindow (Editor* editor, Gwk::Controls::Canvas* canvas);

        void Show ();

        void OnSave ();
        void OnRemove ();
        void OnOutcome ();

        void OnAccept ();
        void OnCancel ();
        void OnSpeechSelect (Gwk::Controls::Base*);
        void OnNameEdit ();
        void OnLangSelected ();
        void OnTextChanged ();

        void UpdateOutcomes ();
        void UpdateSpeechesTree ();

        void OnOutcomeIDTextChanged (Gwk::Controls::Base* control);
        void OnOutcomeDataTextChanged (Gwk::Controls::Base* control);
        void OnOutcomeActionChanged (Gwk::Controls::Base* control);
        void OnUpOutcome (Gwk::Controls::Base* control);
        void OnDownOutcome (Gwk::Controls::Base* control);
        void OnRemoveOutcome (Gwk::Controls::Base* control);

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

#endif //   __EDITOR_SPEECH_WINDOW_H__
