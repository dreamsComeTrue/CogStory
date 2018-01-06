// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "EditorActorMode.h"
#include "ActorFactory.h"
#include "Editor.h"
#include "MainLoop.h"
#include "SceneManager.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    EditorActorMode::EditorActorMode (Editor* editor)
      : m_Editor (editor)
      , m_SelectedActor (nullptr)
      , m_ActorUnderCursor (nullptr)
      , m_Rotation (0)
    {
    }

    //--------------------------------------------------------------------------------------------------

    EditorActorMode::~EditorActorMode () {}

    //--------------------------------------------------------------------------------------------------

    bool EditorActorMode::AddOrUpdateActor (const std::string& oldName, const std::string& actorType)
    {
        Actor* actor = m_Editor->m_MainLoop->GetSceneManager ().GetActiveScene ()->GetActor (oldName);

        if (actor)
        {
            m_Editor->m_MainLoop->GetSceneManager ().GetActiveScene ()->RemoveActor (oldName);
        }

        Actor* newActor = ActorFactory::GetActor (&m_Editor->m_MainLoop->GetSceneManager (), actorType);
        newActor->Name = oldName;

        m_Editor->m_MainLoop->GetSceneManager ().GetActiveScene ()->AddActor (oldName, newActor);

        m_Editor->m_EditorActorMode.Clear ();

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    void EditorActorMode::RemoveActor (const std::string& name)
    {
        m_Editor->m_MainLoop->GetSceneManager ().GetActiveScene ()->RemoveActor (name);
    }

    //--------------------------------------------------------------------------------------------------

    void EditorActorMode::Clear () {}

    //--------------------------------------------------------------------------------------------------

    bool EditorActorMode::MoveSelectedActor ()
    {
        ALLEGRO_MOUSE_STATE state;
        al_get_mouse_state (&state);

        if (state.buttons == 1 && m_SelectedActor)
        {
            Point translate = m_Editor->m_MainLoop->GetSceneManager ().GetCamera ().GetTranslate ();
            Point scale = m_Editor->m_MainLoop->GetSceneManager ().GetCamera ().GetScale ();
            Point point =
              m_Editor->CalculateCursorPoint (state.x + m_TileSelectionOffset.X, state.y + m_TileSelectionOffset.Y);

            m_SelectedActor->Bounds.SetPos (
              { (translate.X + point.X) * 1 / scale.X, (translate.Y + point.Y) * 1 / scale.Y });
            m_SelectedActor->TemplateBounds = m_SelectedActor->Bounds;
            m_SelectedActor->SetPhysOffset (
              m_SelectedActor->Bounds.GetPos ().X - m_SelectedActor->Bounds.GetHalfSize ().Width,
              m_SelectedActor->Bounds.GetPos ().Y - m_SelectedActor->Bounds.GetHalfSize ().Height);

            QuadTreeNode& quadTree = m_Editor->m_MainLoop->GetSceneManager ().GetActiveScene ()->GetQuadTree ();
            quadTree.Remove (m_SelectedActor);
            quadTree.Insert (m_SelectedActor);
            quadTree.UpdateStructures ();
        }

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    Actor* EditorActorMode::GetActorUnderCursor (int mouseX, int mouseY, Rect&& outRect)
    {
        std::vector<Actor*>& actors = m_Editor->m_MainLoop->GetSceneManager ().GetActiveScene ()->GetActors ();
        Actor* result = nullptr;

        for (Actor* actorIt : actors)
        {
            Rect r = GetRenderBounds (actorIt);

            if (InsideRect (mouseX, mouseY, r))
            {
                if ((result == nullptr) || (result && (result->RenderID < actorIt->RenderID)))
                {
                    outRect = r;
                    result = actorIt;

                    m_TileSelectionOffset = { r.GetTopLeft ().X - mouseX + r.GetSize ().Width * 0.5f,
                                              r.GetTopLeft ().Y - mouseY + r.GetSize ().Height * 0.5f };
                }
            }
        }

        return result;
    }

    //--------------------------------------------------------------------------------------------------

    Rect EditorActorMode::GetRenderBounds (Actor* actor)
    {
        Point translate = m_Editor->m_MainLoop->GetSceneManager ().GetCamera ().GetTranslate ();
        Point scale = m_Editor->m_MainLoop->GetSceneManager ().GetCamera ().GetScale ();

        Rect b = actor->Bounds;
        int halfWidth = b.GetSize ().Width * 0.5f;
        int halfHeight = b.GetSize ().Height * 0.5f;

        float x1 = (b.GetPos ().X - translate.X * (1 / scale.X) - halfWidth) * (scale.X);
        float y1 = (b.GetPos ().Y - translate.Y * (1 / scale.Y) - halfHeight) * (scale.Y);
        float x2 = (b.GetPos ().X - translate.X * (1 / scale.X) + halfWidth) * (scale.X);
        float y2 = (b.GetPos ().Y - translate.Y * (1 / scale.Y) + halfHeight) * (scale.Y);

        Point origin = { x1 + (x2 - x1) * 0.5f, y1 + (y2 - y1) * 0.5f };
        Point pointA = RotatePoint (x1, y1, origin, actor->Rotation);
        Point pointB = RotatePoint (x1, y2, origin, actor->Rotation);
        Point pointC = RotatePoint (x2, y1, origin, actor->Rotation);
        Point pointD = RotatePoint (x2, y2, origin, actor->Rotation);

        float minX, minY, maxX, maxY;

        minX = std::min (pointA.X, std::min (pointB.X, std::min (pointC.X, pointD.X)));
        minY = std::min (pointA.Y, std::min (pointB.Y, std::min (pointC.Y, pointD.Y)));
        maxX = std::max (pointA.X, std::max (pointB.X, std::max (pointC.X, pointD.X)));
        maxY = std::max (pointA.Y, std::max (pointB.Y, std::max (pointC.Y, pointD.Y)));

        return { { minX, minY }, { maxX, maxY } };
    }

    //--------------------------------------------------------------------------------------------------

    void EditorActorMode::ChangeRotation (bool clockwise)
    {
        m_Rotation += clockwise ? -15 : 15;

        if (m_Rotation <= -360)
        {
            m_Rotation = 0;
        }

        if (m_Rotation >= 360)
        {
            m_Rotation = 0;
        }

        if (m_SelectedActor)
        {
            m_SelectedActor->Rotation = m_Rotation;
        }
    }

    //--------------------------------------------------------------------------------------------------

    void EditorActorMode::ChangeZOrder (bool clockwise)
    {
        if (m_SelectedActor)
        {
            m_SelectedActor->ZOrder += clockwise ? -1 : 1;

            int currentID = m_SelectedActor->ID;

            m_Editor->m_MainLoop->GetSceneManager ().GetActiveScene ()->SortActors ();

            std::vector<Actor*>& actors = m_Editor->m_MainLoop->GetSceneManager ().GetActiveScene ()->GetActors ();

            for (Actor* actor : actors)
            {
                if (actor->ID == currentID)
                {
                    m_SelectedActor = actor;
                    break;
                }
            }
        }
    }

    //--------------------------------------------------------------------------------------------------
}
