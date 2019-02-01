// Copyright 2017-2019 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

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

        SpeechData& GetSpeechData ();
        void SetSpeechData (SpeechData& data);

    private:
        Editor* m_Editor;
        SpeechData m_Speech;
    };
}

#endif //   __EDITOR_SPEECH_MODE_H__
