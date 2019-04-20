// Copyright 2017-2019 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __EDITOR_SPEECH_WINDOW_H__
#define __EDITOR_SPEECH_WINDOW_H__

#include "Common.h"

namespace aga
{
    class Editor;

    struct EditorSpeechOutcome
    {
        char Name[50] = {0};
        char Text[50] = {0};
        char Action[50] = {0};
        int ActionIndex = 0;
    };

    class EditorSpeechWindow
    {
    public:
        EditorSpeechWindow (Editor* editor);

        void Show ();
        void RenderUI ();

        bool IsVisible () const;

    private:
        void OnSave ();
        void OnRemove ();
        void OnOutcome ();
        void OnPreview ();

        void SelectSpeech (long id);

        void OnWindoClosed ();

        void OnPositionTypeChanged ();

        void UpdateOutcomes ();

        void ClearControls ();

    private:
        Editor* m_Editor;
        bool m_IsVisible;

        char m_SpeechID[10];
        char m_SpeechName[100];

        char m_SpeechGroup[100];

        int m_LangIndex;
        std::vector<std::string> m_Languages;

        int m_SelectedRegion;
        std::vector<std::string> m_Regions;

        int m_MaxChars;
        int m_MaxLines;
        int m_Speed;

        int m_AbsPosX;
        int m_AbsPosY;

        int m_RelPosition;
        std::vector<std::string> m_Positions;

        int m_Action;
        std::vector<std::string> m_Actions;

        char m_Text[2000];

        std::vector<EditorSpeechOutcome> m_SpeechOutcomes;
    };
}

#endif //   __EDITOR_SPEECH_WINDOW_H__
