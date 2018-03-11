/*
 *  Gwork
 *  Copyright (c) 2010 Facepunch Studios
 *  Copyright (c) 2013-17 Nick Trout
 *  See license in Gwork.h
 */

#pragma once
#ifndef GWK_CONTROLS_PROPERTY_LABEL_TWO_BUTTON_H
#define GWK_CONTROLS_PROPERTY_LABEL_TWO_BUTTON_H

#include <Gwork/Controls/Button.h>
#include <Gwork/Controls/Property/BaseProperty.h>
#include <Gwork/Controls/TextBox.h>

namespace Gwk
{
    namespace Controls
    {
        namespace Property
        {
            class GWK_EXPORT LabelTwoButton : public Property::Base
            {
            public:
                GWK_CONTROL_INLINE (LabelTwoButton, Property::Base)
                {
                    TextLabel = new TextBox (this);
                    TextLabel->Dock (Position::Fill);
                    TextLabel->SetTextColor (Gwk::Colors::White);
                    TextLabel->SetShouldDrawBackground (false);
                    TextLabel->SetEditable (false);

                    FuncButton2 = new Button (this);
                    FuncButton2->SetWidth (30);
                    FuncButton2->Dock (Position::Right);
                    FuncButton2->SetMargin (Margin (1, 1, 1, 2));

                    FuncButton1 = new Button (this);
                    FuncButton1->SetWidth (30);
                    FuncButton1->Dock (Position::Right);
                    FuncButton1->SetMargin (Margin (1, 1, 1, 2));
                }

                LabelTwoButton (Gwk::Controls::Base* parent, const String& labelText, const String& button1Text,
                                const String& button2Text)
                    : LabelTwoButton (parent)
                {
                    TextLabel->SetText (labelText);
                    FuncButton1->SetText (button1Text);
                    FuncButton2->SetText (button2Text);
                }

                String GetPropertyValue () override { return TextLabel->GetText (); }

                void SetPropertyValue (const String& v, bool bFireChangeEvents) override
                {
                    // m_Label->SetText (v, bFireChangeEvents);
                }

                bool IsEditing () override { return FuncButton1->IsFocussed () || FuncButton2->IsFocussed (); }

                bool IsHovered () const override { return ParentClass::IsHovered () || TextLabel->IsHovered (); }

                TextBox* TextLabel;
                Button* FuncButton1;
                Button* FuncButton2;
            };
        }
    }
}
#endif // ifndef GWK_CONTROLS_PROPERTY_LABEL_TWO_BUTTON_H
