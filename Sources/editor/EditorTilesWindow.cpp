// Copyright 2017-2019 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "EditorTilesWindow.h"
#include "Editor.h"
#include "MainLoop.h"
#include "Screen.h"

#include "imgui.h"
#include "addons/tiny-file-dialogs/tinyfiledialogs.h"

using json = nlohmann::json;

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    EditorTilesWindow::EditorTilesWindow (Editor* editor)
        : m_Editor (editor)
        , m_Image (nullptr)
        , m_TilesX (0)
        , m_TilesY (0)
        , m_DrawGrid (true)
        , m_IsVisible (false)
        , m_Width (32)
        , m_Height (32)
    {
    }

    //--------------------------------------------------------------------------------------------------

    EditorTilesWindow ::~EditorTilesWindow () {}

    //--------------------------------------------------------------------------------------------------

    void EditorTilesWindow::Show (std::function<void(std::string, std::string)> OnAcceptFunc,
        std::function<void(std::string, std::string)> OnCancelFunc)
    {
        m_OnAcceptFunc = OnAcceptFunc;
        m_OnCancelFunc = OnCancelFunc;

        m_IsVisible = true;
        m_TilesX = 0;
        m_TilesY = 0;

        memset (m_Name, 0, ARRAY_SIZE (m_Name));
        memset (m_Path, 0, ARRAY_SIZE (m_Path));

        if (m_Image)
        {
            al_destroy_bitmap (m_Image);
        }

        m_SaveTilesButtonPressed = false;
        m_SavePackButtonPressed = false;
        m_OpenImageButtonPressed = false;
    }

    //--------------------------------------------------------------------------------------------------

    void EditorTilesWindow::OnSave ()
    {
        if (m_Image && m_SaveTilesButtonPressed)
        {
            std::string path = GetDataPath () + "x/";
            std::string dirName = GetDataPath ();

            char const * filterPatterns[1] = { "*.*" };

            const char* chosenPath = tinyfd_saveFileDialog (
               "Save Tiles",
                        path.c_str (),
                        1,
                        filterPatterns,
                        "all files");

            if (chosenPath != nullptr && strlen (chosenPath) > 0)
            {
                std::string fileName = chosenPath;
                std::replace (fileName.begin (), fileName.end (), '\\', '/');

                dirName = GetDirectory (fileName);

                //  Count number of characters required to generate name
                int numberOfNums = static_cast<int> (std::to_string (m_TilesX * m_TilesY).length ());

                for (int y = 0; y < m_TilesY; ++y)
                {
                    for (int x = 0; x < m_TilesX; ++x)
                    {
                        ALLEGRO_BITMAP* sprite
                            = al_create_sub_bitmap (m_Image, x * m_Width, y * m_Height, m_Width, m_Height);

                        if (sprite)
                        {
                            std::ostringstream out;
                            out << std::internal << std::setfill ('0') << std::setw (numberOfNums)
                                << (x + y * m_TilesX);

                            std::string name = dirName + "/" + std::string (m_Name) + "_" + out.str () + ".png";

                            al_save_bitmap (name.c_str (), sprite);
                            al_destroy_bitmap (sprite);
                        }
                    }
                }

                m_SaveTilesButtonPressed = false;
            }
            else
            {
                m_SaveTilesButtonPressed = false;
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    void EditorTilesWindow::OnSavePack ()
    {
        if (m_Image && m_SavePackButtonPressed)
        {
            std::string path = GetDataPath () + "x/";
            std::string dirName = GetDataPath ();

            char const * filterPatterns[1] = { "*.*" };

            const char* chosenPath = tinyfd_saveFileDialog (
               "Save Tiles",
                        path.c_str (),
                        1,
                        filterPatterns,
                        "all files");

            if (chosenPath != nullptr && strlen (chosenPath) > 0)
            {
                std::string fileName = chosenPath;
                std::replace (fileName.begin (), fileName.end (), '\\', '/');

                dirName = GetDirectory (fileName);

                std::string fName = dirName + "/" + std::string (m_Name);

                al_save_bitmap ((fName + ".png").c_str (), m_Image);

                std::string packData = "";
                packData += "\n";
                packData += std::string (m_Name) + ".png\n";
                packData += "size: " + std::to_string (al_get_bitmap_width (m_Image)) + ","
                    + std::to_string (al_get_bitmap_height (m_Image)) + "\n";
                packData += "format: RGBA8888\n";
                packData += "filter: Nearest,Nearest\n";
                packData += "repeat: none\n";

                //  Count number of characters required to generate name
                int numberOfNums = static_cast<int> (std::to_string (m_TilesX * m_TilesY).length ());

                for (int y = 0; y < m_TilesY; ++y)
                {
                    for (int x = 0; x < m_TilesX; ++x)
                    {
                        std::ostringstream out;
                        out << std::internal << std::setfill ('0') << std::setw (numberOfNums) << (x + y * m_TilesX);

                        std::string name = "tile_" + out.str ();

                        packData += name + "\n";
                        packData += "  rotate: false\n";
                        packData
                            += "  xy: " + std::to_string (x * m_Width) + ", " + std::to_string (y * m_Height) + "\n";
                        packData += "  size: " + std::to_string (m_Width) + ", " + std::to_string (m_Height) + "\n";
                        packData += "  orig: 32, 32\n";
                        packData += "  offset: 0, 0\n";
                        packData += "  index: -1\n";
                    }
                }

                std::ofstream outFile ((fName + ".pack").c_str ());
                outFile.write (packData.c_str (), static_cast<int> (packData.length ()));
                outFile.close ();

                m_SavePackButtonPressed = false;
            }
            else
            {
                m_SavePackButtonPressed = false;
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    void EditorTilesWindow::OnAccept ()
    {
        if (m_OnAcceptFunc)
        {
        }
    }

    //--------------------------------------------------------------------------------------------------

    void EditorTilesWindow::OnCancel ()
    {
        if (m_OnCancelFunc)
        {
        }
    }

    //--------------------------------------------------------------------------------------------------

    std::string fileOpenPath = "";
    void EditorTilesWindow::Render ()
    {
        ImGui::SetNextWindowPos (ImVec2 (0, 0));
        ImGui::SetNextWindowSize (ImVec2 (340, 220), ImGuiCond_Always);

        if (ImGui::BeginPopupModal ("Tile Editor", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            float controlWidth = 260.f;

            ImGui::Text ("  Name");
            ImGui::SameLine ();
            ImGui::PushItemWidth (controlWidth);
            ImGui::InputText ("##name", m_Name, ARRAY_SIZE (m_Name));
            ImGui::PopItemWidth ();

            ImGui::Text ("  Path");
            ImGui::SameLine ();
            ImGui::PushItemWidth (controlWidth - 90);
            ImGui::InputText ("##path", m_Path, ARRAY_SIZE (m_Path));
            ImGui::PopItemWidth ();
            ImGui::SameLine ();
            if (ImGui::Button ("BROWSE", ImVec2 (80, 18)))
            {
                m_OpenImageButtonPressed = true;
            }

            if (m_OpenImageButtonPressed)
            {
                if (fileOpenPath == "")
                {
                    fileOpenPath = GetDataPath () + "x/";
                }

                char const * filterPatterns[1] = { "*.*" };

                const char* chosenPath = tinyfd_openFileDialog (
                   "Open Tiles",
                            fileOpenPath.c_str (),
                            1,
                            filterPatterns,
                            "all files",
                            0);

                if (chosenPath != nullptr && strlen (chosenPath) > 0)
                {
                    std::string fileName = chosenPath;
                    std::replace (fileName.begin (), fileName.end (), '\\', '/');

                    strcpy (m_Path, fileName.c_str ());
                    fileOpenPath = m_Path;

                    if (m_Image)
                    {
                        al_destroy_bitmap (m_Image);
                    }

                    m_Image = al_load_bitmap (m_Path);

                    m_OpenImageButtonPressed = false;
                }
                else
                {
                    m_OpenImageButtonPressed = false;
                }
            }

            ImGui::Text ("  Grid");
            ImGui::SameLine ();
            ImGui::Checkbox ("", &m_DrawGrid);
            ImGui::Text ("     W");
            ImGui::SameLine ();
            ImGui::PushItemWidth (controlWidth / 2 - 17);
            ImGui::InputInt ("##cellX", &m_Width);
            ImGui::PopItemWidth ();
            ImGui::SameLine ();
            ImGui::Text ("H");
            ImGui::SameLine ();
            ImGui::PushItemWidth (controlWidth / 2 - 17);
            ImGui::InputInt ("##cellY", &m_Height);
            ImGui::PopItemWidth ();
            ImGui::Text ("TilesX");
            ImGui::SameLine ();
            ImGui::PushItemWidth (50);
            ImGui::InputInt ("##tilesX", &m_TilesX, 0, 0, ImGuiInputTextFlags_ReadOnly);
            ImGui::PopItemWidth ();
            ImGui::SameLine ();
            ImGui::Text ("TilesY");
            ImGui::SameLine ();
            ImGui::PushItemWidth (50);
            ImGui::InputInt ("##tilesY", &m_TilesY, 0, 0, ImGuiInputTextFlags_ReadOnly);
            ImGui::PopItemWidth ();

            ImGui::Separator ();

            if (ImGui::Button ("CUT & SAVE", ImVec2 (controlWidth + 65, 18)))
            {
                m_SaveTilesButtonPressed = true;
            }

            OnSave ();

            if (ImGui::Button ("SAVE PACK", ImVec2 (controlWidth + 65, 18)))
            {
                m_SavePackButtonPressed = true;
            }

            OnSavePack ();

            ImGui::BeginGroup ();

            if (ImGui::Button ("ACCEPT", ImVec2 (controlWidth / 2 + 25, 18)))
            {
                ImGui::CloseCurrentPopup ();
                m_IsVisible = false;

                OnAccept ();
            }

            ImGui::SameLine ();

            if (ImGui::Button ("CANCEL", ImVec2 (controlWidth / 2 + 25, 18)) || m_Editor->IsCloseCurrentPopup ())
            {
                OnCancel ();

                ImGui::CloseCurrentPopup ();
                m_IsVisible = false;

                m_Editor->SetCloseCurrentPopup (false);
            }
            ImGui::EndGroup ();

            ImGui::EndPopup ();
        }
    }

    //--------------------------------------------------------------------------------------------------

    void EditorTilesWindow::RenderSpritesheet ()
    {
        const Point winSize = m_Editor->GetMainLoop ()->GetScreen ()->GetRealWindowSize ();
        const float margin = 10;
        const float frameBoxSize = 200;
        const Point selBeginPoint = {margin, winSize.Height - frameBoxSize - margin};

        // Selection box
        al_draw_filled_rectangle (selBeginPoint.X, selBeginPoint.Y, selBeginPoint.X + frameBoxSize,
            selBeginPoint.Y + frameBoxSize, COLOR_BLACK);
        al_draw_rectangle (selBeginPoint.X, selBeginPoint.Y, selBeginPoint.X + frameBoxSize,
            selBeginPoint.Y + frameBoxSize, COLOR_GREEN, 2);

        Point beginPoint = {345, 0};

        //  Spritesheet
        al_draw_filled_rectangle (beginPoint.X, beginPoint.Y, winSize.Width, winSize.Height, COLOR_BLACK);
        al_draw_rectangle (beginPoint.X, beginPoint.Y, winSize.Width, winSize.Height, COLOR_GREEN, 2);

        if (m_Image)
        {
            const int margin = 2 * 2;

            int imageWidth = al_get_bitmap_width (m_Image);
            int imageHeight = al_get_bitmap_height (m_Image);
            int canvasWidth = static_cast<int> (winSize.Width - margin - beginPoint.X);
            int canvasHeight = static_cast<int> (winSize.Height - margin - beginPoint.Y);

            int destWidth = imageWidth;
            int destHeight = imageHeight;
            float ratio = 1.0f;

            if (imageWidth > canvasWidth || imageHeight > canvasHeight)
            {
                ratio = std::max (
                    static_cast<float> (imageWidth) / canvasWidth, static_cast<float> (imageHeight) / canvasHeight);
                destWidth /= ratio;
                destHeight /= ratio;
            }

            beginPoint.X += 2;
            beginPoint.Y += 2;

            al_draw_scaled_bitmap (
                m_Image, 0, 0, imageWidth, imageHeight, beginPoint.X, beginPoint.Y, destWidth, destHeight, 0);

            float widthGridStep = m_Width / ratio;
            float heightGridStep = m_Height / ratio;

            m_TilesX = static_cast<int> (imageWidth / ratio / widthGridStep);
            m_TilesY = static_cast<int> (imageHeight / ratio / heightGridStep);

            if (m_DrawGrid)
            {
                int stepsX = 0;
                for (float i = beginPoint.X + widthGridStep; stepsX < m_TilesX; stepsX++, i += widthGridStep)
                {
                    al_draw_line (i, beginPoint.Y, i, beginPoint.Y + imageHeight / ratio, COLOR_YELLOW, 1);
                }

                int stepsY = 0;
                for (float i = beginPoint.Y + heightGridStep; stepsY < m_TilesY; stepsY++, i += heightGridStep)
                {
                    al_draw_line (beginPoint.X, i, beginPoint.X + imageWidth / ratio, i, COLOR_YELLOW, 1);
                }
            }

            //  Mouse selection
            ALLEGRO_MOUSE_STATE state;
            al_get_mouse_state (&state);

            if ((state.x > beginPoint.X && state.x < beginPoint.X + m_TilesX * m_Width / ratio)
                && (state.y > beginPoint.Y && state.y < beginPoint.Y + m_TilesY * m_Height / ratio))
            {
                float canvasX = (state.x - beginPoint.X) * ratio;
                float canvasY = (state.y - beginPoint.Y) * ratio;

                int selX = static_cast<int> (canvasX / m_Width) * m_Width;
                int selY = static_cast<int> (canvasY / m_Height) * m_Height;

                float drawX = beginPoint.X + selX / ratio;
                float drawY = beginPoint.Y + selY / ratio;

                int blendOp, blendSrc, blendDst;
                al_get_blender (&blendOp, &blendSrc, &blendDst);
                al_set_blender (ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA);
                ALLEGRO_COLOR color = COLOR_RED;
                color.a = 0.5f;
                al_draw_filled_rectangle (drawX, drawY, drawX + m_Width / ratio, drawY + m_Height / ratio, color);
                al_set_blender (blendOp, blendSrc, blendDst);

                float tileScale = (frameBoxSize - 2 * 2) / m_Width;

                al_draw_tinted_scaled_rotated_bitmap_region (m_Image, selX, selY, m_Width, m_Height, COLOR_WHITE, 0, 0,
                    selBeginPoint.X + 2, selBeginPoint.Y + 2, tileScale, tileScale, 0.f, 0);

                int index = (selX / m_Width) + (selY / m_Height) * m_TilesX;

                m_Editor->GetMainLoop ()->GetScreen ()->GetFont ().DrawText (FONT_NAME_MEDIUM,
                    std::string (m_Name) + "_" + std::to_string (index), COLOR_WHITE, selBeginPoint.X,
                    selBeginPoint.Y - 20, 1.0f, ALLEGRO_ALIGN_LEFT);
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    bool EditorTilesWindow::IsVisible () { return m_IsVisible; }

    //--------------------------------------------------------------------------------------------------
}
