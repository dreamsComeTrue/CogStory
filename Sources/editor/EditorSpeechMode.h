// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __EDITOR_SPEECH_MODE_H__
#define __EDITOR_SPEECH_MODE_H__

#include "Common.h"

namespace aga
{
    class Editor;

    class EditorSpeechMode
    {
        friend class Editor;

    public:
        EditorSpeechMode (Editor* editor);
        virtual ~EditorSpeechMode ();

    private:
        Editor* m_Editor;
    };
}

#endif //   __EDITOR_SPEECH_MODE_H__
