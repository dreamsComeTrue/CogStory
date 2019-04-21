// Copyright 2017-2019 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "SpeechFrame.h"
#include "Atlas.h"
#include "AudioSample.h"
#include "MainLoop.h"
#include "Resources.h"
#include "Scene.h"
#include "Screen.h"
#include "SpeechFrameManager.h"

namespace aga
{
	//--------------------------------------------------------------------------------------------------

	SpeechFrame::SpeechFrame (SpeechFrameManager* manager)
		: SpeechFrame (manager, "", Rect (), true, "")
	{
	}

	//--------------------------------------------------------------------------------------------------

	SpeechFrame::SpeechFrame (SpeechFrameManager* manager, const std::string& text, Rect rect, bool shouldBeHandled,
		const std::string& actionName, const std::string& regionName)
		: m_Manager (manager)
		, m_ActorRegionName (regionName)
		, m_Action (actionName)
		, m_ActualChoiceIndex (0)
		, m_DrawRect (rect)
		, m_Visible (true)
		, m_DrawTextCenter (false)
		, m_StillUpdating (true)
		, m_DrawSpeed (SPEECH_FRAME_DEFAULT_SPEED)
		, m_DrawTimeAccumulator (0)
		, m_CurrentIndexInLine (0)
		, m_CurrentLine (0)
		, m_CurrentCharIndex (0)
		, m_ArrowDrawSpeed (300)
		, m_DrawLightArrow (true)
		, m_ChosenLineDelta (0)
		, m_MaxKeyDelta (200)
		, m_KeyEventHandled (false)
		, m_ShouldBeHandled (shouldBeHandled)
		, m_Handled (false)
		, m_DelayCounter (0.0f)
		, m_IsDelayed (false)
		, m_AttrColorIndex (0)
		, m_AttrDelayIndex (0)
		, m_ScriptHandleFunction (nullptr)
		, m_IsSuspended (false)
		, m_OverrideSuspension (false)
		, m_CurrentLineBreakOffset (0)
	{
		AtlasManager& atlasManager = m_Manager->GetSceneManager ()->GetMainLoop ()->GetAtlasManager ();
		Atlas* atlas = atlasManager.GetAtlas (GetBaseName (GetResourcePath (PACK_MENU_UI)));
		Rect atlasRect = atlas->GetRegion ("text_frame").Bounds;
		ALLEGRO_BITMAP* bmp = al_create_sub_bitmap (atlas->GetImage (), static_cast<int> (atlasRect.GetPos ().X),
			static_cast<int> (atlasRect.GetPos ().Y), static_cast<int> (atlasRect.GetSize ().Width),
			static_cast<int> (atlasRect.GetSize ().Height));

		m_FrameBitmap = create_nine_patch_bitmap (bmp, true);

		if (m_ActorRegionName != "")
		{
			m_Atlas = atlasManager.GetAtlas (GetBaseName (GetResourcePath (PACK_ACTORS_UI)));
		}

		Font& font = m_Manager->GetSceneManager ()->GetMainLoop ()->GetScreen ()->GetFont ();
		m_LineAscent = font.GetFontAscent (FONT_NAME_SPEECH_FRAME);
		m_LineDescent = font.GetFontDescent (FONT_NAME_SPEECH_FRAME);

		m_LineHeight = m_LineAscent + m_LineDescent;

		SetText (text);
	}

	//--------------------------------------------------------------------------------------------------

	SpeechFrame::~SpeechFrame ()
	{
		if (m_FrameBitmap)
		{
			destroy_nine_patch_bitmap (m_FrameBitmap);
		}
	}

	//--------------------------------------------------------------------------------------------------

	size_t SpeechFrame::GetMaxLinesCanFit ()
	{
		float areaWithoutInsets = (m_DrawRect.GetSize ().Height - 2 * SPEECH_FRAME_TEXT_INSETS);
		float currentHeight = 0.f;

		for (int maxLines = 0;; ++maxLines)
		{
			currentHeight += m_LineAscent;

			if (currentHeight > areaWithoutInsets)
			{
				return maxLines;
			}

			currentHeight += m_LineDescent;
		}
	}

	//--------------------------------------------------------------------------------------------------

	void SpeechFrame::MoveChoiceUp ()
	{
		--m_ActualChoiceIndex;

		if (m_ActualChoiceIndex < 0)
		{
			m_ActualChoiceIndex = static_cast<int> (m_Choices.size () - 1);
		}

		if (ChoiceUpFunction)
		{
			ChoiceUpFunction ();
		}
	}

	//--------------------------------------------------------------------------------------------------

	void SpeechFrame::MoveChoiceDown ()
	{
		++m_ActualChoiceIndex;

		if (m_ActualChoiceIndex > static_cast<int> (m_Choices.size () - 1))
		{
			m_ActualChoiceIndex = 0;
		}

		if (ChoiceDownFunction)
		{
			ChoiceDownFunction ();
		}
	}

	//--------------------------------------------------------------------------------------------------

	void SpeechFrame::HandleKeyUp ()
	{
		if (!m_IsSuspended && !m_StillUpdating)
		{
			//  We can only scroll, when there are no choices
			if (!m_Choices.empty () && m_CurrentLine >= m_TextLines.size () - m_Choices.size ())
			{
				MoveChoiceUp ();
			}
			else if (!IsTextFitWithoutScroll ())
			{
				if (GetCurrentDrawingLine () > 0)
				{
					if (ScrollUpFunction)
					{
						ScrollUpFunction ();
					}
				}

				--m_ChosenLineDelta;
			}
		}
	}

	//--------------------------------------------------------------------------------------------------

	void SpeechFrame::HandleKeyDown ()
	{
		if (!m_IsSuspended && !m_StillUpdating)
		{
			if (!m_Choices.empty () && m_CurrentLine >= m_TextLines.size () - m_Choices.size ())
			{
				MoveChoiceDown ();
			}
			else if (!IsTextFitWithoutScroll ())
			{
				int diff = m_CurrentLine + 1 - GetMaxLinesCanFit ();

				if (GetCurrentDrawingLine () < diff)
				{
					if (ScrollDownFunction)
					{
						ScrollDownFunction ();
					}
				}

				++m_ChosenLineDelta;
			}
		}
	}

	//--------------------------------------------------------------------------------------------------

	void SpeechFrame::UpdateScrollArrowsFlash (float deltaTime)
	{
		m_CurrentFlashTime += deltaTime * 1000;

		if (m_CurrentFlashTime >= m_ArrowDrawSpeed)
		{
			m_CurrentFlashTime = 0;
			m_DrawLightArrow = !m_DrawLightArrow;
		}
	}

	//--------------------------------------------------------------------------------------------------

	void SpeechFrame::UpdateTextPosition (float deltaTime)
	{
		if (m_StillUpdating && !m_IsSuspended && !m_TextLines.empty ())
		{
			//  Handle [WAIT] annotation
			UpdateWaitTime (deltaTime);

			m_DrawTimeAccumulator += deltaTime * 1000;

			//  Update each character
			if (m_DrawTimeAccumulator >= m_DrawSpeed)
			{
				PlayTypeWriterSound ();

				m_DrawTimeAccumulator = 0.0f;

				if (!m_IsDelayed)
				{
					++m_CurrentCharIndex;
					++m_CurrentIndexInLine;
					++m_AttrDelayIndex;
				}

				//  Try to switch to the next line
				if (m_CurrentIndexInLine >= m_TextLines[m_CurrentLine].size ())
				{
					bool drawnLastLine = (m_CurrentLine + 1) % GetMaxLinesCanFit () == 0;
					bool notOnLastLine = !(m_CurrentLine + 1 >= m_TextLines.size ());

					if (!m_OverrideSuspension && notOnLastLine && drawnLastLine)
					{
						m_IsSuspended = true;
						m_OverrideSuspension = false;
					}

					if (!m_IsSuspended)
					{
						m_CurrentIndexInLine = 0;
						++m_CurrentLine;
					}
				}

				//  Are we at the end of processing?
				if (!m_IsSuspended && m_CurrentLine >= m_TextLines.size ())
				{
					m_StillUpdating = false;
					m_CurrentLine = m_TextLines.size () - 1;
					m_CurrentIndexInLine = m_TextLines[m_CurrentLine].size ();
				}

				TryToSuspendOnBreakPoints ();
			}
		}
	}

	//--------------------------------------------------------------------------------------------------

	void SpeechFrame::UpdateWaitTime (float deltaTime)
	{
		if (m_IsDelayed)
		{
			m_DelayCounter -= deltaTime * 1000.0f;

			if (m_DelayCounter <= 0.0f)
			{
				m_IsDelayed = false;
				m_DelayCounter = 0.0f;
			}
		}
	}

	//--------------------------------------------------------------------------------------------------

	void SpeechFrame::PlayTypeWriterSound ()
	{
		if (m_CurrentCharIndex % 7 == 0)
		{
			m_Manager->GetTypeSample ()->Play ();
		}
	}

	//--------------------------------------------------------------------------------------------------

	void SpeechFrame::TryToSuspendOnBreakPoints ()
	{
		for (size_t breakPoint : m_BreakPoints)
		{
			if (m_CurrentCharIndex == breakPoint)
			{
				m_IsSuspended = true;
				m_OverrideSuspension = false;

				break;
			}
		}
	}

	//--------------------------------------------------------------------------------------------------

	bool SpeechFrame::Update (float deltaTime)
	{
		if (m_KeyEventHandled)
		{
			m_KeyEventHandled = false;
			m_KeyDelta = 0.0f;
		}
		else
		{
			m_KeyDelta += deltaTime * 1000;
		}

		if ( //! IsTextFitWithoutScroll () &&
			m_KeyDelta > m_MaxKeyDelta)
		{
			ALLEGRO_KEYBOARD_STATE state;
			al_get_keyboard_state (&state);

			if (al_key_down (&state, ALLEGRO_KEY_UP) || al_key_down (&state, ALLEGRO_KEY_W))
			{
				HandleKeyUp ();
			}

			if (al_key_down (&state, ALLEGRO_KEY_DOWN) || al_key_down (&state, ALLEGRO_KEY_S))
			{
				HandleKeyDown ();
			}

			m_KeyDelta = 0.0f;
		}

		UpdateScrollArrowsFlash (deltaTime);
		UpdateTextPosition (deltaTime);

		return true;
	}

	//--------------------------------------------------------------------------------------------------

	void SpeechFrame::ProcessEvent (ALLEGRO_EVENT* event, float)
	{
		if (event->type == ALLEGRO_EVENT_JOYSTICK_BUTTON_UP)
		{
			if (event->joystick.button == 2)
			{
				HandleAction ();
			}
		}

		if (event->type == ALLEGRO_EVENT_JOYSTICK_AXIS)
		{
			if (event->joystick.pos < -0.7f)
			{
				HandleKeyUp ();
				m_KeyEventHandled = true;
			}
			if (event->joystick.pos > 0.7f)
			{
				HandleKeyDown ();
				m_KeyEventHandled = true;
			}
		}

		if (event->type == ALLEGRO_EVENT_KEY_DOWN)
		{
			switch (event->keyboard.keycode)
			{
			case ALLEGRO_KEY_UP:
			case ALLEGRO_KEY_W:
			{
				HandleKeyUp ();
				m_KeyEventHandled = true;

				break;
			}

			case ALLEGRO_KEY_DOWN:
			case ALLEGRO_KEY_S:
			{
				HandleKeyDown ();
				m_KeyEventHandled = true;

				break;
			}

			case ALLEGRO_KEY_ENTER:
			case ALLEGRO_KEY_SPACE:
			case ALLEGRO_KEY_X:
			{
				HandleAction ();

				break;
			}
			}
		}
	}

	//--------------------------------------------------------------------------------------------------

	void SpeechFrame::HandleAction ()
	{
		if (m_IsSuspended)
		{
			m_IsSuspended = false;
			m_OverrideSuspension = true;
			m_CurrentLineBreakOffset = m_CurrentLine;

			//  After resume - do we still need to update?
			if (m_CurrentLine >= m_TextLines.size () - 1)
			{
				m_StillUpdating = false;
			}
		}

		if (m_ShouldBeHandled && !m_StillUpdating)
		{
			if (!m_Choices.empty ())
			{
				SpeechChoice& choice = m_Choices[m_ActualChoiceIndex];

				std::function<void()>& handler = choice.Func;

				if (handler)
				{
					handler ();
				}

				//  Check, if we are using plain OUTCOME reference,
				//  or one with REGISTERED_CHOICE_PREFIX (usually '*' mark) as the prefix
				//  which coresponds to function registered with 'RegisterChoiceFunction'
				if (StartsWith (choice.Action, REGISTERED_CHOICE_PREFIX))
				{
					std::string actionName = choice.Action.substr (REGISTERED_CHOICE_PREFIX.length ());
					std::map<std::string, asIScriptFunction*>& choiceFunctions
						= m_Manager->GetSceneManager ()->GetActiveScene ()->GetChoiceFunctions ();
					asIScriptFunction* func = choiceFunctions[actionName];

					if (func)
					{
						asIScriptContext* ctx
							= m_Manager->GetSceneManager ()->GetMainLoop ()->GetScriptManager ().GetContext ();
						ctx->Prepare (func);

						if (ctx->Execute () == asEXECUTION_FINISHED)
						{
							m_OutcomeAction = *static_cast<std::string*> (ctx->GetReturnObject ());
						}

						ctx->Unprepare ();
						ctx->GetEngine ()->ReturnContext (ctx);
					}
				}
				else
				{
					m_OutcomeAction = choice.Action;
				}
			}
			else
			{
				m_OutcomeAction = m_Action;
			}

			m_Handled = true;

			if (m_ScriptHandleFunction)
			{
				asIScriptContext* ctx
					= m_Manager->GetSceneManager ()->GetMainLoop ()->GetScriptManager ().GetContext ();
				ctx->Prepare (m_ScriptHandleFunction);
				ctx->Execute ();
				ctx->Unprepare ();
				ctx->GetEngine ()->ReturnContext (ctx);
			}

			if (HandledFunction)
			{
				HandledFunction ();
			}
		}
	}

	//--------------------------------------------------------------------------------------------------

	int SpeechFrame::GetCurrentDrawingLine ()
	{
		size_t maxLinesCanFit = GetMaxLinesCanFit ();
		int currentDrawingLine = 0;

		//  How many lines are hidden
		int diff = static_cast<int> (m_CurrentLine) + 1 - maxLinesCanFit - m_CurrentLineBreakOffset;

		//  Offset current drawing start position by hidden lines
		if (m_CurrentLine + 1 - m_CurrentLineBreakOffset > maxLinesCanFit)
		{
			currentDrawingLine = diff;
		}

		//  Move line position pointer by keyboard
		currentDrawingLine += m_ChosenLineDelta;

		//  If we select line too far up
		if (currentDrawingLine < 0)
		{
			currentDrawingLine = 0;
			m_ChosenLineDelta = -diff;
		}

		//  If we select line too far down
		if (currentDrawingLine > diff && diff >= 0)
		{
			currentDrawingLine = diff;
			m_ChosenLineDelta = 0;
		}

		return currentDrawingLine + m_CurrentLineBreakOffset;
	}

	//--------------------------------------------------------------------------------------------------

	void SpeechFrame::SetText (const std::string& text)
	{
		m_OriginalText = text;
		m_Text = text;

		//  Extract break points and text attributes
		PreprocessText (m_Text);

		m_TextLines = BreakTextLines (m_Text, m_DrawRect.GetSize ().Width - 2 * SPEECH_FRAME_TEXT_INSETS);
		m_AttrColorIndex = 0;
		m_AttrDelayIndex = 0;
	}

	//--------------------------------------------------------------------------------------------------

	void SpeechFrame::AddChoice (const std::string& text, const std::string& action, std::function<void()> func)
	{
		SpeechChoice choice;
		choice.Text = text;
		choice.Action = action;
		choice.Func = func;

		m_Choices.push_back (choice);

		SetText (m_OriginalText + "\n" + text);
	}

	//--------------------------------------------------------------------------------------------------

	void SpeechFrame::Render (float)
	{
		if (!m_Visible || m_TextLines.empty ())
		{
			return;
		}

		DrawActorSprite ();

		//  Draw background
		draw_nine_patch_bitmap (m_FrameBitmap, static_cast<int> (m_DrawRect.GetPos ().X),
			static_cast<int> (m_DrawRect.GetPos ().Y), static_cast<int> (m_DrawRect.GetSize ().Width),
			static_cast<int> (m_DrawRect.GetSize ().Height));

		size_t maxLines = GetMaxLinesCanFit ();
		int currentDrawingLine = GetCurrentDrawingLine ();
		int currentDrawingLineCopy = currentDrawingLine;

		int x, y, w, h;
		al_get_clipping_rectangle (&x, &y, &w, &h);
		al_set_clipping_rectangle (static_cast<int> (m_DrawRect.GetPos ().X), static_cast<int> (m_DrawRect.GetPos ().Y),
			static_cast<int> (m_DrawRect.GetSize ().Width),
			static_cast<int> (m_DrawRect.GetSize ().Height - SPEECH_FRAME_LINE_OFFSET));

		float yOffset = -1;

		for (int i = 0; currentDrawingLine < m_CurrentLine + 1; ++currentDrawingLine, ++i)
		{
			if ((currentDrawingLine > m_TextLines.size () - 1) || (i > maxLines))
			{
				break;
			}

			std::string line = m_TextLines[currentDrawingLine];

			if (currentDrawingLine >= m_CurrentLine)
			{
				line = line.substr (0, m_CurrentIndexInLine);
			}

			Point drawPoint = GetNextDrawPoint (i);

			if (yOffset < 0 && currentDrawingLine >= m_TextLines.size () - m_Choices.size ())
			{
				yOffset = drawPoint.Y;
			}

			m_AttrColorIndex = 0;

			for (int j = 0; j < currentDrawingLine; ++j)
			{
				m_AttrColorIndex += m_TextLines[j].size ();
			}

			float advance = GetTextAdvance (currentDrawingLine);
			DrawTextLine (line, drawPoint, advance);
		}

		al_set_clipping_rectangle (x, y, w, h);

		//  We can only scroll, when there are no choices
		if (m_Choices.empty () || m_Choices.size () > maxLines)
		{
			DrawScrollArrows (currentDrawingLineCopy);
		}
		else
		{
			if (!m_StillUpdating)
			{
				DrawChoiceArrow (yOffset);
			}
		}
	}

	//--------------------------------------------------------------------------------------------------

	void SpeechFrame::DrawActorSprite ()
	{
		if (m_Atlas && m_ActorRegionName != "")
		{
			Point characterOffset = GetActorRegionOffset ();
			int edgeLength = static_cast<int> (
				std::min (m_DrawRect.GetSize ().Height - 2 * characterOffset.Y, SPEECH_FRAME_MAX_CHAR_EDGE_LENGTH));
			AtlasRegion region = m_Atlas->GetRegion (m_ActorRegionName);

			float ratio = std::min (static_cast<float> (edgeLength) / region.Bounds.Size.Width,
				static_cast<float> (edgeLength) / region.Bounds.Size.Height);

			float xPos = m_DrawRect.GetPos ().X - ratio * region.Bounds.Size.Width * 0.5f - characterOffset.X;
			float yPos
				= m_DrawRect.GetPos ().Y + m_DrawRect.GetHalfSize ().Height - region.Bounds.GetHalfSize ().Height;

			m_Atlas->DrawRegion (m_ActorRegionName, xPos, yPos, ratio, ratio, 0);
		}
	}

	//--------------------------------------------------------------------------------------------------

	void SpeechFrame::DrawTextLine (const std::string& line, Point drawPoint, float advance)
	{
		Font& font = m_Manager->GetSceneManager ()->GetMainLoop ()->GetScreen ()->GetFont ();

		for (size_t j = 0; j < line.length (); ++j)
		{
			ALLEGRO_COLOR color = {0.9f, 0.9f, 0.9f, 1.0f};

			for (size_t k = 0; k < m_Attributes.size (); ++k)
			{
				SpeechTextAttribute& attr = m_Attributes[k];

				if (m_AttrColorIndex + j >= attr.BeginIndex && m_AttrColorIndex + j <= attr.EndIndex)
				{
					if (attr.AttributesMask & ATTRIBUTE_COLOR)
					{
						color = attr.Color;
					}
				}

				if (m_AttrDelayIndex >= attr.BeginIndex && m_AttrDelayIndex <= attr.EndIndex)
				{
					if (attr.AttributesMask & ATTRIBUTE_DELAY && !m_IsDelayed && attr.Delay > 0.0f)
					{
						m_IsDelayed = true;
						m_DelayCounter = attr.Delay;
					}
				}
			}

			std::string begin = line.substr (0, j);
			if (!(isspace (line[j]) && TrimString (begin) == ""))
			{
				std::string charToDraw = std::string (1, line[j]);

				if (charToDraw == "," || charToDraw == ".")
				{
					advance -= 8;
				}

				font.DrawText (FONT_NAME_SPEECH_FRAME, charToDraw, color, drawPoint.X + advance, drawPoint.Y, 1.0f,
					GetTextAlign ());

				if (charToDraw == ",")
				{
					advance -= 8;
				}

				if (charToDraw == " ")
				{
					advance += SPEECH_FRAME_ADVANCE_SPACE;
				}
				else if (charToDraw == "'")
				{
					advance += 8;
				}
				else if (charToDraw == ".")
				{
					advance += 18;
				}
				else
				{
					advance += 25;
				}
			}
		}
	}

	//--------------------------------------------------------------------------------------------------

	void SpeechFrame::DrawChoiceArrow (float yOffset)
	{
		float xPos = m_DrawRect.GetPos ().X + SPEECH_FRAME_ADVANCE_CHOICE - SPEECH_FRAME_TEXT_INSETS * 0.5f;
		float yPos = yOffset + (m_ActualChoiceIndex) * (m_LineHeight + SPEECH_FRAME_LINE_OFFSET) + m_LineHeight * 0.5f;

		m_Manager->GetSceneManager ()
			->GetMainLoop ()
			->GetAtlasManager ()
			.GetAtlas (GetBaseName (GetResourcePath (PACK_MENU_UI)))
			->DrawRegion ("arrow_light", xPos, yPos, 1.0f, 1.0f, DegressToRadians (-90.0f), false);
	}

	//--------------------------------------------------------------------------------------------------

	void SpeechFrame::DrawScrollArrows (int currentDrawingLine)
	{
		int xOffset = 20;
		int yOffset = 2;
		std::string regionName = m_DrawLightArrow ? "arrow_light" : "arrow_dark";

		Atlas* atlas = m_Manager->GetSceneManager ()->GetMainLoop ()->GetAtlasManager ().GetAtlas (
			GetBaseName (GetResourcePath (PACK_MENU_UI)));

		//  Up arrow
		if (currentDrawingLine > 0)
		{
			atlas->DrawRegion (regionName, m_DrawRect.GetPos ().X + m_DrawRect.GetSize ().Width - xOffset,
				m_DrawRect.GetPos ().Y + yOffset, 1.0f, 1.0f, DegressToRadians (180.0f), false);
		}

		//  How many lines are hidden
		int diff = static_cast<int> (m_CurrentLine) + 1 - GetMaxLinesCanFit ();

		//  Down arrow
		if (currentDrawingLine < diff)
		{
			atlas->DrawRegion (regionName, m_DrawRect.GetPos ().X + m_DrawRect.GetSize ().Width - xOffset,
				m_DrawRect.GetPos ().Y + m_DrawRect.GetSize ().Height - yOffset, 1.0f, 1.0f, 0.0f, false);
		}
	}

	//--------------------------------------------------------------------------------------------------

	Point SpeechFrame::GetNextDrawPoint (int lineIndex)
	{
		float xPoint;
		float yPoint;

		if (m_DrawTextCenter)
		{
			Font& font = m_Manager->GetSceneManager ()->GetMainLoop ()->GetScreen ()->GetFont ();
			std::string text = m_Text.substr (0, m_CurrentIndexInLine);
			Point textDimensions = font.GetTextDimensions (FONT_NAME_SPEECH_FRAME, text);

			xPoint = m_DrawRect.GetPos ().X + m_DrawRect.GetSize ().Width * 0.5f;
			yPoint = m_DrawRect.GetPos ().Y + m_DrawRect.GetSize ().Height * 0.5f - textDimensions.Height * 0.5f;
		}
		else
		{
			xPoint = m_DrawRect.GetPos ().X + SPEECH_FRAME_TEXT_INSETS;
			yPoint = m_DrawRect.GetPos ().Y + SPEECH_FRAME_TEXT_INSETS;
		}

		yPoint = yPoint + lineIndex * (m_LineHeight + SPEECH_FRAME_LINE_OFFSET);

		return {xPoint, yPoint};
	}

	//--------------------------------------------------------------------------------------------------

	int SpeechFrame::GetTextAlign () { return m_DrawTextCenter ? ALLEGRO_ALIGN_CENTER : ALLEGRO_ALIGN_LEFT; }

	//--------------------------------------------------------------------------------------------------

	float SpeechFrame::GetTextAdvance (int lineCounter)
	{
		float advance = 0;

		if (m_DrawTextCenter)
		{
			Font& font = m_Manager->GetSceneManager ()->GetMainLoop ()->GetScreen ()->GetFont ();
			std::string text = m_Text.substr (0, m_CurrentIndexInLine);
			Point textDimensions = font.GetTextDimensions (FONT_NAME_SPEECH_FRAME, text);
			advance = -textDimensions.Width * 0.5f;
		}

		if (lineCounter >= m_TextLines.size () - m_Choices.size ())
		{
			advance = SPEECH_FRAME_ADVANCE_CHOICE;
		}

		return advance;
	}

	//--------------------------------------------------------------------------------------------------

	std::vector<std::string> SpeechFrame::BreakTextLines (const std::string& text, float maxWidth)
	{
		std::vector<std::string> ret;
		Font& font = m_Manager->GetSceneManager ()->GetMainLoop ()->GetScreen ()->GetFont ();
		std::string workLine = text;
		int currentIndex = 0;

		auto cutLine = [&](int indexToCut) {
			std::string str = workLine.substr (0, indexToCut);
			ret.push_back (str);
			workLine = workLine.substr (indexToCut);
			currentIndex = 0;
		};

		while (currentIndex < workLine.size ())
		{
			std::string currentPart = workLine.substr (0, currentIndex + 1);

			if (currentPart[currentPart.length () - 1] == '\n')
			{
				cutLine (currentIndex + 1);
			}
			else
			{
				if (font.GetTextDimensions (FONT_NAME_SPEECH_FRAME, currentPart).Width >= maxWidth)
				{
					int lastSpaceIndex = currentPart.rfind (' ');

					if (lastSpaceIndex != std::string::npos)
					{
						cutLine (lastSpaceIndex + 1);
					}
					else 
					{
						cutLine (currentIndex);
					}
				}
				else
				{
					++currentIndex;
				}
			}
		}

		if (workLine != "")
		{
			ret.push_back (workLine);
		}

		return ret;
	}

	//--------------------------------------------------------------------------------------------------

	void SpeechFrame::SetDrawRect (Rect rect) { m_DrawRect = rect; }

	//--------------------------------------------------------------------------------------------------

	void SpeechFrame::Show ()
	{
		m_Visible = true;
		m_CurrentIndexInLine = 0;
		m_AttrColorIndex = 0;
		m_AttrDelayIndex = 0;
		m_CurrentLine = 0;
		m_DrawTimeAccumulator = 0;
		m_ChosenLineDelta = 0;
		m_StillUpdating = true;
		m_Handled = false;
		m_DelayCounter = 0.0f;
		m_IsDelayed = false;
		m_ActualChoiceIndex = 0;
		m_IsSuspended = false;
		m_OverrideSuspension = false;
	}

	//--------------------------------------------------------------------------------------------------

	void SpeechFrame::Hide () { m_Visible = false; }

	//--------------------------------------------------------------------------------------------------

	bool SpeechFrame::IsVisible () const { return m_Visible; }

	//--------------------------------------------------------------------------------------------------

	void SpeechFrame::SetDrawTextCenter (bool center) { m_DrawTextCenter = center; }

	//--------------------------------------------------------------------------------------------------

	bool SpeechFrame::IsDrawTextCenter () const { return m_DrawTextCenter; }

	//--------------------------------------------------------------------------------------------------

	bool SpeechFrame::IsTextFitWithoutScroll () { return m_TextLines.size () <= GetMaxLinesCanFit (); }

	//--------------------------------------------------------------------------------------------------

	void SpeechFrame::PreprocessText (std::string& text)
	{
		std::stack<SpeechTextAttribute> attributes;
		int currIndex = 0;
		int newLinesCount = 0;

		m_Attributes.clear ();

		while (currIndex < text.length ())
		{
			if (text[currIndex] == '\n')
			{
				++newLinesCount;
			}
			else if (text[currIndex] == '@')
			{
				m_BreakPoints.push_back (currIndex - newLinesCount);
				text.erase (currIndex, 1);
				--currIndex;
			}
			else if (text[currIndex] == '[')
			{
				if (text[currIndex + 1] == '/')
				{
					SpeechTextAttribute& attr = attributes.top ();
					attributes.pop ();

					attr.EndIndex = currIndex - newLinesCount - 1;
					m_Attributes.push_back (attr);

					size_t close = text.find (']', currIndex);

					text.erase (currIndex, close - currIndex + 1);
					--currIndex;
				}
				else
				{
					size_t keyIndex = text.find ('=', currIndex + 1);
					std::string key = text.substr (currIndex + 1, keyIndex - 1 - currIndex);
					std::transform (key.begin (), key.end (), key.begin (), ::toupper);

					size_t close = text.find (']', currIndex);
					std::string value = text.substr (keyIndex + 1, close - 1 - keyIndex);
					std::transform (value.begin (), value.end (), value.begin (), ::toupper);

					SpeechTextAttribute attr;
					attr.BeginIndex = currIndex - newLinesCount;

					if (key == "COLOR")
					{
						if (value == "RED")
						{
							attr.Color = COLOR_RED;
						}
						else if (value == "YELLOW")
						{
							attr.Color = COLOR_YELLOW;
						}
						else if (value == "GREEN")
						{
							attr.Color = COLOR_GREEN;
						}
						else if (value == "LIGHTBLUE")
						{
							attr.Color = COLOR_LIGHTBLUE;
						}

						attr.AttributesMask |= ATTRIBUTE_COLOR;
					}
					else if (key == "WAIT")
					{
						attr.Delay = static_cast<float> (atof (value.c_str ()));
						attr.BeginIndex -= 1;

						if (attr.BeginIndex < 0)
						{
							attr.BeginIndex = 0;
						}

						attr.AttributesMask |= ATTRIBUTE_DELAY;
					}

					attributes.push (attr);

					text.erase (currIndex, close - currIndex + 1);
					currIndex -= 1;

					if (currIndex < 0)
					{
						currIndex = 0;
					}
				}
			}

			++currIndex;
		}
	}

	//--------------------------------------------------------------------------------------------------

	void SpeechFrame::SetDrawSpeed (float speedInMs) { m_DrawSpeed = speedInMs; }

	//--------------------------------------------------------------------------------------------------

	bool SpeechFrame::ShouldBeHandled () { return m_ShouldBeHandled; }

	//--------------------------------------------------------------------------------------------------

	bool SpeechFrame::IsHandled () { return m_Handled; }

	//--------------------------------------------------------------------------------------------------

	Point SpeechFrame::GetActorRegionOffset () { return Point (20, 10); }

	//--------------------------------------------------------------------------------------------------
}
