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
        , m_SceneManager (sceneManager)
        , m_Image (nullptr)
        , m_CheckOverlap (false)
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

    void Actor::Render (float)
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

    void Actor::SetPosition (const Point& pos) { SetPosition (pos.X, pos.Y); }

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

    void Actor::SetCheckOverlap (bool check) { m_CheckOverlap = check; }

    //--------------------------------------------------------------------------------------------------

    bool Actor::IsCheckOverlap () { return m_CheckOverlap; }

    //--------------------------------------------------------------------------------------------------

    void Actor::CheckOverlap ()
    {
        Rect myBounds = m_SceneManager->GetActiveScene ()->GetRenderBounds (this);

        std::vector<Entity*> entites = m_SceneManager->GetActiveScene ()->GetVisibleEntities ();

        //        al_draw_rectangle (myBounds.GetTopLeft ().X,
        //                           myBounds.GetTopLeft ().Y,
        //                           myBounds.GetBottomRight ().X,
        //                           myBounds.GetBottomRight ().Y,
        //                           COLOR_YELLOW,
        //                           2);

        //  Special-case entity :)
        entites.push_back (&m_SceneManager->GetPlayer ());

        for (Entity* ent : entites)
        {
            if (ent != this)
            {
                Rect otherBounds = m_SceneManager->GetActiveScene ()->GetRenderBounds (ent);

                if (Intersect (myBounds, otherBounds))
                {
                    bool found = false;

                    for (Entity* saved : m_OverlapedEntities)
                    {
                        if (saved == ent)
                        {
                            found = true;
                            break;
                        }
                    }

                    if (!found)
                    {
                        m_OverlapedEntities.push_back (ent);

                        BeginOverlap (ent);
                    }
                }
                else
                {
                    for (std::vector<Entity*>::iterator it = m_OverlapedEntities.begin ();
                         it != m_OverlapedEntities.end (); ++it)
                    {
                        if (*it == ent)
                        {
                            EndOverlap (ent);

                            m_OverlapedEntities.erase (it);
                            break;
                        }
                    }
                }
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Actor::BeginOverlap (Entity* entity) {}

    //--------------------------------------------------------------------------------------------------

    void Actor::EndOverlap (Entity* entity) {}

    //--------------------------------------------------------------------------------------------------
}
