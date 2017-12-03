// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __EDITOR_SPEECH_MODE_H__
#define __EDITOR_SPEECH_MODE_H__

#include "Common.h"
#include "Scene.h"

namespace aga
{
    class Editor;
    struct SpeechWindow;

    class EditorSpeechMode
    {
        friend class Editor;
        friend class SpeechWindow;

    public:
        EditorSpeechMode (Editor* editor);
        virtual ~EditorSpeechMode ();

        bool AddOrUpdateSpeech (const std::string& oldName);
        void RemoveSpeech (const std::string& name);
        void Clear ();

    private:
        Editor* m_Editor;
        SpeechData m_Speech;
    };
}

#endif //   __EDITOR_SPEECH_MODE_H__
