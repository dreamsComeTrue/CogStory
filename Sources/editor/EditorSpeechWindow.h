// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __EDITOR_SPEECH_WINDOW_H__
#define __EDITOR_SPEECH_WINDOW_H__

#include "Common.h"

#include <Gwork/Controls.h>
#include <Gwork/Controls/NumericUpDown.h>

namespace aga
{
    class Editor;

    class EditorSpeechWindow : public Gwk::Event::Handler
    {
    public:
        EditorSpeechWindow (Editor* editor, Gwk::Controls::Canvas* canvas);

        void Show ();
        void UpdateSpeechesTree ();
        Gwk::Controls::WindowControl* GetSceneWindow () { return m_SceneWindow; }

    private:
        void OnSave ();
        void OnRemove ();
        void OnOutcome ();
        void OnPreview ();

        void SelectSpeech (int id);

        void OnWindoClosed ();
        void OnAccept ();
        void OnCancel ();
        void OnSpeechSelect (Gwk::Controls::Base*);
        void OnNameEdit ();
        void OnLangSelected ();
        void OnRegionNameSelected ();
        void OnTextChanged ();

        void OnPositionTypeChanged ();
        void OnPositionXChanged ();
        void OnPositionYChanged ();

        void UpdateOutcomes ();

        void OnOutcomeIDTextChanged (Gwk::Controls::Base* control);
        void OnOutcomeDataTextChanged (Gwk::Controls::Base* control);
        void OnOutcomeActionChanged (Gwk::Controls::Base* control);
        void OnUpOutcome (Gwk::Controls::Base* control);
        void OnDownOutcome (Gwk::Controls::Base* control);
        void OnRemoveOutcome (Gwk::Controls::Base* control);

        void ClearControls ();

    private:
        Editor* m_Editor;

        int m_LangIndex;

        Gwk::Controls::WindowControl* m_SceneWindow;
        Gwk::Controls::TreeControl* m_SpeechesTree;
        Gwk::Controls::TextBox* m_IDTextBox;
        Gwk::Controls::TextBox* m_NameTextBox;
        Gwk::Controls::ComboBox* m_LanguageCombo;
        Gwk::Controls::ComboBox* m_RegionCombo;
        Gwk::Controls::NumericUpDown* m_MaxCharsInLineNumeric;
        Gwk::Controls::NumericUpDown* m_MaxLinesNumeric;
        Gwk::Controls::ComboBox* m_RelPositionCombo;
        Gwk::Controls::NumericUpDown* m_AbsPositionXNumeric;
        Gwk::Controls::NumericUpDown* m_AbsPositionYNumeric;
        Gwk::Controls::TextBoxMultiline* m_TextData;
        Gwk::Controls::ScrollControl* m_OutcomesContainer;
    };
}

#endif //   __EDITOR_SPEECH_WINDOW_H__
