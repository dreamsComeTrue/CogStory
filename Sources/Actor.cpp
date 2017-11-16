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

        m_Image = al_load_bitmap (GetResourcePath (ResourceID::GFX_PLAYER).c_str ());
        Bounds.SetSize ({ 64, 64 });

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    void Actor::BeforeEnter ()
    {
        PhysPoints.clear ();
        PhysPoints.push_back ({ { 20, 10 }, { 25, 0 }, { 39, 0 }, { 44, 10 }, { 44, 64 }, { 20, 64 } });
        SetPhysOffset (Bounds.GetPos ());
    }

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

    bool Actor::Update (float deltaTime)
    {
        m_Animation.Update (deltaTime);
        UpdateScripts (deltaTime);

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    void Actor::Render (float deltaTime)
    {
        AnimationFrames& frames = m_Animation.GetCurrentAnimation ();
        Rect& frame = frames.GetFrame (m_Animation.GetCurrentFrame ());
        float width = frame.GetSize ().Width;
        float height = frame.GetSize ().Height;

        al_draw_scaled_bitmap (
          m_Image, frame.GetPos ().X, frame.GetPos ().Y, width, height, Bounds.GetPos ().X, Bounds.GetPos ().Y, width, height, 0);
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

        SetPhysOffset ({ x, y });
    }

    //--------------------------------------------------------------------------------------------------

    Point Actor::GetPosition () { return Bounds.GetPos (); }

    //--------------------------------------------------------------------------------------------------

    Point Actor::GetSize () { return Bounds.GetSize (); }

    //--------------------------------------------------------------------------------------------------
}
