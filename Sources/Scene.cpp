// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "Scene.h"
#include "Atlas.h"
#include "AtlasManager.h"
#include "MainLoop.h"
#include "SceneManager.h"
#include "Screen.h"

#include "addons/json/json.hpp"

using json = nlohmann::json;

namespace aga
{
    class PhysicsDraw : public b2Draw
    {
        virtual void DrawPolygon (const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
        {
            std::vector<float> out;

            for (int i = 0; i < vertexCount; ++i)
            {
                float xPoint = vertices[i].x * PTM_RATIO;
                float yPoint = vertices[i].y * PTM_RATIO;

                out.push_back (xPoint);
                out.push_back (yPoint);
            }

            al_draw_polygon (out.data (), vertexCount, 0, al_map_rgba (color.r * 256, color.g * 256, color.b * 256, color.a * 256), 2, 0);
        }

        /// Draw a solid closed polygon provided in CCW order.
        virtual void DrawSolidPolygon (const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
        {
            std::vector<float> out;

            for (int i = 0; i < vertexCount; ++i)
            {
                float xPoint = vertices[i].x;
                float yPoint = vertices[i].y;

                out.push_back (xPoint);
                out.push_back (yPoint);
            }

            al_draw_filled_polygon (out.data (), vertexCount, al_map_rgba (color.r * 256, color.g * 256, color.b * 256, color.a * 256));
        }

        /// Draw a circle.
        virtual void DrawCircle (const b2Vec2& center, float32 radius, const b2Color& color)
        {
            al_draw_circle (center.x, center.y, radius, al_map_rgba (color.r * 256, color.g * 256, color.b * 256, color.a * 256), 2);
        }

        /// Draw a solid circle.
        virtual void DrawSolidCircle (const b2Vec2& center, float32 radius, const b2Vec2& axis, const b2Color& color)
        {
            al_draw_filled_circle (center.x, center.y, radius, al_map_rgba (color.r * 256, color.g * 256, color.b * 256, color.a * 256));
        }

        /// Draw a line segment.
        virtual void DrawSegment (const b2Vec2& p1, const b2Vec2& p2, const b2Color& color) {}

        /// Draw a transform. Choose your own length scale.
        /// @param xf a transform.
        virtual void DrawTransform (const b2Transform& xf) {}

        /// Draw a point.
        virtual void DrawPoint (const b2Vec2& p, float32 size, const b2Color& color)
        {
            al_draw_filled_circle (p.x, p.y, size, al_map_rgba (color.r * 256, color.g * 256, color.b * 256, color.a * 256));
        }
    };

    b2Vec2 gravity (0.0f, 0.0f);

    //--------------------------------------------------------------------------------------------------

    /*
     *
     *  {
     *      "name": "Home",
     *      "spawn_points" :
     *      [
     *          "SPAWN_1" : "0 0"
     *      ]
     *  }
     *
     */

    //--------------------------------------------------------------------------------------------------
    //--------------------------------------------------------------------------------------------------

    auto StringToVector = [](std::string in) -> std::vector<Point> {
        size_t count = 0;
        const char* delimiter = " ";
        std::vector<Point> nums;
        char* str = const_cast<char*> (in.c_str ());

        for (char* pch = strtok (str, delimiter); pch != NULL; pch = strtok (NULL, delimiter))
        {
            float x = atof (pch);

            pch = strtok (NULL, delimiter);

            float y = atof (pch);

            nums.push_back ({ x, y });
        }

        return nums;
    };

    //--------------------------------------------------------------------------------------------------

    auto StringToPoint = [](std::string in) -> Point {
        std::vector<Point> vec = StringToVector (in);

        return vec[0];
    };

    //--------------------------------------------------------------------------------------------------

    auto PointToString = [](Point in) -> std::string { return ToString (in.X) + " " + ToString (in.Y); };

    //--------------------------------------------------------------------------------------------------

    auto VectorToString = [](std::vector<Point>& points) -> std::string {
        std::string out;

        for (Point& p : points)
        {
            out += PointToString (p) + " ";
        }

        return out;
    };

    //--------------------------------------------------------------------------------------------------

    void Tile::Draw (AtlasManager* atlasManager)
    {
        Atlas* atlas = atlasManager->GetAtlas (Tileset);

        if (atlas)
        {
            atlas->DrawRegion (Name, Bounds.TopLeft.X, Bounds.TopLeft.Y, 1, 1, DegressToRadians (Rotation));
        }
    }

    //--------------------------------------------------------------------------------------------------

    PhysicsDraw physDraw;

    Scene::Scene (SceneManager* sceneManager)
      : m_SceneManager (sceneManager)
      , m_PhysicsWorld (gravity)
    {
        m_PhysicsWorld.SetDebugDraw (&physDraw);
        physDraw.SetFlags (b2Draw::e_shapeBit | b2Draw::e_jointBit | b2Draw::e_aabbBit | b2Draw::e_pairBit | b2Draw::e_centerOfMassBit);
    }

    //--------------------------------------------------------------------------------------------------

    Scene::~Scene ()
    {
        if (!IsDestroyed ())
        {
            Destroy ();
        }

        Lifecycle::Destroy ();
    }

    //--------------------------------------------------------------------------------------------------

    bool Scene::Initialize () { return Lifecycle::Initialize (); }

    //--------------------------------------------------------------------------------------------------

    bool Scene::Destroy ()
    {
        for (int i = 0; i < m_Tiles.size (); ++i)
        {
            delete m_Tiles[i];
        }

        return Lifecycle::Destroy ();
    }

    //--------------------------------------------------------------------------------------------------

    Scene* Scene::LoadScene (SceneManager* sceneManager, const std::string& filePath)
    {
        try
        {
            Scene* scene = new Scene (sceneManager);
            std::ifstream file (filePath.c_str ());
            json j;
            file >> j;
            file.close ();

            scene->m_Name = j["name"];
            scene->m_Size = StringToPoint (j["size"]);

            auto& scripts = j["scripts"];

            for (auto& j_tile : scripts)
            {
                std::string name = j_tile["name"];
                std::string path = j_tile["path"];

                Script* script =
                  sceneManager->GetMainLoop ()->GetScriptManager ().LoadScriptFromFile (GetDataPath () + "scripts/" + path, name);
                scene->AttachScript (script, path);
            }

            auto& tiles = j["tiles"];

            for (auto& j_tile : tiles)
            {
                Tile* tile = new Tile ();

                std::string id = j_tile["id"];
                tile->ID = atoi (id.c_str ());
                tile->Tileset = j_tile["tileset"];
                tile->Name = j_tile["name"];
                tile->Bounds.TopLeft = StringToPoint (j_tile["pos"]);
                tile->Bounds.BottomRight =
                  sceneManager->GetAtlasManager ()->GetAtlas (tile->Tileset)->GetRegion (tile->Name).Bounds.BottomRight;
                std::string zOrder = j_tile["z-order"];
                tile->ZOrder = atoi (zOrder.c_str ());
                std::string rot = j_tile["rot"];
                tile->Rotation = atof (rot.c_str ());

                //  Physics
                if (!j_tile["phys"].is_null ())
                {
                    tile->PhysVertices = StringToVector (j_tile["phys"]);
                }

                b2BodyDef physBodyDef;
                physBodyDef.type = b2_staticBody;
                physBodyDef.position.Set (tile->Bounds.TopLeft.X / PTM_RATIO, tile->Bounds.TopLeft.Y / PTM_RATIO);

                tile->PhysBody = scene->m_PhysicsWorld.CreateBody (&physBodyDef);
                tile->PhysShape.SetAsBox (50.0f / PTM_RATIO / 2, 50.0f / PTM_RATIO / 2);

                b2FixtureDef fixtureDef;
                fixtureDef.shape = &tile->PhysShape;
                fixtureDef.density = 10.0f;
                fixtureDef.isSensor = true;

                tile->PhysBody->CreateFixture (&fixtureDef);

                scene->AddTile (tile);
            }

            auto& spawn_points = j["spawn_points"];

            for (auto& spawn_point : spawn_points)
            {
                std::string name = spawn_point["name"];
                Point pos = StringToPoint (spawn_point["pos"]);

                scene->m_SpawnPoints.insert (make_pair (name, pos));
            }

            return scene;
        }
        catch (const std::exception& e)
        {
            return nullptr;
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Scene::SaveScene (Scene* scene, const std::string& filePath)
    {
        try
        {
            json j;

            j["name"] = scene->m_Name;
            j["size"] = PointToString (scene->m_Size);

            j["scripts"] = json::array ({});

            for (std::map<ScriptMetaData, Script*>::iterator it = scene->m_Scripts.begin (); it != scene->m_Scripts.end (); ++it)
            {
                json scriptObj = json::object ({});

                scriptObj["name"] = it->first.Name;
                scriptObj["path"] = it->first.Path;

                j["scripts"].push_back (scriptObj);
            }

            j["tiles"] = json::array ({});

            for (Tile* tile : scene->m_Tiles)
            {
                json tileObj = json::object ({});

                tileObj["id"] = ToString (tile->ID);
                tileObj["tileset"] = tile->Tileset;
                tileObj["name"] = tile->Name;
                tileObj["pos"] = PointToString (tile->Bounds.TopLeft);
                tileObj["z-order"] = ToString (tile->ZOrder);
                tileObj["rot"] = ToString (tile->Rotation);
                tileObj["phys"] = VectorToString (tile->PhysVertices);

                j["tiles"].push_back (tileObj);
            }

            j["spawn_points"] = json::array ({});

            for (std::map<std::string, Point>::iterator it = scene->m_SpawnPoints.begin (); it != scene->m_SpawnPoints.end (); ++it)
            {
                json spawnObj = json::object ({});

                spawnObj["name"] = it->first;
                spawnObj["pos"] = PointToString (it->second);

                j["spawn_points"].push_back (spawnObj);
            }

            // write prettified JSON to another file
            std::ofstream out (filePath);
            out << std::setw (4) << j.dump (4, ' ') << "\n";
        }
        catch (const std::exception& e)
        {
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Scene::BeforeEnter ()
    {
        m_SceneManager->GetPlayer ().CreatePhysics (this);
        RunAllScripts ("void BeforeEnterScene ()");
    }

    //--------------------------------------------------------------------------------------------------

    void Scene::AfterLeave () { m_SceneManager->GetPlayer ().DestroyPhysics (this); }

    //--------------------------------------------------------------------------------------------------

    void Scene::Update (float deltaTime)
    {
        float32 timeStep = m_SceneManager->GetMainLoop ()->GetScreen ()->GetDeltaTime ();
        int32 velocityIterations = 6;
        int32 positionIterations = 2;

        m_PhysicsWorld.Step (timeStep, velocityIterations, positionIterations);

        for (int i = 0; i < m_Tiles.size (); ++i)
        {
            Tile* tile = m_Tiles[i];

            if (tile->PhysBody)
            {
                const b2Vec2 pos = tile->PhysBody->GetPosition ();

                tile->Bounds.TopLeft = { pos.x * PTM_RATIO, pos.y * PTM_RATIO };
            }
        }

        m_SceneManager->GetPlayer ().Update (deltaTime);
        UpdateScripts (deltaTime);
    }

    //--------------------------------------------------------------------------------------------------

    void Scene::Render (float deltaTime)
    {
        m_SceneManager->GetCamera ().Update (deltaTime);

        bool isPlayerDrawn = false;

        for (int i = 0; i < m_Tiles.size (); ++i)
        {
            Tile* tile = m_Tiles[i];

            if (!isPlayerDrawn && tile->ZOrder >= PLAYER_Z_ORDER)
            {
                m_SceneManager->GetPlayer ().Render (deltaTime);
                isPlayerDrawn = true;
            }

            tile->RenderID = i;
            tile->Draw (m_SceneManager->GetAtlasManager ());
        }

        if (!isPlayerDrawn)
        {
            m_SceneManager->GetPlayer ().Render (deltaTime);
        }

        m_SceneManager->GetMainLoop ()->GetScreen ()->GetFont ().DrawText (
          FONT_NAME_MAIN, al_map_rgb (0, 255, 0), -100, -50, m_Name, ALLEGRO_ALIGN_LEFT);

        // m_PhysicsWorld.DrawDebugData ();
    }

    //--------------------------------------------------------------------------------------------------

    void Scene::AddTile (Tile* tile) { m_Tiles.push_back (tile); }

    //--------------------------------------------------------------------------------------------------

    void Scene::RemoveTile (Tile* tile) { m_Tiles.erase (std::remove (m_Tiles.begin (), m_Tiles.end (), tile), m_Tiles.end ()); }

    //--------------------------------------------------------------------------------------------------

    void Scene::SortTiles () { std::sort (m_Tiles.begin (), m_Tiles.end (), Tile::CompareByZOrder); }

    //--------------------------------------------------------------------------------------------------

    std::vector<Tile*>& Scene::GetTiles () { return m_Tiles; }

    //--------------------------------------------------------------------------------------------------

    Point Scene::GetSpawnPoint (const std::string& name)
    {
        std::map<std::string, Point>::iterator it = m_SpawnPoints.find (name);

        if (it != m_SpawnPoints.end ())
        {
            return (*it).second;
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Scene::Reset () { m_Tiles.clear (); }

    //--------------------------------------------------------------------------------------------------

    std::string Scene::GetName () { return m_Name; }

    //--------------------------------------------------------------------------------------------------

    b2World& Scene::GetPhysicsWorld () { return m_PhysicsWorld; }

    //--------------------------------------------------------------------------------------------------
}
