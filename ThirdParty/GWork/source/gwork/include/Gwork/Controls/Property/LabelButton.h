/*
 *  Gwork
 *  Copyright (c) 2010 Facepunch Studios
 *  Copyright (c) 2013-17 Nick Trout
 *  See license in Gwork.h
 */

#pragma once
#ifndef GWK_CONTROLS_PROPERTY_LABEL_BUTTON_H
#define GWK_CONTROLS_PROPERTY_LABEL_BUTTON_H

#include <Gwork/Controls/Property/BaseProperty.h>
#include <Gwork/Controls/TextBox.h>
#include <Gwork/Controls/Button.h>

namespace Gwk
{
    namespace Controls
    {
        namespace Property
        {
            class GWK_EXPORT LabelButton : public Property::Base
            {
            public:
                GWK_CONTROL_INLINE (LabelButton, Property::Base)
                {
                    TextLabel = new TextBox (this);
                    TextLabel->Dock (Position::Fill);
                    TextLabel->SetTextColor (Gwk::Colors::White);
                    TextLabel->SetShouldDrawBackground (false);
                    TextLabel->SetEditable (false);

                    FuncButton = new Button (this);
                    FuncButton->SetWidth(30);
                    FuncButton->Dock (Position::Right);
                    FuncButton->SetMargin(Margin(1, 1, 1, 2));
                }

                LabelButton (Gwk::Controls::Base *parent, const String &labelText, const String& buttonText) :
                    LabelButton (parent)
                {
                    TextLabel->SetText (labelText);
                    FuncButton->SetText (buttonText);
                }

                String GetPropertyValue () override { return TextLabel->GetText (); }

                void SetPropertyValue (const String& v, bool bFireChangeEvents) override
                {
                    //m_Label->SetText (v, bFireChangeEvents);
                }

                bool IsEditing () override { return FuncButton->IsFocussed (); }

                bool IsHovered () const override { return ParentClass::IsHovered () || TextLabel->IsHovered (); }

                TextBox* TextLabel;
                Button* FuncButton;
            };
        }
    }
}
#endif // ifndef GWK_CONTROLS_PROPERTY_LABEL_BUTTON_H
