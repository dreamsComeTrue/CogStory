// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __EDITOR_ACTOR_MODE_H__
#define __EDITOR_ACTOR_MODE_H__

#include "Common.h"
#include "Scene.h"

namespace aga
{
    class Editor;
    struct SpeechWindow;

    class EditorActorMode
    {
        friend class Editor;
        friend class EditorSpeechWindow;

    public:
        EditorActorMode (Editor* editor);
        virtual ~EditorActorMode ();

        bool AddOrUpdateSpeech (const std::string& oldName);
        void RemoveSpeech (const std::string& name);
        void Clear ();

    private:
        Editor* m_Editor;
        SpeechData m_Speech;
    };
}

#endif //   __EDITOR_ACTOR_MODE_H__
