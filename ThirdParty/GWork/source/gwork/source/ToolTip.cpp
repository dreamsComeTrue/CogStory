/*
 *  Gwork
 *  Copyright (c) 2010 Facepunch Studios
 *  Copyright (c) 2013-17 Nick Trout
 *  See license in Gwork.h
 */


#include <Gwork/ToolTip.h>
#include <Gwork/Utility.h>
#include <Gwork/InputHandler.h>
#include <Gwork/Controls/Canvas.h>

namespace Gwk
{
namespace ToolTip
{
    
static Controls::Base* g_toolTip = nullptr;

bool TooltipActive()
{
    return g_toolTip != nullptr;
}

void Enable(Controls::Base* control)
{
    if (!control->GetToolTip())
        return;

    g_toolTip = control;
}

void Disable(Controls::Base* control)
{
    if (g_toolTip == control)
        g_toolTip = nullptr;
}

void RenderToolTip(Skin::Base* skin)
{
    if (!g_toolTip)
        return;

    Gwk::Renderer::Base* render = skin->GetRender();
    Gwk::Point oldRenderOffset = render->GetRenderOffset();
    Gwk::Point MousePos = Input::GetMousePosition();
    Gwk::Rect Bounds = g_toolTip->GetToolTip()->GetBounds();
    Gwk::Rect rOffset = Gwk::Rect(MousePos.x-Bounds.w*0.5f, MousePos.y-Bounds.h-10,
                                    Bounds.w,
                                    Bounds.h);
    rOffset = Utility::ClampRectToRect(rOffset, g_toolTip->GetCanvas()->GetBounds());
    
    // Calculate offset on screen bounds
    render->AddRenderOffset(rOffset);
    render->EndClip();
    skin->DrawToolTip(g_toolTip->GetToolTip());
    g_toolTip->GetToolTip()->DoRender(skin);
    render->SetRenderOffset(oldRenderOffset);
}

void ControlDeleted(Controls::Base* control)
{
    Disable(control);
}

} // namespace ToolTip
} // namespace Gwk
