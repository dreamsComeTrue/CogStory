// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.
#include "Actor.h"
#include "Font.h"
#include "MainLoop.h"
#include "Scene.h"
#include "SceneManager.h"
#include "Screen.h"
#include "Script.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    Actor::Actor (SceneManager* sceneManager)
        : Animable (&sceneManager->GetMainLoop ()->GetAtlasManager ())
        , Scriptable (&sceneManager->GetMainLoop ()->GetScriptManager ())
        , Collidable (&sceneManager->GetMainLoop ()->GetPhysicsManager ())
        , Entity (sceneManager)
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

    bool Actor::Destroy () { return Lifecycle::Destroy (); }

    //--------------------------------------------------------------------------------------------------

    void Actor::ChooseAnimation (float angleDeg)
    {
        if (GetAnimations ().empty ())
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
        Animable::Update (deltaTime);

        UpdateScripts (deltaTime);

        if (m_CheckOverlap)
        {
            CheckOverlap ();
        }

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    void Actor::Render (float deltaTime) { Animable::Render (this); }

    //--------------------------------------------------------------------------------------------------

    void Actor::DrawBounds ()
    {
        if (m_SceneManager->IsDrawBoundingBox ())
        {
            al_draw_rectangle (Bounds.GetTopLeft ().X, Bounds.GetTopLeft ().Y, Bounds.GetBottomRight ().X,
                               Bounds.GetBottomRight ().Y, COLOR_YELLOW, 2);
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Actor::DrawName ()
    {
        Font& font = m_SceneManager->GetMainLoop ()->GetScreen ()->GetFont ();
        Point pos = { Bounds.GetCenter ().X, Bounds.GetBottomRight ().Y };
        std::string str = Name + "[" + ToString (ID) + "]";
        font.DrawText (FONT_NAME_SMALL, al_map_rgb (0, 255, 0), pos.X, pos.Y, str, ALLEGRO_ALIGN_CENTER);
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
                                    script->Run (area.ScriptOnEnterCallback->GetDeclaration (), &point);
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
                                script->Run (area.ScriptOnLeaveCallback->GetDeclaration (), &point);
                            }
                        }
                    }
                }
            }
        }
    }

    //--------------------------------------------------------------------------------------------------
}
