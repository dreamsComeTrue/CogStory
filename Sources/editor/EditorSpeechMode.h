// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __EDITOR_SPEECH_MODE_H__
#define __EDITOR_SPEECH_MODE_H__

#include "Common.h"
#include "Scene.h"

namespace aga
{
    class Editor;
    class SpeechWindow;

    class EditorSpeechMode
    {
    public:
        EditorSpeechMode (Editor* editor);
        virtual ~EditorSpeechMode ();

        bool AddOrUpdateSpeech ();
        void RemoveSpeech (int id);
        void Clear ();

        SpeechData& GetSpeechData () { return m_Speech; }
        void SetSpeechData (SpeechData& data) { m_Speech = data; }

    private:
        Editor* m_Editor;
        SpeechData m_Speech;
    };
}

#endif //   __EDITOR_SPEECH_MODE_H__
