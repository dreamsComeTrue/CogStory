// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "EditorSpeechWindow.h"
#include "Editor.h"
#include "MainLoop.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    EditorSpeechWindow::EditorSpeechWindow (Editor* editor, Gwk::Controls::Canvas* canvas)
        : m_Editor (editor)
    {
        m_LangIndex = 0;

        m_SceneWindow = new Gwk::Controls::WindowControl (canvas);
        m_SceneWindow->SetTitle ("Speech Editor");
        m_SceneWindow->SetSize (canvas->Width () - 150, canvas->Height () - 80);
        m_SceneWindow->CloseButtonPressed ();

        Gwk::Controls::DockBase* dock = new Gwk::Controls::DockBase (m_SceneWindow);
        dock->Dock (Gwk::Position::Fill);

        m_SpeechesTree = new Gwk::Controls::TreeControl (dock);
        m_SpeechesTree->SetBounds (10, 10, 300, dock->Height () - 55);
        m_SpeechesTree->ExpandAll ();
        m_SpeechesTree->Dock (Gwk::Position::Fill);
        m_SpeechesTree->onSelect.Add (this, &EditorSpeechWindow::OnSpeechSelect);

        dock->GetLeft ()->GetTabControl ()->AddPage ("Speeches", m_SpeechesTree);
        m_SpeechesTree->SetMargin (Gwk::Margin ());
        dock->GetLeft ()->GetTabControl ()->GetTab (0)->Hide ();

        int xOffset = 10;

        Gwk::Controls::Base* center = new Gwk::Controls::Base (dock);
        center->Dock (Gwk::Position::Fill);

        Gwk::Controls::Label* nameLabel = new Gwk::Controls::Label (center);
        nameLabel->SetPos (xOffset, 10);
        nameLabel->SetText ("Name:");
        nameLabel->SizeToContents ();

        int controlWidth = 610;

        m_NameTextBox = new Gwk::Controls::TextBox (center);
        m_NameTextBox->SetTextColor (Gwk::Colors::White);
        m_NameTextBox->SetText ("");
        m_NameTextBox->SetWidth (controlWidth);
        m_NameTextBox->SetPos (xOffset, nameLabel->Bottom () + 5);
        m_NameTextBox->onTextChanged.Add (this, &EditorSpeechWindow::OnNameEdit);

        Gwk::Controls::Label* langLabel = new Gwk::Controls::Label (center);
        langLabel->SetPos (xOffset, m_NameTextBox->Bottom () + 5);
        langLabel->SetText ("Lang:");
        langLabel->SizeToContents ();

        m_LanguageCombo = new Gwk::Controls::ComboBox (center);
        m_LanguageCombo->SetPos (xOffset, langLabel->Bottom () + 5);
        m_LanguageCombo->SetWidth (controlWidth);
        m_LanguageCombo->AddItem ("EN", "EN");
        m_LanguageCombo->AddItem ("PL", "PL");
        m_LanguageCombo->onSelection.Add (this, &EditorSpeechWindow::OnLangSelected);

        Gwk::Controls::Label* textLabel = new Gwk::Controls::Label (center);
        textLabel->SetPos (xOffset, m_LanguageCombo->Bottom () + 5);
        textLabel->SetText ("Text:");
        textLabel->SizeToContents ();

        m_TextData = new Gwk::Controls::TextBoxMultiline (center);
        m_TextData->SetTextColor (Gwk::Colors::White);
        m_TextData->SetPos (xOffset, textLabel->Bottom () + 5);
        m_TextData->SetSize (controlWidth, 70);
        m_TextData->onTextChanged.Add (this, &EditorSpeechWindow::OnTextChanged);

        Gwk::Controls::Label* regionLabel = new Gwk::Controls::Label (center);
        regionLabel->SetPos (xOffset, m_TextData->Bottom () + 5);
        regionLabel->SetText ("Region:");
        regionLabel->SizeToContents ();

        m_RegionCombo = new Gwk::Controls::ComboBox (center);
        m_RegionCombo->SetPos (regionLabel->Right () + 5, m_TextData->Bottom () + 5);
        m_RegionCombo->SetWidth (200);
        m_RegionCombo->AddItem ("", "");

        for (const std::string& s : g_SpeechFrames)
        {
            m_RegionCombo->AddItem (s, s);
        }

        m_RegionCombo->onSelection.Add (this, &EditorSpeechWindow::OnRegionNameSelected);

        Gwk::Controls::Label* maxCharsInLineLabel = new Gwk::Controls::Label (center);
        maxCharsInLineLabel->SetPos (m_RegionCombo->Right () + 10, m_TextData->Bottom () + 5);
        maxCharsInLineLabel->SetText ("Max chars/line:");
        maxCharsInLineLabel->SizeToContents ();

        m_MaxCharsInLineNumeric = new Gwk::Controls::NumericUpDown (center);
        m_MaxCharsInLineNumeric->SetTextColor (Gwk::Colors::White);
        m_MaxCharsInLineNumeric->SetMax (1000);
        m_MaxCharsInLineNumeric->SetIntValue (0);
        m_MaxCharsInLineNumeric->SetWidth (50);
        m_MaxCharsInLineNumeric->SetPos (maxCharsInLineLabel->Right () + 5, m_TextData->Bottom () + 5);
        m_MaxCharsInLineNumeric->onTextChanged.Add (this, &EditorSpeechWindow::OnMaxCharsChanged);

        Gwk::Controls::Label* maxLinesLabel = new Gwk::Controls::Label (center);
        maxLinesLabel->SetPos (m_MaxCharsInLineNumeric->Right () + 10, m_TextData->Bottom () + 5);
        maxLinesLabel->SetText ("Max lines:");
        maxLinesLabel->SizeToContents ();

        m_MaxLinesNumeric = new Gwk::Controls::NumericUpDown (center);
        m_MaxLinesNumeric->SetTextColor (Gwk::Colors::White);
        m_MaxLinesNumeric->SetIntValue (0);
        m_MaxLinesNumeric->SetWidth (50);
        m_MaxLinesNumeric->SetPos (maxLinesLabel->Right () + 5, m_TextData->Bottom () + 5);
        m_MaxLinesNumeric->onTextChanged.Add (this, &EditorSpeechWindow::OnMaxLinesChanged);

        Gwk::Controls::Label* relPosLabel = new Gwk::Controls::Label (center);
        relPosLabel->SetPos (xOffset, m_MaxLinesNumeric->Bottom () + 5);
        relPosLabel->SetText ("Position:");
        relPosLabel->SizeToContents ();

        m_RelPositionCombo = new Gwk::Controls::ComboBox (center);
        m_RelPositionCombo->SetPos (relPosLabel->Right () + 5, m_MaxLinesNumeric->Bottom () + 5);
        m_RelPositionCombo->SetWidth (195);

        for (const auto& s : g_SpeechFramePosition)
        {
            m_RelPositionCombo->AddItem (s.second, s.second);
        }

        m_RelPositionCombo->onSelection.Add (this, &EditorSpeechWindow::OnPositionTypeChanged);

        Gwk::Controls::Label* xPosLabel = new Gwk::Controls::Label (center);
        xPosLabel->SetPos (m_RelPositionCombo->Right () + 10, m_MaxLinesNumeric->Bottom () + 5);
        xPosLabel->SetText ("X:");
        xPosLabel->SizeToContents ();

        m_AbsPositionXNumeric = new Gwk::Controls::NumericUpDown (center);
        m_AbsPositionXNumeric->SetTextColor (Gwk::Colors::White);
        m_AbsPositionXNumeric->SetIntValue (0);
        m_AbsPositionXNumeric->SetMin (-10000);
        m_AbsPositionXNumeric->SetMax (10000);
        m_AbsPositionXNumeric->SetWidth (50);
        m_AbsPositionXNumeric->SetPos (xPosLabel->Right () + 5, m_MaxLinesNumeric->Bottom () + 5);
        m_AbsPositionXNumeric->onTextChanged.Add (this, &EditorSpeechWindow::OnPositionXChanged);

        Gwk::Controls::Label* yPosLabel = new Gwk::Controls::Label (center);
        yPosLabel->SetPos (m_AbsPositionXNumeric->Right () + 10, m_MaxLinesNumeric->Bottom () + 5);
        yPosLabel->SetText ("Y:");
        yPosLabel->SizeToContents ();

        m_AbsPositionYNumeric = new Gwk::Controls::NumericUpDown (center);
        m_AbsPositionYNumeric->SetTextColor (Gwk::Colors::White);
        m_AbsPositionYNumeric->SetIntValue (0);
        m_AbsPositionYNumeric->SetMin (-10000);
        m_AbsPositionYNumeric->SetMax (10000);
        m_AbsPositionYNumeric->SetWidth (50);
        m_AbsPositionYNumeric->SetPos (yPosLabel->Right () + 5, m_MaxLinesNumeric->Bottom () + 5);
        m_AbsPositionYNumeric->onTextChanged.Add (this, &EditorSpeechWindow::OnPositionYChanged);

        Gwk::Controls::Button* addSpeechButton = new Gwk::Controls::Button (center);
        addSpeechButton->SetText ("SAVE");
        addSpeechButton->SetWidth (155);
        addSpeechButton->SetPos (xOffset, m_AbsPositionYNumeric->Bottom () + 10);
        addSpeechButton->onPress.Add (this, &EditorSpeechWindow::OnSave);

        Gwk::Controls::Button* removeSpeechButton = new Gwk::Controls::Button (center);
        removeSpeechButton->SetText ("REMOVE");
        removeSpeechButton->SetWidth (155);
        removeSpeechButton->SetPos (addSpeechButton->Right () + 5, m_AbsPositionYNumeric->Bottom () + 10);
        removeSpeechButton->onPress.Add (this, &EditorSpeechWindow::OnRemove);

        Gwk::Controls::Button* outcomeButton = new Gwk::Controls::Button (center);
        outcomeButton->SetText ("OUTCOME");
        outcomeButton->SetWidth (160);
        outcomeButton->SetPos (removeSpeechButton->Right () + 5, m_AbsPositionYNumeric->Bottom () + 10);
        outcomeButton->onPress.Add (this, &EditorSpeechWindow::OnOutcome);

        m_OutcomesContainer = new Gwk::Controls::ScrollControl (center);
        m_OutcomesContainer->SetBounds (xOffset, outcomeButton->Bottom () + 10, 610, 190);

        Gwk::Controls::Button* acceptButton = new Gwk::Controls::Button (center);
        acceptButton->SetText ("ACCEPT");
        acceptButton->SetPos (m_SceneWindow->Width () - 330, m_SceneWindow->Height () - 85);
        acceptButton->onPress.Add (this, &EditorSpeechWindow::OnAccept);

        Gwk::Controls::Button* cancelButton = new Gwk::Controls::Button (center);
        cancelButton->SetText ("CANCEL");
        cancelButton->SetPos (m_SceneWindow->Width () - 330, acceptButton->Bottom () + 5);
        cancelButton->onPress.Add (this, &EditorSpeechWindow::OnCancel);
    }

    //--------------------------------------------------------------------------------------------------

    void EditorSpeechWindow::Show ()
    {
        m_SceneWindow->SetPosition (Gwk::Position::Center);
        m_SceneWindow->SetHidden (false);
        m_SceneWindow->MakeModal (true);
    }

    //--------------------------------------------------------------------------------------------------

    void EditorSpeechWindow::OnSave ()
    {
        SpeechData& speechData = m_Editor->GetEditorSpeechMode ().GetSpeechData ();

        for (const auto& pos : g_SpeechFramePosition)
        {
            if (pos.second == m_RelPositionCombo->GetSelectedItem ()->GetText ())
            {
                speechData.RelativeFramePosition = pos.first;
                break;
            }
        }

        speechData.ActorRegionName = m_RegionCombo->GetSelectedItem ()->GetText ();
        speechData.AbsoluteFramePosition.X = m_AbsPositionXNumeric->GetIntValue ();
        speechData.AbsoluteFramePosition.Y = m_AbsPositionYNumeric->GetIntValue ();

        if (m_NameTextBox->GetText () != "")
        {
            Gwk::Controls::Base::List& childNodes = m_SpeechesTree->GetChildNodes ();
            std::string oldName = "";

            for (Gwk::Controls::Base* control : childNodes)
            {
                Gwk::Controls::TreeNode* node = (Gwk::Controls::TreeNode*)control;

                if (node->IsSelected ())
                {
                    oldName = node->GetText ();
                    break;
                }
            }

            bool ret = m_Editor->GetEditorSpeechMode ().AddOrUpdateSpeech (oldName);

            if (ret)
            {
                m_NameTextBox->SetText ("");
                m_TextData->SetText ("");
                m_LanguageCombo->SelectItemByName ("EN");
                m_MaxCharsInLineNumeric->SetIntValue (0);
                m_MaxLinesNumeric->SetIntValue (0);
                m_RelPositionCombo->SetValue (g_SpeechFramePosition[BottomCenter]);
                m_AbsPositionXNumeric->SetIntValue (0);
                m_AbsPositionYNumeric->SetIntValue (0);
                m_Editor->GetEditorSpeechMode ().Clear ();

                UpdateSpeechesTree ();
                UpdateOutcomes ();
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    void EditorSpeechWindow::OnRemove ()
    {
        if (m_NameTextBox->GetText () != "")
        {
            m_Editor->GetEditorSpeechMode ().RemoveSpeech (m_NameTextBox->GetText ());
            UpdateSpeechesTree ();

            m_NameTextBox->SetText ("");
            m_TextData->SetText ("");
            m_LanguageCombo->SelectItemByName ("EN");
            m_MaxCharsInLineNumeric->SetIntValue (0);
            m_MaxLinesNumeric->SetIntValue (0);
            m_RelPositionCombo->SetValue (g_SpeechFramePosition[BottomCenter]);
            m_AbsPositionXNumeric->SetIntValue (0);
            m_AbsPositionYNumeric->SetIntValue (0);
            m_Editor->GetEditorSpeechMode ().Clear ();

            UpdateOutcomes ();
        }
    }

    //--------------------------------------------------------------------------------------------------

    void EditorSpeechWindow::OnOutcome ()
    {
        SpeechOutcome outcome;
        m_Editor->GetEditorSpeechMode ().GetSpeechData ().Outcomes[m_LangIndex].push_back (outcome);

        UpdateOutcomes ();
    }

    //--------------------------------------------------------------------------------------------------

    void EditorSpeechWindow::OnAccept () { m_SceneWindow->CloseButtonPressed (); }

    //--------------------------------------------------------------------------------------------------

    void EditorSpeechWindow::OnCancel () { m_SceneWindow->CloseButtonPressed (); }

    //--------------------------------------------------------------------------------------------------

    void EditorSpeechWindow::OnSpeechSelect (Gwk::Controls::Base* control)
    {
        Gwk::Controls::TreeNode* node = (Gwk::Controls::TreeNode*)control;

        if (node != nullptr && node->IsSelected ())
        {
            std::map<std::string, SpeechData>& speeches
                = m_Editor->GetMainLoop ()->GetSceneManager ().GetActiveScene ()->GetSpeeches ();
            SpeechData& speech = speeches[node->GetText ()];
            m_Editor->GetEditorSpeechMode ().SetSpeechData (speech);

            m_NameTextBox->SetText (speech.Name);
            m_RegionCombo->SelectItemByName (speech.ActorRegionName);
            m_TextData->SetText (speech.Text[m_LangIndex]);
            m_MaxCharsInLineNumeric->SetIntValue (speech.MaxCharsInLine);
            m_MaxLinesNumeric->SetIntValue (speech.MaxLines);
            m_RelPositionCombo->SelectItemByName (g_SpeechFramePosition[speech.RelativeFramePosition]);
            m_AbsPositionXNumeric->SetIntValue (speech.AbsoluteFramePosition.X);
            m_AbsPositionYNumeric->SetIntValue (speech.AbsoluteFramePosition.Y);

            UpdateOutcomes ();
        }
    }

    //--------------------------------------------------------------------------------------------------

    void EditorSpeechWindow::OnNameEdit ()
    {
        m_Editor->GetEditorSpeechMode ().GetSpeechData ().Name = m_NameTextBox->GetText ();
    }

    //--------------------------------------------------------------------------------------------------

    void EditorSpeechWindow::OnLangSelected ()
    {
        Gwk::Controls::Label* selItem = m_LanguageCombo->GetSelectedItem ();

        if (selItem->GetText () == "EN")
        {
            m_LangIndex = 0;
        }
        else if (selItem->GetText () == "PL")
        {
            m_LangIndex = 1;
        }

        m_TextData->SetText (m_Editor->GetEditorSpeechMode ().GetSpeechData ().Text[m_LangIndex]);
    }

    //--------------------------------------------------------------------------------------------------

    void EditorSpeechWindow::OnRegionNameSelected ()
    {
        m_Editor->GetEditorSpeechMode ().GetSpeechData ().ActorRegionName
            = m_RegionCombo->GetSelectedItem ()->GetText ();
    }

    //--------------------------------------------------------------------------------------------------

    void EditorSpeechWindow::OnTextChanged ()
    {
        m_Editor->GetEditorSpeechMode ().GetSpeechData ().Text[m_LangIndex] = m_TextData->GetText ();
    }

    //--------------------------------------------------------------------------------------------------

    void EditorSpeechWindow::OnMaxCharsChanged ()
    {
        m_Editor->GetEditorSpeechMode ().GetSpeechData ().MaxCharsInLine = m_MaxCharsInLineNumeric->GetIntValue ();
    }

    //--------------------------------------------------------------------------------------------------

    void EditorSpeechWindow::OnMaxLinesChanged ()
    {
        m_Editor->GetEditorSpeechMode ().GetSpeechData ().MaxLines = m_MaxLinesNumeric->GetIntValue ();
    }

    //--------------------------------------------------------------------------------------------------

    void EditorSpeechWindow::OnPositionTypeChanged ()
    {
        for (const auto& pos : g_SpeechFramePosition)
        {
            if (pos.second == m_RelPositionCombo->GetSelectedItem ()->GetText ())
            {
                m_Editor->GetEditorSpeechMode ().GetSpeechData ().RelativeFramePosition = pos.first;
                break;
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    void EditorSpeechWindow::OnPositionXChanged ()
    {
        m_Editor->GetEditorSpeechMode ().GetSpeechData ().AbsoluteFramePosition.X
            = m_AbsPositionXNumeric->GetIntValue ();
    }

    //--------------------------------------------------------------------------------------------------

    void EditorSpeechWindow::OnPositionYChanged ()
    {
        m_Editor->GetEditorSpeechMode ().GetSpeechData ().AbsoluteFramePosition.Y
            = m_AbsPositionYNumeric->GetIntValue ();
    }

    //--------------------------------------------------------------------------------------------------

    void EditorSpeechWindow::UpdateOutcomes ()
    {
        m_OutcomesContainer->RemoveAllChildren ();

        std::vector<SpeechOutcome>& outcomes = m_Editor->GetEditorSpeechMode ().GetSpeechData ().Outcomes[m_LangIndex];

        std::map<std::string, SpeechData>& speeches
            = m_Editor->GetMainLoop ()->GetSceneManager ().GetActiveScene ()->GetSpeeches ();

        int currentY = 0;
        for (int i = 0; i < outcomes.size (); ++i)
        {
            Gwk::Controls::TextBox* idTextBox = new Gwk::Controls::TextBox (m_OutcomesContainer);
            idTextBox->SetTextColor (Gwk::Colors::White);
            idTextBox->SetText (outcomes[i].Name);
            idTextBox->SetWidth (65);
            idTextBox->SetPos (0, currentY);
            idTextBox->onTextChanged.Add (this, &EditorSpeechWindow::OnOutcomeIDTextChanged);

            Gwk::Controls::TextBox* dataTextBox = new Gwk::Controls::TextBox (m_OutcomesContainer);
            dataTextBox->SetTextColor (Gwk::Colors::White);
            dataTextBox->SetText (outcomes[i].Text);
            dataTextBox->SetWidth (255);
            dataTextBox->SetPos (idTextBox->Right () + 5, currentY);
            dataTextBox->onTextChanged.Add (this, &EditorSpeechWindow::OnOutcomeDataTextChanged);

            Gwk::Controls::ComboBox* actionCombo = new Gwk::Controls::ComboBox (m_OutcomesContainer);
            actionCombo->SetPos (dataTextBox->Right () + 5, currentY);
            actionCombo->SetWidth (150);
            actionCombo->AddItem ("[CLOSE]", "[CLOSE]");

            for (std::map<std::string, SpeechData>::iterator it = speeches.begin (); it != speeches.end (); ++it)
            {
                actionCombo->AddItem ((*it).first, (*it).first);
            }

            actionCombo->SelectItemByName (outcomes[i].Action, false);
            actionCombo->onSelection.Add (this, &EditorSpeechWindow::OnOutcomeActionChanged);

            Gwk::Controls::Button* upButton = new Gwk::Controls::Button (m_OutcomesContainer);
            upButton->SetText (" ^");
            upButton->SetWidth (20);
            upButton->SetName (ToString (i));
            upButton->SetPos (actionCombo->Right () + 5, currentY);
            upButton->onPress.Add (this, &EditorSpeechWindow::OnUpOutcome);

            Gwk::Controls::Button* downButton = new Gwk::Controls::Button (m_OutcomesContainer);
            downButton->SetText (" v");
            downButton->SetWidth (20);
            downButton->SetName (ToString (i));
            downButton->SetPos (upButton->Right () + 5, currentY);
            downButton->onPress.Add (this, &EditorSpeechWindow::OnDownOutcome);

            Gwk::Controls::Button* removeButton = new Gwk::Controls::Button (m_OutcomesContainer);
            removeButton->SetText ("REMOVE");
            removeButton->SetWidth (60);
            removeButton->SetName (ToString (i));
            removeButton->SetPos (downButton->Right () + 5, currentY);
            removeButton->onPress.Add (this, &EditorSpeechWindow::OnRemoveOutcome);

            currentY += 22;
        }
    }

    //--------------------------------------------------------------------------------------------------

    void EditorSpeechWindow::UpdateSpeechesTree ()
    {
        m_SpeechesTree->Clear ();

        std::map<std::string, SpeechData>& speeches
            = m_Editor->GetMainLoop ()->GetSceneManager ().GetActiveScene ()->GetSpeeches ();

        for (std::map<std::string, SpeechData>::iterator it = speeches.begin (); it != speeches.end (); ++it)
        {
            Gwk::Controls::TreeNode* node = m_SpeechesTree->AddNode ((*it).first);
            node->onSelect.Add (this, &EditorSpeechWindow::OnSpeechSelect);
        }

        m_SpeechesTree->ExpandAll ();
    }

    //--------------------------------------------------------------------------------------------------

    void EditorSpeechWindow::OnOutcomeIDTextChanged (Gwk::Controls::Base* control)
    {
        std::vector<SpeechOutcome>& outcomes = m_Editor->GetEditorSpeechMode ().GetSpeechData ().Outcomes[m_LangIndex];

        int childIndex = 0;
        for (int i = 0; i < outcomes.size (); ++i)
        {
            Gwk::Controls::Base* child = m_OutcomesContainer->GetChild (childIndex);

            if (control == child)
            {
                outcomes[i].Name = ((Gwk::Controls::TextBox*)control)->GetText ();
                break;
            }

            childIndex += 6;
        }
    }

    //--------------------------------------------------------------------------------------------------

    void EditorSpeechWindow::OnOutcomeDataTextChanged (Gwk::Controls::Base* control)
    {
        std::vector<SpeechOutcome>& outcomes = m_Editor->GetEditorSpeechMode ().GetSpeechData ().Outcomes[m_LangIndex];

        int childIndex = 1;
        for (int i = 0; i < outcomes.size (); ++i)
        {
            Gwk::Controls::Base* child = m_OutcomesContainer->GetChild (childIndex);

            if (control == child)
            {
                outcomes[i].Text = ((Gwk::Controls::TextBox*)control)->GetText ();
                break;
            }

            childIndex += 6;
        }
    }

    //--------------------------------------------------------------------------------------------------

    void EditorSpeechWindow::OnOutcomeActionChanged (Gwk::Controls::Base* control)
    {
        std::vector<SpeechOutcome>& outcomes = m_Editor->GetEditorSpeechMode ().GetSpeechData ().Outcomes[m_LangIndex];

        int childIndex = 2;
        for (int i = 0; i < outcomes.size (); ++i)
        {
            Gwk::Controls::Base* child = m_OutcomesContainer->GetChild (childIndex);

            if (control == child)
            {
                Gwk::Controls::ComboBox* actionCombo = (Gwk::Controls::ComboBox*)control;

                outcomes[i].Action = actionCombo->GetSelectedItem ()->GetText ();
                break;
            }

            childIndex += 6;
        }

        UpdateOutcomes ();
    }

    //--------------------------------------------------------------------------------------------------

    void EditorSpeechWindow::OnUpOutcome (Gwk::Controls::Base* control)
    {
        std::vector<SpeechOutcome>& outcomes = m_Editor->GetEditorSpeechMode ().GetSpeechData ().Outcomes[m_LangIndex];

        int childIndex = 3;
        for (int i = 0; i < outcomes.size (); ++i)
        {
            Gwk::Controls::Base* child = m_OutcomesContainer->GetChild (childIndex);

            if (control == child)
            {
                if (i == 0)
                {
                    std::iter_swap (outcomes.begin (), outcomes.end () - 1);
                }
                else
                {
                    std::iter_swap (outcomes.begin () + i, outcomes.begin () + i - 1);
                }

                break;
            }

            childIndex += 6;
        }

        UpdateOutcomes ();
    }

    //--------------------------------------------------------------------------------------------------

    void EditorSpeechWindow::OnDownOutcome (Gwk::Controls::Base* control)
    {
        std::vector<SpeechOutcome>& outcomes = m_Editor->GetEditorSpeechMode ().GetSpeechData ().Outcomes[m_LangIndex];

        int childIndex = 4;
        for (int i = 0; i < outcomes.size (); ++i)
        {
            Gwk::Controls::Base* child = m_OutcomesContainer->GetChild (childIndex);

            if (control == child)
            {
                if (i == outcomes.size () - 1)
                {
                    std::iter_swap (outcomes.begin (), outcomes.end () - 1);
                }
                else
                {
                    std::iter_swap (outcomes.begin () + i, outcomes.begin () + i + 1);
                }

                break;
            }

            childIndex += 6;
        }

        UpdateOutcomes ();
    }

    //--------------------------------------------------------------------------------------------------

    void EditorSpeechWindow::OnRemoveOutcome (Gwk::Controls::Base* control)
    {
        std::vector<SpeechOutcome>& outcomes = m_Editor->GetEditorSpeechMode ().GetSpeechData ().Outcomes[m_LangIndex];

        int childIndex = 5;
        for (int i = 0; i < outcomes.size (); ++i)
        {
            Gwk::Controls::Base* child = m_OutcomesContainer->GetChild (childIndex);

            if (control == child)
            {
                outcomes.erase (outcomes.begin () + i);
                break;
            }

            childIndex += 6;
        }

        UpdateOutcomes ();
    }

    //--------------------------------------------------------------------------------------------------
}
