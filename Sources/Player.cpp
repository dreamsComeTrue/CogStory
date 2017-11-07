// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.
#include "Player.h"
#include "MainLoop.h"
#include "PhysicsManager.h"
#include "Scene.h"
#include "SceneManager.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    //  Pixels Per Second
    const float MOVE_SPEED = 110.0;
    const std::string ANIM_IDLE = "ANIM_IDLE";
    const std::string ANIM_MOVE_UP = "ANIM_MOVE_UP";
    const std::string ANIM_MOVE_LEFT = "ANIM_MOVE_LEFT";
    const std::string ANIM_MOVE_RIGHT = "ANIM_MOVE_RIGHT";

    //--------------------------------------------------------------------------------------------------

    Player::Player (SceneManager* sceneManager)
        : Scriptable (&sceneManager->GetMainLoop ()->GetScriptManager ())
        , Collidable (&sceneManager->GetMainLoop ()->GetPhysicsManager ())
        , m_SceneManager (sceneManager)
        , m_Image (nullptr)
        , m_FollowCamera (true)
    {
    }

    //--------------------------------------------------------------------------------------------------

    Player::~Player ()
    {
        if (!IsDestroyed ())
        {
            Destroy ();
        }
    }

    //--------------------------------------------------------------------------------------------------

    bool Player::Initialize ()
    {
        m_Image = al_load_bitmap (GetResourcePath (ResourceID::GFX_PLAYER).c_str ());
        Bounds.Transform.Size = { 64, 64 };

        InitializeAnimations ();

        Lifecycle::Initialize ();
    }

    //--------------------------------------------------------------------------------------------------

    void Player::BeforeEnter ()
    {
        PhysPoints.clear ();
        PhysPoints.push_back ({ { 20, 10 }, { 25, 0 }, { 39, 0 }, { 44, 10 }, { 44, 64 }, { 20, 64 } });
        SetPhysOffset (Bounds.Transform.Pos);
    }

    //--------------------------------------------------------------------------------------------------

    void Player::AfterLeave () {}

    //--------------------------------------------------------------------------------------------------

    bool Player::Destroy ()
    {
        if (m_Image != nullptr)
        {
            al_destroy_bitmap (m_Image);
            m_Image = nullptr;
        }

        Lifecycle::Destroy ();
    }

    //--------------------------------------------------------------------------------------------------

    bool Player::Update (float deltaTime)
    {
        m_Animation.Update (deltaTime);
        UpdateScripts (deltaTime);

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    void Player::ProcessEvent (ALLEGRO_EVENT* event, float deltaTime)
    {
        if (event->type == ALLEGRO_EVENT_KEY_UP)
        {
            m_Animation.SetCurrentAnimation (ANIM_IDLE);
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Player::Render (float deltaTime)
    {
        AnimationFrames& frames = m_Animation.GetCurrentAnimation ();
        const Rect& frame = frames.GetFrame (m_Animation.GetCurrentFrame ());
        int width = frame.Transform.Size.Width;
        int height = frame.Transform.Size.Height;

        al_draw_scaled_bitmap (m_Image, frame.Dim.TopLeft.X, frame.Dim.TopLeft.Y, width, height, Bounds.Transform.Pos.X,
            Bounds.Transform.Pos.Y, width, height, 0);
    }

    //--------------------------------------------------------------------------------------------------

    void Player::InitializeAnimations ()
    {
        AnimationFrames idleFrames (3);
        idleFrames.AddFrame (0, Rect{ Point (0, 0), Point (64, 64) });
        idleFrames.AddFrame (1, Rect{ Point (64, 0), Point (64, 64) });
        idleFrames.AddFrame (2, Rect{ Point (128, 0), Point (64, 64) });
        idleFrames.SetPlaySpeed (500);
        m_Animation.AddAnimationFrames (ANIM_IDLE, idleFrames);

        AnimationFrames moveLeftFrames (1);
        moveLeftFrames.AddFrame (0, Rect{ Point (0, 64), Point (64, 64) });
        moveLeftFrames.SetPlaySpeed (500);
        m_Animation.AddAnimationFrames (ANIM_MOVE_LEFT, moveLeftFrames);

        AnimationFrames moveRightFrames (1);
        moveRightFrames.AddFrame (0, Rect{ Point (0, 128), Point (64, 64) });
        moveRightFrames.SetPlaySpeed (500);
        m_Animation.AddAnimationFrames (ANIM_MOVE_RIGHT, moveRightFrames);

        AnimationFrames moveUpFrames (1);
        moveUpFrames.AddFrame (0, Rect{ Point (0, 64 * 3), Point (64, 64) });
        moveUpFrames.SetPlaySpeed (500);
        m_Animation.AddAnimationFrames (ANIM_MOVE_UP, moveUpFrames);

        m_Animation.SetCurrentAnimation (ANIM_IDLE);
    }

    //--------------------------------------------------------------------------------------------------

    void Player::HandleInput (float deltaTime)
    {
        ALLEGRO_KEYBOARD_STATE state;
        al_get_keyboard_state (&state);
        float dx = 0, dy = 0;

        if (al_key_down (&state, ALLEGRO_KEY_DOWN) || al_key_down (&state, ALLEGRO_KEY_S))
        {
            m_Animation.SetCurrentAnimation (ANIM_IDLE);
            dy = MOVE_SPEED * deltaTime;
        }

        if (al_key_down (&state, ALLEGRO_KEY_UP) || al_key_down (&state, ALLEGRO_KEY_W))
        {
            m_Animation.SetCurrentAnimation (ANIM_MOVE_UP);
            dy = -MOVE_SPEED * deltaTime;
        }

        if (al_key_down (&state, ALLEGRO_KEY_RIGHT) || al_key_down (&state, ALLEGRO_KEY_D))
        {
            m_Animation.SetCurrentAnimation (ANIM_MOVE_RIGHT);
            dx = MOVE_SPEED * deltaTime;
        }

        if (al_key_down (&state, ALLEGRO_KEY_LEFT) || al_key_down (&state, ALLEGRO_KEY_A))
        {
            m_Animation.SetCurrentAnimation (ANIM_MOVE_LEFT);
            dx = -MOVE_SPEED * deltaTime;
        }

        std::map<std::string, TriggerArea>& triggerAreas = m_SceneManager->GetActiveScene ()->GetTriggerAreas ();

        for (std::map<std::string, TriggerArea>::iterator it = triggerAreas.begin (); it != triggerAreas.end (); ++it)
        {
            TriggerArea& area = it->second;

            for (Polygon& polygon : area.Polygons)
            {
                if (area.TriggerCallback || area.ScriptTriggerCallback)
                {
                    PolygonCollisionResult r = m_SceneManager->GetMainLoop ()->GetPhysicsManager ().PolygonCollision (
                        GetPhysPolygon (0), polygon, { dx, dy });

                    if (r.WillIntersect)
                    {
                        if (area.TriggerCallback)
                        {
                            area.TriggerCallback (dx + r.MinimumTranslationVector.X, dy + r.MinimumTranslationVector.Y);
                        }

                        if (area.ScriptTriggerCallback)
                        {
                            const char* moduleName = area.ScriptTriggerCallback->GetModuleName ();
                            Script* script = m_SceneManager->GetMainLoop ()->GetScriptManager ().GetScriptByModuleName (
                                moduleName);

                            if (script)
                            {
                                Point point = { dx + r.MinimumTranslationVector.X, dy + r.MinimumTranslationVector.Y };
                                asIScriptContext* ctx = script->GetContext ();
                                ctx->Prepare (area.ScriptTriggerCallback);
                                ctx->SetArgObject (0, &point);

                                ctx->Execute ();
                            }
                        }
                    }
                }
            }
        }

        for (Tile* tile : m_SceneManager->GetActiveScene ()->GetTiles ())
        {
            if (!tile->PhysPoints.empty ())
            {
                for (int i = 0; i < tile->GetPhysPolygonsCount (); ++i)
                {
                    PolygonCollisionResult r = m_SceneManager->GetMainLoop ()->GetPhysicsManager ().PolygonCollision (
                        GetPhysPolygon (0), tile->GetPhysPolygon (i), { dx, dy });

                    if (r.WillIntersect)
                    {
                        dx = dx + r.MinimumTranslationVector.X;
                        dy = dy + r.MinimumTranslationVector.Y;
                    }
                }
            }
        }

        if (!AreSame (dx, 0) || !AreSame (dy, 0))
        {
            if (al_key_down (&state, ALLEGRO_KEY_LSHIFT))
            {
                float multiplier = 3;
                Move (dx * multiplier, dy * multiplier);
            }
            else
            {
                Move (dx, dy);
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Player::Move (float dx, float dy)
    {
        m_OldPosition = Bounds.Transform.Pos;
        Bounds.Transform.Pos.X += dx;
        Bounds.Transform.Pos.Y += dy;

        if (MoveCallback != nullptr && m_FollowCamera)
        {
            MoveCallback (dx, dy);
        }

        AddPhysOffset ({ dx, dy });
    }

    //--------------------------------------------------------------------------------------------------

    void Player::SetPosition (const Point& pos) { SetPosition (pos.X, pos.Y); }

    //--------------------------------------------------------------------------------------------------

    void Player::SetPosition (float x, float y)
    {
        m_OldPosition = Bounds.Transform.Pos;
        Bounds.Transform.Pos.X = x;
        Bounds.Transform.Pos.Y = y;

        if (MoveCallback != nullptr && m_FollowCamera)
        {
            MoveCallback (Bounds.Transform.Pos.X - m_OldPosition.X, Bounds.Transform.Pos.Y - m_OldPosition.Y);
        }

        SetPhysOffset ({ x, y });
    }

    //--------------------------------------------------------------------------------------------------

    Point Player::GetPosition () { return Bounds.Transform.Pos; }

    //--------------------------------------------------------------------------------------------------

    Point Player::GetSize () { return Bounds.Transform.Size; }

    //--------------------------------------------------------------------------------------------------

    void Player::SetFollowCamera (bool follow) { m_FollowCamera = follow; }

    //--------------------------------------------------------------------------------------------------
}
