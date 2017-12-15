// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "EditorWindows.h"
#include "Editor.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    EditorInfoWindow::EditorInfoWindow (Editor* editor, Gwk::Controls::Canvas* canvas)
    {
        m_Editor = editor;

        m_SceneWindow = new Gwk::Controls::WindowControl (canvas);
        m_SceneWindow->SetTitle ("Info");
        m_SceneWindow->SetSize (450, 120);
        m_SceneWindow->CloseButtonPressed ();

        m_Label = new Gwk::Controls::Label (m_SceneWindow);
        m_Label->SetBounds (20, 10, m_SceneWindow->Width () - 40, m_SceneWindow->Height () - 40);
        m_Label->SetText (m_Text);

        Gwk::Controls::Button* okButton = new Gwk::Controls::Button (m_SceneWindow);
        okButton->SetText ("OK");
        okButton->SetPos (m_SceneWindow->Width () / 2 - 50, m_SceneWindow->Height () - 60);
        okButton->onPress.Add (this, &EditorInfoWindow::OnAccept);
    }

    //--------------------------------------------------------------------------------------------------

    void EditorInfoWindow::Show (const std::string& text)
    {
        m_Text = text;

        m_Label->SetText (m_Text);
        m_Label->SetWrap (true);
        m_SceneWindow->SetPosition (Gwk::Position::Center);
        m_SceneWindow->SetHidden (false);
        m_SceneWindow->MakeModal (true);
    }

    //--------------------------------------------------------------------------------------------------

    void EditorInfoWindow::OnAccept () { m_SceneWindow->CloseButtonPressed (); }

    //--------------------------------------------------------------------------------------------------
    //--------------------------------------------------------------------------------------------------

    EditorQuestionWindow::EditorQuestionWindow (Editor* editor, Gwk::Controls::Canvas* canvas)
    {
        m_Result = true;
        m_Editor = editor;

        m_SceneWindow = new Gwk::Controls::WindowControl (canvas);
        m_SceneWindow->SetTitle ("Question");
        m_SceneWindow->SetSize (350, 120);
        m_SceneWindow->CloseButtonPressed ();

        m_Label = new Gwk::Controls::Label (m_SceneWindow);
        m_Label->SetBounds (20, 10, m_SceneWindow->Width () - 40, m_SceneWindow->Height () - 40);
        m_Label->SetText (m_Text);

        Gwk::Controls::Button* yesButton = new Gwk::Controls::Button (m_SceneWindow);
        yesButton->SetText ("YES");
        yesButton->SetPos (m_SceneWindow->Width () / 2 - 2 * 50 - 5, m_SceneWindow->Height () - 60);
        yesButton->onPress.Add (this, &EditorQuestionWindow::OnYes);

        Gwk::Controls::Button* noButton = new Gwk::Controls::Button (m_SceneWindow);
        noButton->SetText ("NO");
        noButton->SetPos (yesButton->Right () + 10, m_SceneWindow->Height () - 60);
        noButton->onPress.Add (this, &EditorQuestionWindow::OnNo);
    }

    //--------------------------------------------------------------------------------------------------

    void EditorQuestionWindow::Show (
        const std::string& text, std::function<void(void)> OnYesFunc, std::function<void(void)> OnNoFunc)
    {
        m_Text = text;
        m_OnYesFunc = OnYesFunc;
        m_OnNoFunc = OnNoFunc;

        m_Label->SetText (m_Text);
        m_Label->SetWrap (true);
        m_SceneWindow->SetPosition (Gwk::Position::Center);
        m_SceneWindow->SetHidden (false);
        m_SceneWindow->MakeModal (true);
    }

    //--------------------------------------------------------------------------------------------------

    void EditorQuestionWindow::OnYes ()
    {
        m_Result = true;

        if (m_OnYesFunc)
        {
            m_OnYesFunc ();
        }

        m_SceneWindow->CloseButtonPressed ();
    }

    //--------------------------------------------------------------------------------------------------

    void EditorQuestionWindow::OnNo ()
    {
        m_Result = false;

        if (m_OnNoFunc)
        {
            m_OnNoFunc ();
        }

        m_SceneWindow->CloseButtonPressed ();
    }

    //--------------------------------------------------------------------------------------------------

    bool EditorQuestionWindow::GetResult () { return m_Result; }

    //--------------------------------------------------------------------------------------------------
}
