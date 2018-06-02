// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __EDITOR_WINDOWS_H__
#define __EDITOR_WINDOWS_H__

#include "Common.h"

#include <Gwork/Controls.h>

namespace aga
{
    class Editor;

    class EditorInfoWindow : public Gwk::Event::Handler
    {
    public:
        EditorInfoWindow (Editor* editor, Gwk::Controls::Canvas* canvas);

        void Show (const std::string& text);
        void OnAccept ();

    private:
        Editor* m_Editor;
        Gwk::Controls::WindowControl* m_SceneWindow;
        Gwk::Controls::Label* m_Label;
        Gwk::Controls::Button* m_OKButton;
        std::string m_Text;
    };

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

    class EditorInputWindow : public Gwk::Event::Handler
    {
    public:
        EditorInputWindow (Editor* editor, Gwk::Controls::Canvas* canvas);

        void Show (const std::string& labelText, const std::string& inputText, std::function<void(void)> OnYesFunc,
                   std::function<void(void)> OnNoFunc);
        bool GetResult ();
        std::string GetText () const;

    private:
        void OnEdit (Gwk::Controls::Base* control);
        void OnYes ();
        void OnNo ();

    private:
        Editor* m_Editor;
        Gwk::Controls::WindowControl* m_SceneWindow;
        Gwk::Controls::Label* m_Label;
        Gwk::Controls::TextBox* m_TextBox;
        std::string m_Text;
        bool m_Result;

        std::function<void(void)> m_OnYesFunc;
        std::function<void(void)> m_OnNoFunc;
    };
}

#endif //   __EDITOR_WINDOWS_H__
