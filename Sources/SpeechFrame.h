// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __SPEECH_FRAME_H__
#define __SPEECH_FRAME_H__

#include "Common.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    const std::string REGISTERED_CHOICE_PREFIX = "*";

    const int SPEECH_FRAME_TEXT_INSETS = 10;
    const int SPEECH_FRAME_LINE_OFFSET = 5;
    const int SPEECH_FRAME_ADVANCE_LETTERS = 4;
    const int SPEECH_FRAME_ADVANCE_SPACE = 8;
    const int SPEECH_FRAME_ADVANCE_CHOICE = 30;
    const float SPEECH_FRAME_MAX_CHAR_EDGE_LENGTH = 64;

    class Atlas;
    class SpeechFrameManager;

#define ATTRIBUTE_COLOR 1
#define ATTRIBUTE_DELAY 2

    struct SpeechTextAttribute
    {
        int LineIndex;
        int BeginIndex;
        int EndIndex;
        ALLEGRO_COLOR Color;
        float Delay = 0.0f;

        int AttributesMask = 0;
    };

    struct SpeechChoice
    {
        std::string Text;
        std::string Action;
        std::function<void()> Func;
        asIScriptFunction* ScriptFunc = nullptr;
    };

    class SpeechFrame
    {
    public:
        SpeechFrame (SpeechFrameManager* manager);
        SpeechFrame (SpeechFrameManager* manager, const std::string& text, Rect rect, bool shouldBeHandled = true,
                     const std::string& regionName = "");
        virtual ~SpeechFrame ();

        bool Update (float deltaTime);
        void Render (float deltaTime);

        void ProcessEvent (ALLEGRO_EVENT* event, float deltaTime);

        void SetText (const std::string& text);

        void AddChoice (const std::string& text, const std::string& action, std::function<void()> func);

        void SetDrawRect (Rect rect);
        void Show ();
        void Hide ();
        bool IsVisible () const;

        void SetDrawTextCenter (bool center);
        bool IsDrawTextCenter () const;

        void SetDrawSpeed (float speedInMs);

        bool IsShouldBeHandled ();
        bool IsHandled ();

        static Point GetActorRegionOffset ();

        std::function<void()> ChoiceUpFunction;
        std::function<void()> ChoiceDownFunction;
        std::function<void()> ScrollUpFunction;
        std::function<void()> ScrollDownFunction;
        std::function<void()> HandledFunction;

        std::string GetOutcomeAction () { return m_OutcomeAction; }

    private:
        void DrawActorSprite ();
        void DrawTextLine (const std::string& line, int currentCharIndex, Point drawPoint, int advance);
        void DrawChoiceArrow (float yOffset);
        void DrawScrollArrows (int currentDrawingLine);

        Point GetNextDrawPoint (int lineIndex);
        float GetTextAdvance (int lineCounter);
        int GetTextAlign ();
        int GetMaxLinesCanFit ();

        void MoveChoiceUp ();
        void MoveChoiceDown ();
        void HandleKeyUp ();
        void HandleKeyDown ();

        void UpdateScrollArrowsFlash (float deltaTime);
        void UpdateTextPosition (float deltaTime);
        bool IsTextFitWithoutScroll ();
        void PreprocessText (std::string& text);
        int GetCurrentDrawingLine ();
        std::vector<std::string> BreakLine (const std::string& line, float maxWidth);

    private:
        SpeechFrameManager* m_Manager;
        NINE_PATCH_BITMAP* m_FrameBitmap;
        Atlas* m_Atlas;
        std::string m_ActorRegionName;

        std::vector<SpeechChoice> m_Choices;
        int m_ActualChoiceIndex;

        std::string m_Text;
        std::vector<std::string> m_TextLines;
        std::vector<SpeechTextAttribute> m_Attributes;
        Rect m_DrawRect;
        bool m_Visible;
        bool m_DrawTextCenter;

        bool m_StillUpdating;

        float m_DrawSpeed;
        float m_CurrentDrawTime;

        float m_LineHeight;
        size_t m_CurrentIndex;
        size_t m_CurrentLine;

        float m_CurrentFlashTime;
        float m_ArrowDrawSpeed;
        bool m_DrawLightArrow;

        int m_ChosenLineDelta;

        float m_KeyDelta;
        float m_MaxKeyDelta;
        bool m_KeyEventHandled;

        bool m_ScrollPossible;
        bool m_ShouldBeHandled;
        bool m_Handled;

        float m_DelayCounter;
        bool m_IsDelayed;

        std::string m_OutcomeAction;
    };
}

#endif //   __SPEECH_FRAME_H__
