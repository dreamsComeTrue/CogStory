// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "Editor.h"
#include "AtlasManager.h"
#include "MainLoop.h"
#include "Scene.h"
#include "Screen.h"

//#include "imgui/imgui.h"
//#include "imgui/imgui_impl_a5.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    const int TILES_COUNT = 8;
    const int TILE_SIZE = 90;

    static int CURRENT_ID = 0;

    //--------------------------------------------------------------------------------------------------
    //--------------------------------------------------------------------------------------------------

    Editor::Editor (MainLoop* mainLoop)
      : m_MainLoop (mainLoop)
      , m_IsDrawTiles (true)
      , m_IsSnapToGrid (true)
      , m_IsMousePan (false)
      , m_IsMouseWheel (false)
      , m_Rotation (0)
      , m_BaseGridSize (16)
      , m_GridSize (16)
      , m_CursorMode (CursorMode::TileSelectMode)
      , m_SelectedTile (nullptr)
      , m_TileUnderCursor (nullptr)
    {
    }

    //--------------------------------------------------------------------------------------------------

    Editor::~Editor ()
    {
        if (!IsDestroyed ())
        {
            Destroy ();
        }

        Lifecycle::Destroy ();
    }

    //--------------------------------------------------------------------------------------------------

    bool Editor::Initialize ()
    {
        Lifecycle::Initialize ();

        InitializeUI ();

        //        ImGui_ImplA5_Init (m_MainLoop->GetScreen ()->GetDisplay ());
        // ImGui::GetIO ().IniFilename = nullptr;

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    bool Editor::Destroy ()
    {
        // ImGui_ImplA5_Shutdown ();

        return Lifecycle::Destroy ();
    }

    //--------------------------------------------------------------------------------------------------

    bool Editor::Update (double deltaTime) { return true; }

    //--------------------------------------------------------------------------------------------------

    void Editor::ProcessEvent (SDL_Event* event, double deltaTime)
    {
        // ImGui_ImplA5_ProcessEvent (event);

        //  if (ImGui::GetIO ().WantCaptureMouse)
        {
            //       return;
        }

        bool tileSelected = false;
        if (event->type == SDL_MOUSEBUTTONDOWN)
        {
            if (event->button.button == SDL_BUTTON_LEFT)
            {
                tileSelected = ChooseTile (event->button.x, event->button.y);
            }
        }

        if (event->type == SDL_KEYDOWN)
        {
            switch (event->key.keysym.sym)
            {
                case SDLK_r:
                {
                    m_Rotation += event->key.keysym.mod & KMOD_SHIFT ? 15 : -15;

                    if (m_Rotation <= -360)
                    {
                        m_Rotation = 0;
                    }

                    if (m_Rotation >= 360)
                    {
                        m_Rotation = 0;
                    }

                    break;
                }

                case SDLK_g:
                {
                    m_BaseGridSize *= event->key.keysym.mod & KMOD_SHIFT ? 0.5 : 2;
                    m_BaseGridSize = std::max (1, std::min (m_BaseGridSize, 1024));
                    m_GridSize = std::max (1.0, m_BaseGridSize * m_MainLoop->GetSceneManager ()->GetCamera ().GetScale ().X);

                    break;
                }

                case SDLK_z:
                {
                    if (m_SelectedTile)
                    {
                        m_SelectedTile->ZOrder += event->key.keysym.mod & KMOD_SHIFT ? -1 : 1;

                        int currentID = m_SelectedTile->ID;

                        m_MainLoop->GetSceneManager ()->GetActiveScene ()->SortTiles ();

                        std::vector<Tile>& tiles = m_MainLoop->GetSceneManager ()->GetActiveScene ()->GetTiles ();

                        for (Tile& tile : tiles)
                        {
                            if (tile.ID == currentID)
                            {
                                m_SelectedTile = &tile;
                                break;
                            }
                        }
                    }

                    break;
                }

                case SDLK_DELETE:
                case SDLK_x:
                {
                    if (m_SelectedTile)
                    {
                        m_MainLoop->GetSceneManager ()->GetActiveScene ()->RemoveTile (*m_SelectedTile);
                        m_SelectedTile = nullptr;
                        m_CursorMode = CursorMode::TileSelectMode;
                    }

                    break;
                }

                case SDLK_c:
                {
                    if (m_TileUnderCursor)
                    {
                        m_SelectedAtlasRegion = m_Atlas->GetRegion (m_TileUnderCursor->Name);
                        m_SelectedTile = nullptr;
                        m_CursorMode = CursorMode::TileInsertMode;
                    }

                    break;
                }
            }
        }

        if (event->type == SDL_KEYUP)
        {
            switch (event->key.keysym.sym)
            {
                case SDLK_F5:
                {
                    MenuItemPlay ();
                    break;
                }

                case SDLK_SPACE:
                {
                    m_IsDrawTiles = !m_IsDrawTiles;

                    for (int i = 0; i < TILES_COUNT; ++i)
                    {
                        std::ostringstream name;
                        name << "TileRect" << i;
                    }

                    break;
                }

                case SDLK_s:
                {
                    m_IsSnapToGrid = !m_IsSnapToGrid;
                    break;
                }
            }
        }

        if (event->type == SDL_MOUSEBUTTONDOWN)
        {
            m_IsMousePan = event->button.button == SDL_BUTTON_MIDDLE;

            if (event->button.button == SDL_BUTTON_LEFT)
            {
                if (m_CursorMode == CursorMode::TileInsertMode && !tileSelected)
                {
                    AddTile (event->button.x, event->button.y);
                }

                if (m_CursorMode == CursorMode::TileSelectMode)
                {
                    Rect r;
                    m_SelectedTile = GetTileUnderCursor (event->button.x, event->button.y, std::move (r));

                    if (m_SelectedTile)
                    {
                        m_CursorMode = CursorMode::TileEditMode;
                        m_Rotation = m_SelectedTile->Rotation;
                    }
                }
                else if (m_CursorMode == CursorMode::TileEditMode)
                {
                    m_CursorMode = CursorMode::TileSelectMode;
                }
            }

            if (event->button.button == SDL_BUTTON_RIGHT)
            {
                m_CursorMode = CursorMode::TileSelectMode;
            }
        }
        else if (event->type == SDL_MOUSEBUTTONUP)
        {
            m_IsMousePan = false;
        }
        else if (event->type == SDL_MOUSEMOTION)
        {
            if (event->wheel.direction < 0.0)
            {
                m_MainLoop->GetSceneManager ()->GetCamera ().Scale (0.75f, 0.75f, event->button.x, event->button.y);
                m_GridSize = std::max (1.0, m_BaseGridSize * m_MainLoop->GetSceneManager ()->GetCamera ().GetScale ().X);
            }
            else if (event->wheel.direction > 0.0)
            {
                m_MainLoop->GetSceneManager ()->GetCamera ().Scale (1.25f, 1.25f, event->button.x, event->button.y);
                m_GridSize = std::max (1.0, m_BaseGridSize * m_MainLoop->GetSceneManager ()->GetCamera ().GetScale ().X);
            }

            if (m_IsMousePan)
            {
                m_MainLoop->GetSceneManager ()->GetCamera ().Move (event->button.x, event->button.y);
            }
        }
        else if (event->type == SDL_WINDOWEVENT)
        {
            Resize ();
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::Render (double deltaTime)
    {
        if (m_IsSnapToGrid)
        {
            DrawGrid ();
        }

        m_MainLoop->GetSceneManager ()->Render (deltaTime);

        int x, y;
        SDL_GetMouseState (&x, &y);

        m_MainLoop->GetSceneManager ()->GetCamera ().UseIdentityTransform ();

        Point translate = m_MainLoop->GetSceneManager ()->GetCamera ().GetTranslate ();
        Point scale = m_MainLoop->GetSceneManager ()->GetCamera ().GetScale ();
        Point point = CalculateCursorPoint (x, y);
        SDL_Renderer* renderer = m_MainLoop->GetScreen ()->GetRenderer ();

        if (m_CursorMode == CursorMode::TileInsertMode)
        {
            m_Atlas->DrawRegion (m_SelectedAtlasRegion.Name, point.X, point.Y, scale.X, scale.Y, m_Rotation);
        }

        if (m_CursorMode == CursorMode::TileSelectMode)
        {
            Rect r;
            m_TileUnderCursor = GetTileUnderCursor (x, y, std::move (r));

            if (m_TileUnderCursor)
            {
                SDL_Rect srcRect = { r.TopLeft.X, r.TopLeft.Y, r.BottomRight.Width - r.TopLeft.X, r.BottomRight.Height - r.TopLeft.Y };

                SDL_SetRenderDrawColor (renderer, COLOR_YELLOW.r, COLOR_YELLOW.g, COLOR_YELLOW.b, COLOR_YELLOW.a);
                SDL_RenderDrawRect (renderer, &srcRect);
            }
        }

        if (m_CursorMode == CursorMode::TileEditMode)
        {
            if (m_SelectedTile)
            {
                m_SelectedTile->Rotation = m_Rotation;
                m_SelectedTile->Bounds.TopLeft = { (translate.X + point.X), (translate.Y + point.Y) };

                Rect b = GetRenderBounds (*m_SelectedTile);
                SDL_Rect srcRect = { b.TopLeft.X, b.TopLeft.Y, b.BottomRight.Width - b.TopLeft.X, b.BottomRight.Height - b.TopLeft.Y };

                SDL_SetRenderDrawColor (renderer, COLOR_RED.r, COLOR_RED.g, COLOR_RED.b, COLOR_RED.a);
                SDL_RenderDrawRect (renderer, &srcRect);
            }
        }

        if (m_IsDrawTiles)
        {
            DrawTiles ();
        }

        RenderUI ();
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::DrawTiles ()
    {
        std::vector<AtlasRegion>& regions = m_Atlas->GetRegions ();
        const Point screenSize = m_MainLoop->GetScreen ()->GetScreenSize ();
        double beginning = screenSize.Width * 0.5 - (TILES_COUNT - 1) * 0.5 * TILE_SIZE - TILE_SIZE * 0.5;
        float advance = 0;

        for (int i = 0; i < TILES_COUNT; ++i)
        {
            if (i >= regions.size () - 1)
            {
                break;
            }

            advance = beginning + i * TILE_SIZE;

            Rect r = regions[i].Bounds;

            SDL_Rect srcRect = { r.TopLeft.X, r.TopLeft.Y, r.BottomRight.Width, r.BottomRight.Height };
            SDL_Rect dstRect = { advance + 1, screenSize.Height - TILE_SIZE + 1, TILE_SIZE - 2, TILE_SIZE - 2 };

            SDL_RenderCopyEx (m_MainLoop->GetScreen ()->GetRenderer (), m_Atlas->GetImage (), &srcRect, &dstRect, 0, NULL, SDL_FLIP_NONE);
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::DrawGrid ()
    {
        const SDL_Color LIGHT_GRAY{ 128, 128, 128, 255 };

        Point t = m_MainLoop->GetSceneManager ()->GetCamera ().GetTranslate ();
        Point scale = m_MainLoop->GetSceneManager ()->GetCamera ().GetScale ();
        SDL_Renderer* renderer = m_MainLoop->GetScreen ()->GetRenderer ();

        SDL_SetRenderDrawColor (renderer, LIGHT_GRAY.r, LIGHT_GRAY.g, LIGHT_GRAY.b, LIGHT_GRAY.a);

        int horBeginX = t.X / 2 * (1 / scale.X);

        if (t.X > 0)
        {
            horBeginX *= -1;
        }

        int horBeginY = t.Y / 1 * (1 / scale.Y);

        if (t.Y > 0)
        {
            horBeginY *= -1;
        }

        const Point screenSize = m_MainLoop->GetScreen ()->GetScreenSize ();

        int horEndX = 300 + std::fabs (horBeginX);
        int horEndY = screenSize.Height + std::fabs (horBeginY);

        for (int i = horBeginX; i < horEndX; ++i)
        {
            float xOffset = i * m_GridSize - t.X;

            //  |
            SDL_RenderDrawLine (renderer, xOffset, horBeginY, xOffset, horEndY);
        }

        horEndX = screenSize.Width + std::fabs (horBeginX);
        horEndY = 300 + std::fabs (horBeginY);

        for (int i = horBeginY; i < horEndY; ++i)
        {
            float yOffset = i * m_GridSize - t.Y;

            //  --
            SDL_RenderDrawLine (renderer, horBeginX, yOffset, horEndX, yOffset);
        }
    }

    //--------------------------------------------------------------------------------------------------

    bool Editor::ChooseTile (int mouseX, int mouseY)
    {
        const Point screenSize = m_MainLoop->GetScreen ()->GetScreenSize ();
        double beginning = screenSize.Width * 0.5 - (TILES_COUNT - 1) * 0.5 * TILE_SIZE - TILE_SIZE * 0.5;
        float advance = 0;
        std::vector<AtlasRegion> regions = m_Atlas->GetRegions ();

        for (int i = 0; i < TILES_COUNT; ++i)
        {
            if (i >= regions.size () - 1)
            {
                break;
            }

            advance = beginning + i * TILE_SIZE;
            double x = advance;
            double y = screenSize.Height - TILE_SIZE - 1;
            Rect r = Rect{ { x, y }, { x + TILE_SIZE, y + TILE_SIZE - 2 } };

            if (InsideRect (mouseX, mouseY, r))
            {
                m_CursorMode = CursorMode::TileInsertMode;
                m_SelectedAtlasRegion = regions[i];

                return true;
            }
        }

        return false;
    }

    //--------------------------------------------------------------------------------------------------

    Point Editor::CalculateCursorPoint (int mouseX, int mouseY)
    {
        Point translate = m_MainLoop->GetSceneManager ()->GetCamera ().GetTranslate ();
        Point scale = m_MainLoop->GetSceneManager ()->GetCamera ().GetScale ();
        int finalX = (mouseX + translate.X);
        int finalY = (mouseY + translate.Y);

        if (m_IsSnapToGrid)
        {
            finalX = std::floor ((finalX + m_GridSize * 0.5) / m_GridSize) * m_GridSize;
            finalY = std::floor ((finalY + m_GridSize * 0.5) / m_GridSize) * m_GridSize;
        }

        finalX = (finalX - translate.X);
        finalY = (finalY - translate.Y);

        return { finalX, finalY };
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::AddTile (int mouseX, int mouseY)
    {
        Tile tile;
        AtlasRegion region = m_Atlas->GetRegion (m_SelectedAtlasRegion.Name);
        Point translate = m_MainLoop->GetSceneManager ()->GetCamera ().GetTranslate ();
        Point point = CalculateCursorPoint (mouseX, mouseY);

        tile.ID = ++CURRENT_ID;
        tile.Tileset = m_Atlas->GetName ();
        tile.Name = m_SelectedAtlasRegion.Name;
        tile.Bounds = { { (translate.X + point.X), (translate.Y + point.Y) },
                        { region.Bounds.BottomRight.Width, region.Bounds.BottomRight.Height } };
        tile.Rotation = m_Rotation;

        m_MainLoop->GetSceneManager ()->GetActiveScene ()->AddTile (tile);
    }

    //--------------------------------------------------------------------------------------------------

    Tile* Editor::GetTileUnderCursor (int mouseX, int mouseY, Rect&& outRect)
    {
        std::vector<Tile>& tiles = m_MainLoop->GetSceneManager ()->GetActiveScene ()->GetTiles ();
        Tile* result = nullptr;

        for (Tile& tile : tiles)
        {
            Rect r = GetRenderBounds (tile);

            if (InsideRect (mouseX, mouseY, r))
            {
                if ((result == nullptr) || (result && ((result->ZOrder < tile.ZOrder) || (result->RenderID < tile.RenderID))))
                {
                    outRect = r;
                    result = &tile;
                }
            }
        }

        return result;
    }

    //--------------------------------------------------------------------------------------------------

    Point RotatePoint (float x, float y, const Point& origin, float angle)
    {
        float s = std::sin (DegressToRadians (angle));
        float c = std::cos (DegressToRadians (angle));

        // translate point back to origin:
        x -= origin.X;
        y -= origin.Y;

        double nx = (x * c) - (y * s);
        double ny = (x * s) + (y * c);

        // translate point back:
        return { nx + origin.X, ny + origin.Y };
    }

    //--------------------------------------------------------------------------------------------------

    Rect Editor::GetRenderBounds (Tile& tile)
    {
        Point translate = m_MainLoop->GetSceneManager ()->GetCamera ().GetTranslate ();
        Point scale = m_MainLoop->GetSceneManager ()->GetCamera ().GetScale ();

        Rect b = tile.Bounds;
        int width = b.BottomRight.Width * 0.5;
        int height = b.BottomRight.Height * 0.5;

        float x1 = (b.TopLeft.X - translate.X * (1 / scale.X) - width) * (scale.X);
        float y1 = (b.TopLeft.Y - translate.Y * (1 / scale.Y) - height) * (scale.Y);
        float x2 = (b.TopLeft.X - translate.X * (1 / scale.X) + width) * (scale.X);
        float y2 = (b.TopLeft.Y - translate.Y * (1 / scale.Y) + height) * (scale.Y);

        Point origin = { x1 + (x2 - x1) * 0.5, y1 + (y2 - y1) * 0.5 };
        Point pointA = RotatePoint (x1, y1, origin, tile.Rotation);
        Point pointB = RotatePoint (x1, y2, origin, tile.Rotation);
        Point pointC = RotatePoint (x2, y1, origin, tile.Rotation);
        Point pointD = RotatePoint (x2, y2, origin, tile.Rotation);

        float minX, minY, maxX, maxY;

        minX = std::min (pointA.X, std::min (pointB.X, std::min (pointC.X, pointD.X)));
        minY = std::min (pointA.Y, std::min (pointB.Y, std::min (pointC.Y, pointD.Y)));
        maxX = std::max (pointA.X, std::max (pointB.X, std::max (pointC.X, pointD.X)));
        maxY = std::max (pointA.Y, std::max (pointB.Y, std::max (pointC.Y, pointD.Y)));

        return { { minX, minY }, { maxX, maxY } };
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::InitializeUI ()
    {
        m_Atlas = m_MainLoop->GetSceneManager ()->GetAtlasManager ()->GetAtlas (GetBaseName (GetResourcePath (PACK_0_0_HOME)));

        //  Back frame
        for (int i = 0; i < TILES_COUNT; ++i)
        {
            std::ostringstream nameBack;
            nameBack << "TileRectBack" << i;
        }

        Resize ();
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::OnNewScene ()
    {
        //    window.YesFunction = [=]() {
        //           Gwk::Controls::Button* yesButton = (Gwk::Controls::Button*)control;
        //
        //          m_MainLoop->GetSceneManager ()->GetActiveScene ()->Reset ();
        //
        //          OnResetScale ();
        //    OnResetTranslate ();
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::OnNewSceneYesButton () {}

    //--------------------------------------------------------------------------------------------------

    void Editor::OnNewSceneNoButton () {}

    //--------------------------------------------------------------------------------------------------

    void Editor::OnLoadScene () {}

    //--------------------------------------------------------------------------------------------------

    void Editor::OnSaveScene () {}

    //--------------------------------------------------------------------------------------------------

    void Editor::OnPlay () { MenuItemPlay (); }

    //--------------------------------------------------------------------------------------------------

    void Editor::MenuItemPlay () { m_MainLoop->GetStateManager ()->SetActiveState ("GAMEPLAY_STATE"); }

    //--------------------------------------------------------------------------------------------------

    void Editor::OnExit () { m_MainLoop->Exit (); }

    //--------------------------------------------------------------------------------------------------

    void OnExitYesButton () {}

    //--------------------------------------------------------------------------------------------------

    void OnExitNoButton () {}

    //--------------------------------------------------------------------------------------------------

    void Editor::OnTileSelected () {}

    //--------------------------------------------------------------------------------------------------

    void Editor::OnResetTranslate () { m_MainLoop->GetSceneManager ()->GetCamera ().SetTranslate (0, 0); }

    //--------------------------------------------------------------------------------------------------

    void Editor::OnResetScale ()
    {
        int x, y;
        SDL_GetMouseState (&x, &y);

        Point scale = m_MainLoop->GetSceneManager ()->GetCamera ().GetScale ();

        m_MainLoop->GetSceneManager ()->GetCamera ().Scale (1 / scale.X, 1 / scale.Y, x, y);
        m_GridSize = std::max (1.0, m_BaseGridSize * m_MainLoop->GetSceneManager ()->GetCamera ().GetScale ().X);
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::OnShowGrid () { m_IsSnapToGrid = !m_IsSnapToGrid; }

    //--------------------------------------------------------------------------------------------------

    void Editor::OnGridIncrease ()
    {
        m_BaseGridSize /= 2;

        m_BaseGridSize = std::max (1, std::min (m_BaseGridSize, 1024));
        m_GridSize = std::max (1.0, m_BaseGridSize * m_MainLoop->GetSceneManager ()->GetCamera ().GetScale ().X);
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::OnGridDecrease ()
    {
        m_BaseGridSize *= 2;

        m_BaseGridSize = std::max (1, std::min (m_BaseGridSize, 1024));
        m_GridSize = std::max (1.0, m_BaseGridSize * m_MainLoop->GetSceneManager ()->GetCamera ().GetScale ().X);
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::RenderUI ()
    {
        //        ImGui_ImplA5_NewFrame ();

        //        Point translate = m_MainLoop->GetSceneManager ()->GetCamera ().GetTranslate ();
        //        Point scale = m_MainLoop->GetSceneManager ()->GetCamera ().GetScale ();
        //        const Point screenSize = m_MainLoop->GetScreen ()->GetScreenSize ();
        //        bool open = true;

        //        int winSize = 140.0f;
        //        int xOffset = 20.0f;

        //        ImGui::SetNextWindowPos (ImVec2 (xOffset, 20), ImGuiCond_FirstUseEver);
        //        ImGui::Begin ("Buttons",
        //                      &open,
        //                      ImVec2 (winSize, 120.f),
        //                      0.0f,
        //                      0); // ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
        //        ImGui::Button ("ZERO MOVE");
        //        ImGui::Button ("ZERO SCALE");

        //        if (ImGui::Button (m_IsSnapToGrid ? "HIDE GRID" : "SHOW GRID"))
        //        {
        //            OnShowGrid ();
        //        }

        //        ImGui::Button ("+++", ImVec2 (35, 20));
        //        ImGui::SameLine ();
        //        ImGui::Button ("---", ImVec2 (35, 20));
        //        ImGui::End ();

        //        winSize = 140.0f;
        //        xOffset = screenSize.Width - winSize - 20.0f;

        //        ImGui::SetNextWindowPos (ImVec2 (xOffset, 20), ImGuiCond_FirstUseEver);
        //        ImGui::Begin ("ToolBox",
        //                      &open,
        //                      ImVec2 (winSize, 220.f),
        //                      0.0f,
        //                      ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
        //        ImGui::SetWindowFontScale (1.2);

        //        ImGui::Text (" AVG: %.2f ms", 1000.0f / ImGui::GetIO ().Framerate);
        //        ImGui::Text (" FPS: %.1f", ImGui::GetIO ().Framerate);

        //        ImGui::TextColored (ImVec4 (0, 1, 0, 1), std::string ("   X: " + ToString (translate.X * (1 / scale.X))).c_str ());
        //        ImGui::TextColored (ImVec4 (0, 1, 0, 1), std::string ("   Y: " + ToString (translate.Y * (1 / scale.Y))).c_str ());
        //        ImGui::TextColored (ImVec4 (0, 1, 0, 1),
        //                            std::string ("   W: " + ToString (m_SelectedAtlasRegion.Bounds.BottomRight.Width)).c_str ());
        //        ImGui::TextColored (ImVec4 (0, 1, 0, 1),
        //                            std::string ("   H: " + ToString (m_SelectedAtlasRegion.Bounds.BottomRight.Height)).c_str ());
        //        ImGui::TextColored (ImVec4 (0, 1, 0, 1),
        //                            std::string ("   A: " + (m_SelectedTile ? ToString (m_SelectedTile->Rotation) : "-")).c_str ());
        //        ImGui::TextColored (ImVec4 (0, 1, 0, 1),
        //                            std::string ("ZORD: " + (m_SelectedTile ? ToString (m_SelectedTile->ZOrder) : "-")).c_str ());
        //        ImGui::TextColored (ImVec4 (0, 1, 0, 1), std::string ("   S: " + ToString (scale.X)).c_str ());
        //        ImGui::TextColored (ImVec4 (0, 1, 0, 1), std::string ("SNAP: " + ToString (m_IsSnapToGrid ? "YES" : "NO")).c_str ());
        //        ImGui::TextColored (ImVec4 (0, 1, 0, 1), std::string ("GRID: " + ToString (m_BaseGridSize)).c_str ());

        //        ImGui::End ();

        //        ImGui::Render ();
    }

    //--------------------------------------------------------------------------------------------------

    void Editor::Resize ()
    {
        const Point screenSize = m_MainLoop->GetScreen ()->GetScreenSize ();

        double beginning = screenSize.Width * 0.5 - (TILES_COUNT - 1) * 0.5 * TILE_SIZE - TILE_SIZE * 0.5;
        float advance = 0;

        for (int i = 0; i < TILES_COUNT; ++i)
        {
            advance = beginning + i * TILE_SIZE;

            std::ostringstream nameBack;
            nameBack << "TileRectBack" << i;
        }
    }

    //--------------------------------------------------------------------------------------------------
}
