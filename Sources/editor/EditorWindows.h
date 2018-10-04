// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __EDITOR_WINDOWS_H__
#define __EDITOR_WINDOWS_H__

#include "Common.h"

#include <Gwork/Controls.h>

namespace aga
{
    class Editor;

    class EditorQuestionWindow : public Gwk::Event::Handler
    {
    public:
        EditorQuestionWindow (Editor* editor, Gwk::Controls::Canvas* canvas);

        void Show (const std::string& text, std::function<void(void)> OnYesFunc, std::function<void(void)> OnNoFunc);
        bool GetResult () { return m_Result; }

    private:
        void OnYes ();
        void OnNo ();

    private:
        Editor* m_Editor;
        Gwk::Controls::WindowControl* m_SceneWindow;
        Gwk::Controls::Label* m_Label;
        std::string m_Text;
        bool m_Result;

        std::function<void(void)> m_OnYesFunc;
        std::function<void(void)> m_OnNoFunc;
    };
}

#endif //   __EDITOR_WINDOWS_H__
