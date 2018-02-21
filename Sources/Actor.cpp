// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.
#include "Actor.h"
#include "MainLoop.h"
#include "Scene.h"
#include "SceneManager.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    Actor::Actor (SceneManager* sceneManager)
        : Scriptable (&sceneManager->GetMainLoop ()->GetScriptManager ())
        , Collidable (&sceneManager->GetMainLoop ()->GetPhysicsManager ())
        , Entity (sceneManager)
        , m_Image (nullptr)
    {
        ID = Entity::GetNextID ();
    }

    //--------------------------------------------------------------------------------------------------

    Actor::~Actor ()
    {
        if (!IsDestroyed ())
        {
            Destroy ();
        }
    }

    //--------------------------------------------------------------------------------------------------

    bool Actor::Initialize ()
    {
        Lifecycle::Initialize ();

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    void Actor::BeforeEnter () {}

    //--------------------------------------------------------------------------------------------------

    void Actor::AfterLeave () {}

    //--------------------------------------------------------------------------------------------------

    bool Actor::Destroy ()
    {
        if (m_Image != nullptr)
        {
            al_destroy_bitmap (m_Image);
            m_Image = nullptr;
        }

        return Lifecycle::Destroy ();
    }

    //--------------------------------------------------------------------------------------------------

    void Actor::ChooseAnimation (float angleDeg)
    {
        if (m_Animation.GetAnimations ().empty ())
        {
            return;
        }

        if (angleDeg > 45 && angleDeg < 135)
        {
            SetCurrentAnimation (ANIM_MOVE_UP_NAME);
        }

        if (angleDeg > 225 && angleDeg < 315)
        {
            SetCurrentAnimation (ANIM_MOVE_DOWN_NAME);
        }

        if (angleDeg >= 135 && angleDeg <= 225)
        {
            SetCurrentAnimation (ANIM_MOVE_LEFT_NAME);
        }

        if (angleDeg <= 45 || angleDeg >= 315)
        {
            SetCurrentAnimation (ANIM_MOVE_RIGHT_NAME);
        }
    }

    //--------------------------------------------------------------------------------------------------

    bool Actor::Update (float deltaTime)
    {
        if (!m_Animation.GetAnimations ().empty ())
        {
            m_Animation.Update (deltaTime);
        }

        UpdateScripts (deltaTime);

        if (m_CheckOverlap)
        {
            CheckOverlap ();
        }

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    void Actor::Render (float deltaTime)
    {
        float sourceX = 0;
        float sourceY = 0;
        float sourceWidth;
        float sourceHeight;
        float targetX;
        float targetY;

        if (m_Image && !m_Animation.GetAnimations ().empty ())
        {
            AnimationFrames& frames = m_Animation.GetCurrentAnimation ();
            Rect& frame = frames.GetFrame (m_Animation.GetCurrentFrame ());

            sourceX = frame.GetPos ().X;
            sourceY = frame.GetPos ().Y;
            sourceWidth = frame.GetSize ().Width;
            sourceHeight = frame.GetSize ().Height;
            targetX = Bounds.GetPos ().X;
            targetY = Bounds.GetPos ().Y;
        }
        else
        {
            sourceWidth = al_get_bitmap_width (m_Image);
            sourceHeight = al_get_bitmap_height (m_Image);
            targetX = Bounds.GetPos ().X;
            targetY = Bounds.GetPos ().Y;
        }

        al_draw_tinted_scaled_rotated_bitmap_region (m_Image, sourceX, sourceY, sourceWidth, sourceHeight,
                                                     al_map_rgb (255, 255, 255), sourceWidth * 0.5, sourceHeight * 0.5,
                                                     targetX, targetY, 1, 1, Rotation, 0);
    }

    //--------------------------------------------------------------------------------------------------

    void Actor::DrawBounds ()
    {
        if (m_SceneManager->IsDrawBoundingBox ())
        {
            al_draw_rectangle (Bounds.GetTopLeft ().X - Bounds.GetHalfSize ().Width,
                               Bounds.GetTopLeft ().Y - Bounds.GetHalfSize ().Height,
                               Bounds.GetBottomRight ().X - Bounds.GetHalfSize ().Width,
                               Bounds.GetBottomRight ().Y - Bounds.GetHalfSize ().Height, COLOR_YELLOW, 2);
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Actor::Move (float dx, float dy)
    {
        m_OldPosition = Bounds.GetPos ();
        Bounds.SetPos (Bounds.GetPos () + Point (dx, dy));

        ChooseAnimation (ToPositiveAngle (RadiansToDegrees (std::atan2 (dy, dx))));

        if (MoveCallback != nullptr)
        {
            MoveCallback (dx, dy);
        }

        AddPhysOffset ({ dx, dy });
    }

    //--------------------------------------------------------------------------------------------------

    void Actor::SetPosition (Point pos) { SetPosition (pos.X, pos.Y); }

    //--------------------------------------------------------------------------------------------------

    void Actor::SetPosition (float x, float y)
    {
        m_OldPosition = Bounds.GetPos ();
        Bounds.SetPos ({ x, y });

        if (MoveCallback != nullptr)
        {
            MoveCallback (Bounds.GetPos ().X - m_OldPosition.X, Bounds.GetPos ().Y - m_OldPosition.Y);
        }

        SetPhysOffset (Bounds.GetPos ().X - Bounds.GetHalfSize ().Width,
                       Bounds.GetPos ().Y - Bounds.GetHalfSize ().Height);
    }

    //--------------------------------------------------------------------------------------------------

    Point Actor::GetPosition () { return Bounds.GetPos (); }

    //--------------------------------------------------------------------------------------------------

    Point Actor::GetSize () { return Bounds.GetSize (); }

    //--------------------------------------------------------------------------------------------------

    void Actor::SetCurrentAnimation (const std::string& name) { m_Animation.SetCurrentAnimation (name); }

    //--------------------------------------------------------------------------------------------------

    ALLEGRO_BITMAP* Actor::GetImage () { return m_Image; }

    //--------------------------------------------------------------------------------------------------

    void Actor::ProcessTriggerAreas (float dx, float dy)
    {
        std::map<std::string, TriggerArea>& triggerAreas = m_SceneManager->GetActiveScene ()->GetTriggerAreas ();

        for (std::map<std::string, TriggerArea>::iterator it = triggerAreas.begin (); it != triggerAreas.end (); ++it)
        {
            TriggerArea& area = it->second;

            for (Polygon& polygon : area.Polygons)
            {
                if (area.OnEnterCallback || area.ScriptOnEnterCallback || area.OnLeaveCallback
                    || area.ScriptOnLeaveCallback)
                {
                    PolygonCollisionResult r = m_SceneManager->GetMainLoop ()->GetPhysicsManager ().PolygonCollision (
                        GetPhysPolygon (0), polygon, { dx, dy });

                    if (r.WillIntersect || r.Intersect)
                    {
                        if (!area.WasEntered)
                        {
                            if (area.OnEnterCallback)
                            {
                                area.OnEnterCallback (dx + r.MinimumTranslationVector.X,
                                                      dy + r.MinimumTranslationVector.Y);
                            }

                            if (area.ScriptOnEnterCallback)
                            {
                                const char* moduleName = area.ScriptOnEnterCallback->GetModuleName ();
                                Script* script
                                    = m_SceneManager->GetMainLoop ()->GetScriptManager ().GetScriptByModuleName (
                                        moduleName);

                                if (script)
                                {
                                    Point point
                                        = { dx + r.MinimumTranslationVector.X, dy + r.MinimumTranslationVector.Y };
                                    asIScriptContext* ctx = script->GetContext ();
                                    ctx->Prepare (area.ScriptOnEnterCallback);
                                    ctx->SetArgObject (0, &point);

                                    ctx->Execute ();
                                }
                            }
                        }

                        area.WasEntered = true;
                    }
                    else if (area.WasEntered)
                    {
                        area.WasEntered = false;

                        if (area.OnLeaveCallback)
                        {
                            area.OnLeaveCallback (dx + r.MinimumTranslationVector.X, dy + r.MinimumTranslationVector.Y);
                        }

                        if (area.ScriptOnLeaveCallback)
                        {
                            const char* moduleName = area.ScriptOnLeaveCallback->GetModuleName ();
                            Script* script = m_SceneManager->GetMainLoop ()->GetScriptManager ().GetScriptByModuleName (
                                moduleName);

                            if (script)
                            {
                                Point point = { dx + r.MinimumTranslationVector.X, dy + r.MinimumTranslationVector.Y };
                                asIScriptContext* ctx = script->GetContext ();
                                ctx->Prepare (area.ScriptOnLeaveCallback);
                                ctx->SetArgObject (0, &point);

                                ctx->Execute ();
                            }
                        }
                    }
                }
            }
        }
    }

    //--------------------------------------------------------------------------------------------------
}
