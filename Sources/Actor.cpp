// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.
#include "Actor.h"
#include "Atlas.h"
#include "Component.h"
#include "Font.h"
#include "MainLoop.h"
#include "Scene.h"
#include "SceneManager.h"
#include "Screen.h"
#include "Script.h"
#include "actors/components/MovementComponent.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    Actor::Actor (SceneManager* sceneManager)
        : Animable (&sceneManager->GetMainLoop ()->GetAtlasManager ())
        , Scriptable (&sceneManager->GetMainLoop ()->GetScriptManager ())
        , Collidable (&sceneManager->GetMainLoop ()->GetPhysicsManager ())
        , Entity (sceneManager)
        , m_IsUpdating (true)
        , m_FocusHeight (100.0f) //  We set it way down for small tiles grid layout
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
        if (m_AtlasName != "" && m_AtlasRegionName != "")
        {
            Bounds.Size = m_SceneManager->GetMainLoop ()
                              ->GetAtlasManager ()
                              .GetAtlas (m_AtlasName)
                              ->GetRegion (m_AtlasRegionName)
                              .Bounds.GetSize ();
        }

        return Lifecycle::Initialize ();
    }

    //--------------------------------------------------------------------------------------------------

    void Actor::BeforeEnter () {}

    //--------------------------------------------------------------------------------------------------

    void Actor::AfterLeave () {}

    //--------------------------------------------------------------------------------------------------

    bool Actor::Destroy ()
    {
        for (std::map<std::string, Component*>::iterator it = m_Components.begin (); it != m_Components.end ();)
        {
            SAFE_DELETE (it->second);
            m_Components.erase (it++);
        }

        return Lifecycle::Destroy ();
    }

    //--------------------------------------------------------------------------------------------------

    void Actor::ChooseWalkAnimation (float angleDeg)
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

    void Actor::ChooseStandAnimation (float angleDeg)
    {
        if (GetAnimations ().empty ())
        {
            return;
        }

        if (angleDeg > 45 && angleDeg < 135)
        {
            SetCurrentAnimation (ANIM_STAND_UP_NAME);
        }

        if (angleDeg > 225 && angleDeg < 315)
        {
            SetCurrentAnimation (ANIM_STAND_DOWN_NAME);
        }

        if (angleDeg >= 135 && angleDeg <= 225)
        {
            SetCurrentAnimation (ANIM_STAND_LEFT_NAME);
        }

        if (angleDeg <= 45 || angleDeg >= 315)
        {
            SetCurrentAnimation (ANIM_STAND_RIGHT_NAME);
        }
    }

    //--------------------------------------------------------------------------------------------------

    bool Actor::Update (float deltaTime)
    {
        Animable::Update (deltaTime);

        if (!m_IsUpdating)
        {
            return true;
        }

        UpdateScripts (deltaTime);

        if (m_CheckOverlap)
        {
            CheckOverlap ();
        }

        if (!m_Components.empty ())
        {
            for (std::map<std::string, Component*>::iterator it = m_Components.begin (); it != m_Components.end ();
                 ++it)
            {
                it->second->Update (deltaTime);
            }
        }

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    void Actor::Render (float deltaTime)
    {
        Animable::Render (this);

        if (!m_Components.empty ())
        {
            for (std::map<std::string, Component*>::iterator it = m_Components.begin (); it != m_Components.end ();
                 ++it)
            {
                it->second->Render (deltaTime);
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Actor::AddComponent (const std::string& name, Component* component)
    {
        if (m_Components.find (name) == m_Components.end ())
        {
            m_Components.insert (std::make_pair (name, component));
            component->Initialize ();
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Actor::RemoveComponent (const std::string& name)
    {
        for (std::map<std::string, Component*>::iterator it = m_Components.begin (); it != m_Components.end (); ++it)
        {
            if (it->first == name)
            {
                SAFE_DELETE (it->second);
                m_Components.erase (it);
                break;
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Actor::RemoveComponent (Component* component)
    {
        for (std::map<std::string, Component*>::iterator it = m_Components.begin (); it != m_Components.end (); ++it)
        {
            if (it->second == component)
            {
                SAFE_DELETE (component);
                m_Components.erase (it);
                break;
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    Component* Actor::FindComponent (const std::string& name, const std::string& typeName)
    {
        for (std::map<std::string, Component*>::iterator it = m_Components.begin (); it != m_Components.end (); ++it)
        {
            if (it->first == name && it->second->GetTypeName () == typeName)
            {
                return it->second;
            }
        }

        return nullptr;
    }

    //--------------------------------------------------------------------------------------------------

    MovementComponent* Actor::GetMovementComponent (const std::string& name)
    {
        return static_cast<MovementComponent*> (FindComponent (name, MovementComponent::TypeName));
    }

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
        Point pos = {Bounds.GetCenter ().X, Bounds.GetBottomRight ().Y};
        std::string str = Name + "[" + std::to_string (ID) + "]";
        font.DrawText (FONT_NAME_SMALL, al_map_rgb (0, 255, 0), pos.X, pos.Y, str, ALLEGRO_ALIGN_CENTER);
    }

    //--------------------------------------------------------------------------------------------------

    void Actor::Move (float dx, float dy)
    {
        m_OldPosition = Bounds.GetPos ();
        Bounds.SetPos (Bounds.GetPos () + Point (dx, dy));

        if (MoveCallback != nullptr)
        {
            MoveCallback (dx, dy);
        }

        AddPhysOffset ({dx, dy});
    }

    //--------------------------------------------------------------------------------------------------

    void Actor::Move (Point deltaMove) { Actor::Move (deltaMove.X, deltaMove.Y); }

    //--------------------------------------------------------------------------------------------------

    void Actor::SetPosition (float x, float y)
    {
        m_OldPosition = Bounds.GetPos ();
        Bounds.Pos = Point (x, y);

        FireMoveCallback ();

        SetPhysOffset (Bounds.GetPos ());

        MovementComponent* movementComponent = GetMovementComponent ("MOVEMENT_COMPONENT");
        if (movementComponent)
        {
            movementComponent->SetStartPos ({x, y});
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Actor::SetCenterPosition (float x, float y)
    {
        Point halfSize = Bounds.GetHalfSize ();

        SetPosition (x - halfSize.Width, y - halfSize.Height);
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
                if (GetPhysPolygonsCount () > 0
                    && (area.OnEnterCallback || area.ScriptOnEnterCallback || area.OnLeaveCallback
                           || area.ScriptOnLeaveCallback))
                {
                    PolygonCollisionResult r = m_SceneManager->GetMainLoop ()->GetPhysicsManager ().PolygonCollision (
                        GetPhysPolygon (0), polygon, {dx, dy});

                    if (r.WillIntersect || r.Intersect)
                    {
                        if (!area.WasEntered)
                        {
                            if (area.OnEnterCallback)
                            {
                                area.OnEnterCallback (
                                    dx + r.MinimumTranslationVector.X, dy + r.MinimumTranslationVector.Y);
                            }

                            if (area.ScriptOnEnterCallback)
                            {
                                Point point = {dx + r.MinimumTranslationVector.X, dy + r.MinimumTranslationVector.Y};

                                m_SceneManager->GetMainLoop ()->GetScriptManager ().RunScriptFunction (
                                    area.ScriptOnEnterCallback, &point);
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
                            Point point = {dx + r.MinimumTranslationVector.X, dy + r.MinimumTranslationVector.Y};

                            m_SceneManager->GetMainLoop ()->GetScriptManager ().RunScriptFunction (
                                area.ScriptOnLeaveCallback, &point);
                        }
                    }
                }
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Actor::FireMoveCallback ()
    {
        if (MoveCallback != nullptr)
        {
            MoveCallback (Bounds.GetPos ().X - m_OldPosition.X, Bounds.GetPos ().Y - m_OldPosition.Y);
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Actor::OrientTo (Actor* actor)
    {
        float angleDeg = GetAngleWith (actor);

        if (GetAnimations ().empty ())
        {
            return;
        }

        if (angleDeg > 45 && angleDeg < 135)
        {
            SetCurrentAnimation (ANIM_STAND_DOWN_NAME);
        }

        if (angleDeg > 225 && angleDeg < 315)
        {
            SetCurrentAnimation (ANIM_STAND_UP_NAME);
        }

        if (angleDeg >= 135 && angleDeg <= 225)
        {
            SetCurrentAnimation (ANIM_STAND_RIGHT_NAME);
        }

        if (angleDeg <= 45 || angleDeg >= 315)
        {
            SetCurrentAnimation (ANIM_STAND_LEFT_NAME);
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Actor::AssignFlagPointsToWalk (const std::string& flagPointName)
    {
        MovementComponent* movementComponent = GetMovementComponent ("MOVEMENT_COMPONENT");
        if (movementComponent)
        {
            movementComponent->SetMovementType (MovementType::MovePoints);

            FlagPoint* flagPoint = m_SceneManager->GetFlagPoint (flagPointName);

            if (flagPoint)
            {
                movementComponent->SetWalkPoints (flagPoint);
            }
        }
    }

    //--------------------------------------------------------------------------------------------------
}
